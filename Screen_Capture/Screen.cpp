#include "stdafx.h"
#include "Screen.h"
#include "ScreenInfo.h"
#include <assert.h>
#include <algorithm>


namespace SL {
	namespace Screen_Capture {

#if _WIN32

		//RAII Objects to ensure proper destruction
#define RAIIHDC(handle) std::unique_ptr<std::remove_pointer<HDC>::type, decltype(&::DeleteDC)>(handle, &::DeleteDC)
#define RAIIHBITMAP(handle) std::unique_ptr<std::remove_pointer<HBITMAP>::type, decltype(&::DeleteObject)>(handle, &::DeleteObject)
#define RAIIHANDLE(handle) std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::CloseHandle)>(handle, &::CloseHandle)

		SL::Screen_Capture::Image CaptureDesktopImage(const std::vector<ScreenInfo>& screens)
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
			BITMAPINFO bmpInfo;
			memset(&bmpInfo, 0, sizeof(BITMAPINFO));

			bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmpInfo.bmiHeader.biWidth = ret.Width;
			bmpInfo.bmiHeader.biHeight = -ret.Height;
			bmpInfo.bmiHeader.biPlanes = 1;
			bmpInfo.bmiHeader.biBitCount = get_pixelstride()*8;//always 32 bits damnit!!!
			bmpInfo.bmiHeader.biCompression = BI_RGB;
			bmpInfo.bmiHeader.biSizeImage = ((ret.Width * bmpInfo.bmiHeader.biBitCount + 31) / get_pixelstride() * 8) * get_pixelstride()* ret.Width;

			GetDIBits(desktopdc.get(), capturebmp.get(), 0, (UINT)ret.Width, ret.Data.get(), (BITMAPINFO *)&bmpInfo, DIB_RGB_COLORS);

			SelectObject(capturedc.get(), originalBmp);

			return ret;
		}

#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
		// iOS Simulator
#elif TARGET_OS_IPHONE
		// iOS device
#elif TARGET_OS_MAC
		// Other kinds of Mac OS
#else
#   error "Unknown Apple platform"
#endif


#include <ApplicationServices/ApplicationServices.h>

		std::shared_ptr<Image> CaptureDesktopImage()
		{
			auto image_ref = CGDisplayCreateImage(CGMainDisplayID());
			auto provider = CGImageGetDataProvider(image_ref);
			auto dataref = CGDataProviderCopyData(provider);
			size_t w, h;
			w = CGImageGetWidth(image_ref);
			h = CGImageGetHeight(image_ref);
			size_t bpp = CGImageGetBitsPerPixel(image_ref) / 8;

			auto img = Image::CreateImage(h, w, (const char*)(CFDataGetBytePtr(dataref)), bpp);

			CFRelease(dataref);
			CFRelease(provider);
			CGImageRelease(image_ref);
			return img;
		}

#elif __ANDROID__
		std::shared_ptr<Image> CaptureDesktopImage()
		{
			return Image::CreateImage(0, 0);
		}
#elif __linux__


		std::shared_ptr<Image> CaptureDesktopImage()
		{
			auto display = XOpenDisplay(NULL);
			auto root = DefaultRootWindow(display);
			auto screen = XDefaultScreen(display);
			auto visual = DefaultVisual(display, screen);
			auto depth = DefaultDepth(display, screen);

			XWindowAttributes gwa;
			XGetWindowAttributes(display, root, &gwa);
			auto width = gwa.width;
			auto height = gwa.height;

			XShmSegmentInfo shminfo;
			auto image = XShmCreateImage(display, visual, depth, ZPixmap, NULL, &shminfo, width, height);
			shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);

			shminfo.readOnly = False;
			shminfo.shmaddr = image->data = (char*)shmat(shminfo.shmid, 0, 0);

			XShmAttach(display, &shminfo);

			XShmGetImage(display, root, image, 0, 0, AllPlanes);

			XShmDetach(display, &shminfo);

			auto px = Image::CreateImage(height, width, (char*)shminfo.shmaddr, image->bits_per_pixel / 8);
			assert(image->bits_per_pixel == 32);//this should always be true... Ill write a case where it isnt, but for now it should be

			XDestroyImage(image);
			shmdt(shminfo.shmaddr);
			shmctl(shminfo.shmid, IPC_RMID, 0);
			XCloseDisplay(display);

			return px;

		}
#endif
	}
}


