#include "ScreenCaptureManager.h"
#include <vector>
#include <chrono>
#include <algorithm>
#include <assert.h>
#include <iostream>

struct ScreenInfo {
	int Width = 0;//width in pixels of the screen 
	int Height = 0;//Height in pixels of the screen 
	int Depth = 0;//Depth in pixels of the screen, i.e. 32 bit 
	char Device[32];//name of the screen 
	int Offsetx = 0;//distance in pixels from the MOST left screen. This can be negative because the primary monitor starts at 0, but this screen could be layed out to the left of the primary, in which case the offset is negative 
	int Offsety = 0;//distance in pixels from the TOP MOST screen 
	int Index = 0;//Index of the screen from LEFT to right of the physical monitors 
};
void Reorder(std::vector<ScreenInfo>& screens) {
	//organize the monitors so that the ordering is left to right for displaying purposes 
	std::sort(begin(screens), end(screens), [](const ScreenInfo& i, const ScreenInfo& j) { return i.Offsetx < j.Offsetx; });
	auto index = 0;
	for (auto& x : screens) x.Index = index++;
}
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

//RAII Objects to ensure proper destruction
#define RAIIHDC(handle) std::unique_ptr<std::remove_pointer<HDC>::type, decltype(&::DeleteDC)>(handle, &::DeleteDC)
#define RAIIHBITMAP(handle) std::unique_ptr<std::remove_pointer<HBITMAP>::type, decltype(&::DeleteObject)>(handle, &::DeleteObject)
#define RAIIHANDLE(handle) std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::CloseHandle)>(handle, &::CloseHandle)

SL::Screen_Capture::Image CaptureDesktopImage(const std::vector<ScreenInfo>& screens)
{
	auto start = std::chrono::high_resolution_clock::now();

	SL::Screen_Capture::Image ret;
	int left(0), top(0);
	for (const auto& mon : screens) {
		ret.Width += mon.Width;
		ret.Height = std::max(ret.Height, mon.Height);
		left = std::min(left, mon.Offsetx);
		top = std::min(top, mon.Offsety);
	}
	assert(ret.Width >= 0);
	assert(ret.Height >= 0);

	static auto desktopdc(RAIIHDC(CreateDCA("DISPLAY", NULL, NULL, NULL)));
	static auto capturedc(RAIIHDC(CreateCompatibleDC(desktopdc.get())));
	static auto capturebmp(RAIIHBITMAP(CreateCompatibleBitmap(desktopdc.get(), ret.Width, ret.Height)));

	if (!desktopdc || !capturedc || !capturebmp)
		return ret;

	// Selecting an object into the specified DC
	auto originalBmp = SelectObject(capturedc.get(), capturebmp.get());

	ret.Data = std::shared_ptr<char>(new char[ret.Height*ret.Width*ret.PixelStride], [](char* p) { delete[] p; }); //always

	if (BitBlt(capturedc.get(), 0, 0, ret.Width, ret.Height, desktopdc.get(), left, top, SRCCOPY | CAPTUREBLT) == FALSE) {
		//if the screen cannot be captured, set everything to 1 and return
		memset(ret.Data.get(), 1, ret.Height*ret.Width*ret.PixelStride);
		SelectObject(capturedc.get(), originalBmp);
		return ret;
	}
	BITMAPINFOHEADER bi;
	memset(&bi, 0, sizeof(bi));

	bi.biSize = sizeof(BITMAPINFOHEADER);

	bi.biWidth = ret.Width;
	bi.biHeight = -ret.Height;
	bi.biPlanes = 1;
	bi.biBitCount = ret.PixelStride * 8; //always 32 bits damnit!!!
	bi.biCompression = BI_RGB;
	bi.biSizeImage = ((ret.Width * bi.biBitCount + 31) / (ret.PixelStride * 8)) * ret.PixelStride* ret.Height;

	GetDIBits(desktopdc.get(), capturebmp.get(), 0, (UINT)ret.Height, ret.Data.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	SelectObject(capturedc.get(), originalBmp);

	std::cout << "took " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms\n";
	return ret;
}

std::vector<ScreenInfo> GetMoitors()
{
	std::vector<ScreenInfo> ret;
	DISPLAY_DEVICEA dd;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	for (auto i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
		//monitor must be attached to desktop and not a mirroring device
		if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) & !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
			ScreenInfo temp;
			DEVMODEA devMode;
			devMode.dmSize = sizeof(devMode);
			EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
			strncpy_s(temp.Device, dd.DeviceName, ARRAYSIZE(dd.DeviceName));
			temp.Offsetx = devMode.dmPosition.x;
			temp.Offsety = devMode.dmPosition.y;
			temp.Width = devMode.dmPelsWidth;
			temp.Height = devMode.dmPelsHeight;
			temp.Depth = devMode.dmBitsPerPel;
			ret.push_back(temp);
		}
	}
	//always reorder the screens left to right... as it should be, right?
	Reorder(ret);
	return ret;
}