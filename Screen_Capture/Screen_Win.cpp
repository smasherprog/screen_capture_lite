
#if _WIN32
#include "Screen.h"
#include <assert.h>
#include <algorithm>
#include <fstream>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

//RAII Objects to ensure proper destruction
#define RAIIHDC(handle) std::unique_ptr<std::remove_pointer<HDC>::type, decltype(&::DeleteDC)>(handle, &::DeleteDC)
#define RAIIHBITMAP(handle) std::unique_ptr<std::remove_pointer<HBITMAP>::type, decltype(&::DeleteObject)>(handle, &::DeleteObject)
#define RAIIHANDLE(handle) std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::CloseHandle)>(handle, &::CloseHandle)

void SL::Screen_Capture::Save(const Image& img, std::string path)
{
	assert(path.find(".bmp") != path.npos);
	//must be bmp.. save is for testing that the file is correct
	BITMAPINFOHEADER   bi;
	memset(&bi, 0, sizeof(bi));

	bi.biSize = sizeof(BITMAPINFOHEADER);

	bi.biWidth = img.Width;
	bi.biHeight = -img.Height;
	bi.biPlanes = 1;
	bi.biBitCount = get_pixelstride() * 8;//always 32 bits damnit!!!
	bi.biCompression = BI_RGB;
	bi.biSizeImage = ((img.Width * bi.biBitCount + 31) / (get_pixelstride() * 8)) * get_pixelstride()* img.Height;

	BITMAPFILEHEADER   bmfHeader;
	memset(&bmfHeader, 0, sizeof(bmfHeader));
	// A file is created, this is where we will save the screen capture.
	std::ofstream hFile(path, std::ios::trunc | std::ios::binary);
	assert(hFile);

	//Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = (DWORD)sizeof(bmfHeader) + (DWORD)sizeof(bi);

	//Size of the file
	bmfHeader.bfSize = bi.biSizeImage + bmfHeader.bfOffBits;

	//bfType must always be BM for Bitmaps
	bmfHeader.bfType = 0x4D42; //BM   

	hFile.write((char*)&bmfHeader, sizeof(bmfHeader));
	hFile.write((char*)&bi, sizeof(bi));
	hFile.write(img.Data.get(), bi.biSizeImage);

}

SL::Screen_Capture::Image SL::Screen_Capture::CaptureDesktopImage(const std::vector<ScreenInfo>& screens)
{
	Image ret;
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

	if (!desktopdc || !capturedc || !capturebmp) return ret;

	// Selecting an object into the specified DC 
	auto originalBmp = SelectObject(capturedc.get(), capturebmp.get());

	ret.Data = std::shared_ptr<char>(new char[get_imagesize(ret)], [](char* p) { delete[]p; });//always

	if (BitBlt(capturedc.get(), 0, 0, ret.Width, ret.Height, desktopdc.get(), left, top, SRCCOPY | CAPTUREBLT) == FALSE) {
		//if the screen cannot be captured, set everything to 1 and return 
		memset(ret.Data.get(), 1, get_imagesize(ret));
		SelectObject(capturedc.get(), originalBmp);
		return ret;
	}
	BITMAPINFOHEADER   bi;
	memset(&bi, 0, sizeof(bi));

	bi.biSize = sizeof(BITMAPINFOHEADER);

	bi.biWidth = ret.Width;
	bi.biHeight = -ret.Height;
	bi.biPlanes = 1;
	bi.biBitCount = get_pixelstride() * 8;//always 32 bits damnit!!!
	bi.biCompression = BI_RGB;
	bi.biSizeImage = ((ret.Width * bi.biBitCount + 31) / (get_pixelstride() * 8)) * get_pixelstride()* ret.Height;

	GetDIBits(desktopdc.get(), capturebmp.get(), 0, (UINT)ret.Height, ret.Data.get(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);

	SelectObject(capturedc.get(), originalBmp);

	return ret;
}

std::vector<SL::Screen_Capture::ScreenInfo> SL::Screen_Capture::GetMoitors()
{
	std::vector<SL::Screen_Capture::ScreenInfo> ret;
	DISPLAY_DEVICEA dd;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	for (auto i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
		//monitor must be attached to desktop and not a mirroring device
		if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) & !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
			SL::Screen_Capture::ScreenInfo temp;
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



#endif
