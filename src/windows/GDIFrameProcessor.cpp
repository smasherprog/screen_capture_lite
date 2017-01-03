#include "GDIFrameProcessor.h"

namespace SL {
	namespace Screen_Capture {

		GDIFrameProcessor::GDIFrameProcessor()
		{

		}

		GDIFrameProcessor::~GDIFrameProcessor()
		{

		}
		DUPL_RETURN GDIFrameProcessor::Init(ImageCallback& cb, Monitor monitor) {
			auto Ret = DUPL_RETURN_SUCCESS;

			MonitorDC.DC = CreateDCA(monitor.Name.c_str(), NULL, NULL, NULL);
			CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);
			CaptureBMP.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, monitor.Width, monitor.Height);

			if (!MonitorDC.DC || !CaptureDC.DC || !CaptureBMP.Bitmap) {
				Ret = DUPL_RETURN::DUPL_RETURN_ERROR_UNEXPECTED;
			}
			CurrentMonitor = monitor;
			CallBack = cb;
			return Ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN GDIFrameProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;

			SL::Screen_Capture::CapturedImage ret;
			ret.Height = CurrentMonitor.Height;
			ret.Width = CurrentMonitor.Width;
			ret.Offsetx = 0;
			ret.OffsetY = 0;
			// Selecting an object into the specified DC
			auto originalBmp = SelectObject(CaptureDC.DC, CaptureBMP.Bitmap);

			ret.Data = std::shared_ptr<char>(new char[ret.Height*ret.Width*ret.PixelStride], [](char* p) { delete[] p; }); //always

			if (BitBlt(CaptureDC.DC, 0, 0, ret.Width, ret.Height, MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
				//if the screen cannot be captured, set everything to 1 and return
				memset(ret.Data.get(), 1, ret.Height*ret.Width*ret.PixelStride);
				SelectObject(CaptureDC.DC, originalBmp);
				Ret = DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;//likely a permission issue
			}
			else {
				BITMAPINFOHEADER bi;
				memset(&bi, 0, sizeof(bi));

				bi.biSize = sizeof(BITMAPINFOHEADER);

				bi.biWidth = ret.Width;
				bi.biHeight = -ret.Height;
				bi.biPlanes = 1;
				bi.biBitCount = ret.PixelStride * 8; //always 32 bits damnit!!!
				bi.biCompression = BI_RGB;
				bi.biSizeImage = ((ret.Width * bi.biBitCount + 31) / (ret.PixelStride * 8)) * ret.PixelStride* ret.Height;

				GetDIBits(MonitorDC.DC, CaptureBMP.Bitmap, 0, (UINT)ret.Height, ret.Data.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

				SelectObject(CaptureDC.DC, originalBmp);
				CallBack(ret, CurrentMonitor);
			}

			return Ret;
		}

	}
}