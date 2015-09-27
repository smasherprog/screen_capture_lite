#include "stdafx.h"
#include "Screen.h"
#include <algorithm>
#include <chrono>
#include <iostream>

void Reorder(std::vector<SL::Screen_Capture::Screen_Info>& screens) {
	//organize the monitors so that the ordering is left to right for displaying purposes
	std::sort(begin(screens), end(screens), [](const SL::Screen_Capture::Screen_Info& i, const SL::Screen_Capture::Screen_Info& j) { return i.Offsetx < j.Offsetx; });
	auto index = 0;
	for (auto& x : screens) x.Index = index++;
}


#if _WIN32
//RAII Objects to ensure proper destruction
#define RAIIHDC(handle) std::unique_ptr<std::remove_pointer<HDC>::type, decltype(&::DeleteDC)>(handle, &::DeleteDC)
#define RAIIHBITMAP(handle) std::unique_ptr<std::remove_pointer<HBITMAP>::type, decltype(&::DeleteObject)>(handle, &::DeleteObject)

std::vector<SL::Screen_Capture::Screen_Info> SL::Screen_Capture::GetMoitors() {
	std::vector<SL::Screen_Capture::Screen_Info> ret;
	DISPLAY_DEVICEA dd;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	for (auto i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
		//monitor must be attached to desktop and not a mirroring device
		if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) & !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
			SL::Screen_Capture::Screen_Info temp;
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
	//always reorder the screens 
	Reorder(ret);
	return ret;
}


//needs to be the largest of the monitors
auto CreateCaptureBitmap(const std::vector<SL::Screen_Capture::Screen_Info>& screens, HDC desktopdc) {
	int width = 0;
	int height = 0;
	for (auto& a : screens) {
		width = std::max(a.Width, width);
		height = std::max(a.Height, height);
	}
	return RAIIHBITMAP(CreateCompatibleBitmap(desktopdc, width, height));
}

std::vector<char> CaptureDesktop(HDC desktop, HDC capturedc, HBITMAP bitmap, int left, int top, int width, int height)
{
	std::vector<char> retdata;
	// Selecting an object into the specified DC 
	auto originalBmp = SelectObject(capturedc, bitmap);
	retdata.resize(height*width * 4);
	if (!BitBlt(capturedc, 0, 0, width, height, desktop, left, top, SRCCOPY | CAPTUREBLT)) {
		//if the screen cannot be captured, set everything to 1 and return 
		memset(retdata.data(), 1, retdata.size());
		SelectObject(capturedc, originalBmp);
		return retdata;
	}

	BITMAPINFOHEADER   bi;
	memset(&bi, 0, sizeof(bi));

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;
	bi.biPlanes = 1;
	bi.biBitCount = 32;//always 32 bits damnit!!!
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	bi.biSizeImage = ((width * bi.biBitCount + 31) / 32) * 4 * height;

	GetDIBits(desktop, bitmap, 0, (UINT)height, retdata.data(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
	SelectObject(capturedc, originalBmp);

	return retdata;
}

std::vector<std::shared_ptr<SL::Screen_Capture::Screen>> SL::Screen_Capture::GetScreens(int index)
{
	///// The section in the comment block area is always fast 1-2 ms so recreating the resources each time is not a big deal
	auto ret = std::vector<std::shared_ptr<SL::Screen_Capture::Screen>>();
	auto Screens = GetMoitors();
	auto desktopdc = RAIIHDC(CreateDCA("DISPLAY", NULL, NULL, NULL));
	auto capturedc = RAIIHDC(CreateCompatibleDC(desktopdc.get()));
	auto capturebmp = CreateCaptureBitmap(Screens, desktopdc.get());
	///////
	//each screen grab below is where the time is spent specifically in the BitBlt function. On my computer this takes approx 30 ish ms to grab a 1080 @ 32 bits. This is unfortunately the fastest method on windows
	if (index == -1) {//get all screens
		for (auto& a : Screens) {
			ret.push_back(std::make_shared<SL::Screen_Capture::Screen>(a,CaptureDesktop(desktopdc.get(), capturedc.get(), capturebmp.get(), a.Offsetx, a.Offsety, a.Width, a.Height)));
		}
	}
	else if (index < Screens.size() && index>-1) {
		auto& a = Screens[index];
		ret.push_back(std::make_shared<SL::Screen_Capture::Screen>(a, CaptureDesktop(desktopdc.get(), capturedc.get(), capturebmp.get(), a.Offsetx, a.Offsety, a.Width, a.Height)));
	}
	return ret;
}


#else 
//LINUX or some other OS, please add code here and issue PULL Request!
std::vector<std::shared_ptr<SL::Screen_Capture::Screen>> SL::Screen_Capture::Build()
{
	auto ret = std::vector<std::shared_ptr<SL::Screen_Capture::Screen>>();
	return ret;
}
#endif
