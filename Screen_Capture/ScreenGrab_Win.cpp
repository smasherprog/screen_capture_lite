#include "stdafx.h"
#include "Screen.h"
#include <algorithm>


#if _WIN32

//RAII Objects to ensure proper destruction
#define RAIIHDC(handle) std::unique_ptr<std::remove_pointer<HDC>::type, decltype(&::DeleteDC)>(handle, &::DeleteDC)
#define RAIIHBITMAP(handle) std::unique_ptr<std::remove_pointer<HBITMAP>::type, decltype(&::DeleteObject)>(handle, &::DeleteObject)
#define RAIIHANDLE(handle) std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::CloseHandle)>(handle, &::CloseHandle)



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


void SaveBMP(BITMAPINFOHEADER bi, const char* imgdata, std::string filepath_dst) {
	BITMAPFILEHEADER   bmfHeader;
	// A file is created, this is where we will save the screen capture.
	auto hFile(RAIIHANDLE(CreateFileA(filepath_dst.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL)));

	// Add the size of the headers to the size of the bitmap to get the total file size
	DWORD dwSizeofDIB = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//Offset to where the actual bitmap bits start.
	bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	//Size of the file
	bmfHeader.bfSize = dwSizeofDIB;

	//bfType must always be BM for Bitmaps
	bmfHeader.bfType = 0x4D42; //BM   

	DWORD dwBytesWritten = 0;
	WriteFile(hFile.get(), (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile.get(), (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
	WriteFile(hFile.get(), (LPSTR)imgdata, bi.biSizeImage, &dwBytesWritten, NULL);
}


SL::Screen_Capture::Image CaptureDesktopImage(int left, int top, int width, int height, bool capturemouse)
{

	static auto desktopdc(RAIIHDC(CreateDCA("DISPLAY", NULL, NULL, NULL)));
	static auto capturedc(RAIIHDC(CreateCompatibleDC(desktopdc.get())));
	static auto capturebmp(RAIIHBITMAP(CreateCompatibleBitmap(desktopdc.get(), width, height)));
	
	if (!desktopdc || !capturedc || !capturebmp) return SL::Screen_Capture::Image();

	// Selecting an object into the specified DC 
	auto originalBmp = SelectObject(capturedc.get(), capturebmp.get());
	size_t total_size = height*width * 4;
	auto blk(SL::Screen_Capture::AquireBuffer(total_size));

	if (BitBlt(capturedc.get(), 0, 0, width, height, desktopdc.get(), left, top, SRCCOPY | CAPTUREBLT) == FALSE) {
		//if the screen cannot be captured, set everything to 1 and return 
		memset(SL::Screen_Capture::getData(blk), 1, total_size);
		SelectObject(capturedc.get(), originalBmp);
		return SL::Screen_Capture::Image(height, width, blk);
	}

	if (capturemouse) {
		CURSORINFO cursorInfo;
		cursorInfo.cbSize = sizeof(cursorInfo);
		GetCursorInfo(&cursorInfo);
		if ((cursorInfo.ptScreenPos.y > left && cursorInfo.ptScreenPos.y < left + width) || (cursorInfo.ptScreenPos.y + 32 > left && cursorInfo.ptScreenPos.y + 32 < left + width)) {
			ICONINFO ii = { 0 };
			GetIconInfo(cursorInfo.hCursor, &ii);
			auto colorbmp = RAIIHBITMAP(ii.hbmColor);//make sure this is cleaned up properly
			auto maskbmp = RAIIHBITMAP(ii.hbmMask);//make sure this is cleaned up properly
			DrawIcon(capturedc.get(), cursorInfo.ptScreenPos.x - ii.xHotspot, cursorInfo.ptScreenPos.y - ii.yHotspot, cursorInfo.hCursor);
		}
	}
	BITMAPINFO bmpInfo;
	memset(&bmpInfo, 0, sizeof(BITMAPINFO));

	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biHeight = -height;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 32;//always 32 bits damnit!!!
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biSizeImage = ((width * bmpInfo.bmiHeader.biBitCount + 31) / 32) * 4 * height;

	GetDIBits(desktopdc.get(), capturebmp.get(), 0, (UINT)height, SL::Screen_Capture::getData(blk), (BITMAPINFO *)&bmpInfo, DIB_RGB_COLORS);
	SelectObject(capturedc.get(), originalBmp);

	//Sanity check to ensure the data is correct
	//SaveBMP(bmpInfo.bmiHeader, blk.data.get(), "c:\\users\\scott\\desktop\\one.bmp");
	return SL::Screen_Capture::Image(height, width, blk);
}

SL::Screen_Capture::Image SL::Screen_Capture::CaptureDesktop(bool capturemouse)
{
	auto Screens = GetMoitors();
	if (Screens.empty()) return SL::Screen_Capture::Image();

	auto totalwidth = 0;
	auto totalheight = 0;
	std::for_each(begin(Screens), end(Screens), [&](SL::Screen_Capture::Screen_Info& s) {totalwidth += s.Width; totalheight = std::max(totalheight, s.Height); });

	return CaptureDesktopImage(0, 0, totalwidth, totalheight, capturemouse);
}

#endif
