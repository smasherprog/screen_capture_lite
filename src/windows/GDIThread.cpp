#include "GDIThread.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <vector>
#include <chrono>
#include <assert.h>
#include <iostream>
#include <string>
#include <thread>

//RAII Objects to ensure proper destruction
#define RAIIHDC(handle) std::unique_ptr<std::remove_pointer<HDC>::type, decltype(&::DeleteDC)>(handle, &::DeleteDC)
#define RAIIHBITMAP(handle) std::unique_ptr<std::remove_pointer<HBITMAP>::type, decltype(&::DeleteObject)>(handle, &::DeleteObject)
#define RAIIHANDLE(handle) std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(&::CloseHandle)>(handle, &::CloseHandle)

namespace SL {
	namespace Screen_Capture {

		CapturedImage CaptureDesktopImage(const Monitor& monitor, HDC desktopdc, HDC capturedc, HBITMAP capturebmp)
		{
			SL::Screen_Capture::CapturedImage ret;
			ret.ScreenIndex = monitor.Index;
			ret.Height = monitor.Height;
			ret.Width = monitor.Width;
			ret.Offsetx = 0;
			ret.OffsetY = 0;
			// Selecting an object into the specified DC
			auto originalBmp = SelectObject(capturedc, capturebmp);

			ret.Data = std::shared_ptr<char>(new char[ret.Height*ret.Width*ret.PixelStride], [](char* p) { delete[] p; }); //always

			if (BitBlt(capturedc, 0, 0, ret.Width, ret.Height, desktopdc, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
				//if the screen cannot be captured, set everything to 1 and return
				memset(ret.Data.get(), 1, ret.Height*ret.Width*ret.PixelStride);
				SelectObject(capturedc, originalBmp);
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

			GetDIBits(desktopdc, capturebmp, 0, (UINT)ret.Height, ret.Data.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

			SelectObject(capturedc, originalBmp);
			return ret;
		}



		void GDIThread(std::shared_ptr<THREAD_DATA> data) {

			auto desktopdc(RAIIHDC(CreateDCA(data->SelectedMonitor.Name.c_str(), NULL, NULL, NULL)));
			auto capturedc(RAIIHDC(CreateCompatibleDC(desktopdc.get())));
			auto capturebmp(RAIIHBITMAP(CreateCompatibleBitmap(desktopdc.get(), data->SelectedMonitor.Width, data->SelectedMonitor.Height)));

			if (!desktopdc || !capturedc || !capturebmp) {
				return;
			}
			while (!*data->TerminateThreadsEvent)
			{
				auto start = std::chrono::high_resolution_clock::now();
				auto img = CaptureDesktopImage(data->SelectedMonitor, desktopdc.get(), capturedc.get(), capturebmp.get());

				data->CallBack(img);
				auto mspassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
				std::string msg = "GDI took ";
				msg += std::to_string(mspassed) + "ms for output ";
				msg += std::to_string(data->SelectedMonitor.Index) + "\n";
				OutputDebugStringA(msg.c_str());
				auto timetowait = data->CaptureInterval - mspassed;
				if (timetowait > 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(timetowait));
				}
			}

		}
	}
}

