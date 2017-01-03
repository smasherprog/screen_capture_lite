#include "GDIFrameProcessor.h"

namespace SL {
	namespace Screen_Capture {

		GDIFrameProcessor::GDIFrameProcessor()
		{
			ImageBufferSize = 0;
		}

		GDIFrameProcessor::~GDIFrameProcessor()
		{

		}
		DUPL_RETURN GDIFrameProcessor::Init(std::shared_ptr<THREAD_DATA> data) {
			auto Ret = DUPL_RETURN_SUCCESS;

			MonitorDC.DC = CreateDCA(data->SelectedMonitor.Name.c_str(), NULL, NULL, NULL);
			CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);
			CaptureBMP.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, data->SelectedMonitor.Width, data->SelectedMonitor.Height);

			if (!MonitorDC.DC || !CaptureDC.DC || !CaptureBMP.Bitmap) {
				Ret = DUPL_RETURN::DUPL_RETURN_ERROR_UNEXPECTED;
			}
			Data = data;
			ImageBufferSize = data->SelectedMonitor.Width* data->SelectedMonitor.Height*PixelStride;
			ImageBuffer = std::make_unique<char[]>(ImageBufferSize);
			return Ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN GDIFrameProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;

			ImageRect ret;
			ret.left = ret.top = 0;
			ret.bottom = Data->SelectedMonitor.Height;
			ret.right = Data->SelectedMonitor.Width;

			// Selecting an object into the specified DC
			auto originalBmp = SelectObject(CaptureDC.DC, CaptureBMP.Bitmap);
			
			if (BitBlt(CaptureDC.DC, 0, 0, ret.right, ret.bottom, MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
				//if the screen cannot be captured, set everything to 1 and return
				memset(ImageBuffer.get(), 1, ImageBufferSize);
				SelectObject(CaptureDC.DC, originalBmp);
				Ret = DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;//likely a permission issue
			}
			else {

				BITMAPINFOHEADER bi;
				memset(&bi, 0, sizeof(bi));

				bi.biSize = sizeof(BITMAPINFOHEADER);

				bi.biWidth = ret.right;
				bi.biHeight = -ret.bottom;
				bi.biPlanes = 1;
				bi.biBitCount = PixelStride * 8; //always 32 bits damnit!!!
				bi.biCompression = BI_RGB;
				bi.biSizeImage = ((ret.right * bi.biBitCount + 31) / (PixelStride * 8)) * PixelStride* ret.bottom;
			
				GetDIBits(MonitorDC.DC, CaptureBMP.Bitmap, 0, (UINT)ret.bottom, ImageBuffer.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

				SelectObject(CaptureDC.DC, originalBmp);
				if (Data->CaptureEntireMonitor) {
					Data->CaptureEntireMonitor(ImageBuffer.get(), PixelStride, Data->SelectedMonitor);
				}
				
			}

			return Ret;
		}

	}
}