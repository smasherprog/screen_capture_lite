#include "GDIFrameProcessor.h"
#include "GDIHelpers.h"

namespace SL {
	namespace Screen_Capture {

		struct GDIFrameProcessorImpl {

			HDCWrapper MonitorDC;
			HDCWrapper CaptureDC;
			HBITMAPWrapper CaptureBMP;
			std::shared_ptr<Monitor_Thread_Data> Data;
			std::unique_ptr<char[]> OldImageBuffer, NewImageBuffer;
			size_t ImageBufferSize;
			bool FirstRun;
			std::string MonitorName;
		};


		GDIFrameProcessor::GDIFrameProcessor()
		{
			GDIFrameProcessorImpl_ = std::make_unique<GDIFrameProcessorImpl>();
			GDIFrameProcessorImpl_->ImageBufferSize = 0;
			GDIFrameProcessorImpl_->FirstRun = true;
		}

		GDIFrameProcessor::~GDIFrameProcessor()
		{

		}
		DUPL_RETURN GDIFrameProcessor::Init(std::shared_ptr<Monitor_Thread_Data> data) {
			auto Ret = DUPL_RETURN_SUCCESS;
			GDIFrameProcessorImpl_->MonitorName = Name(data->SelectedMonitor);
			GDIFrameProcessorImpl_->MonitorDC.DC = CreateDCA(GDIFrameProcessorImpl_->MonitorName.c_str(), NULL, NULL, NULL);
			GDIFrameProcessorImpl_->CaptureDC.DC = CreateCompatibleDC(GDIFrameProcessorImpl_->MonitorDC.DC);
			GDIFrameProcessorImpl_->CaptureBMP.Bitmap = CreateCompatibleBitmap(GDIFrameProcessorImpl_->MonitorDC.DC, Width(data->SelectedMonitor), Height(data->SelectedMonitor));

			if (!GDIFrameProcessorImpl_->MonitorDC.DC || !GDIFrameProcessorImpl_->CaptureDC.DC || !GDIFrameProcessorImpl_->CaptureBMP.Bitmap) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}
			GDIFrameProcessorImpl_->Data = data;
			GDIFrameProcessorImpl_->ImageBufferSize = Width(data->SelectedMonitor)* Height(data->SelectedMonitor)* PixelStride;
			if (GDIFrameProcessorImpl_->Data->CaptureDifMonitor) {//only need the old buffer if difs are needed. If no dif is needed, then the image is always new
				GDIFrameProcessorImpl_->OldImageBuffer = std::make_unique<char[]>(GDIFrameProcessorImpl_->ImageBufferSize);
			}
			GDIFrameProcessorImpl_->NewImageBuffer = std::make_unique<char[]>(GDIFrameProcessorImpl_->ImageBufferSize);

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
			ret.bottom = Height(GDIFrameProcessorImpl_->Data->SelectedMonitor);
			ret.right = Width(GDIFrameProcessorImpl_->Data->SelectedMonitor);

			DEVMODEA devMode;
			devMode.dmSize = sizeof(devMode);
			if(EnumDisplaySettingsA(GDIFrameProcessorImpl_->MonitorName.c_str(), ENUM_CURRENT_SETTINGS, &devMode)==TRUE){
				if (static_cast<int>(devMode.dmPelsHeight) != ret.bottom || static_cast<int>(devMode.dmPelsWidth) != ret.right) {
					return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
				}
			}

			// Selecting an object into the specified DC
			auto originalBmp = SelectObject(GDIFrameProcessorImpl_->CaptureDC.DC, GDIFrameProcessorImpl_->CaptureBMP.Bitmap);

			if (BitBlt(GDIFrameProcessorImpl_->CaptureDC.DC, 0, 0, ret.right, ret.bottom, GDIFrameProcessorImpl_->MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
				//if the screen cannot be captured, return
				SelectObject(GDIFrameProcessorImpl_->CaptureDC.DC, originalBmp);
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;//likely a permission issue
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

				GetDIBits(GDIFrameProcessorImpl_->MonitorDC.DC, GDIFrameProcessorImpl_->CaptureBMP.Bitmap, 0, (UINT)ret.bottom, GDIFrameProcessorImpl_->NewImageBuffer.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

				SelectObject(GDIFrameProcessorImpl_->CaptureDC.DC, originalBmp);

				if (GDIFrameProcessorImpl_->Data->CaptureEntireMonitor) {
					auto wholeimg = Create(ret, PixelStride, 0, GDIFrameProcessorImpl_->NewImageBuffer.get());
					GDIFrameProcessorImpl_->Data->CaptureEntireMonitor(wholeimg, GDIFrameProcessorImpl_->Data->SelectedMonitor);
				}
				if (GDIFrameProcessorImpl_->Data->CaptureDifMonitor) {
					if (GDIFrameProcessorImpl_->FirstRun) {
						//first time through, just send the whole image
						auto wholeimgfirst = Create(ret, PixelStride, 0, GDIFrameProcessorImpl_->NewImageBuffer.get());
						GDIFrameProcessorImpl_->Data->CaptureDifMonitor(wholeimgfirst, GDIFrameProcessorImpl_->Data->SelectedMonitor);
						GDIFrameProcessorImpl_->FirstRun = false;
					}
					else {
						//user wants difs, lets do it!
						auto newimg = Create(ret, PixelStride, 0, GDIFrameProcessorImpl_->NewImageBuffer.get());
						auto oldimg = Create(ret, PixelStride, 0, GDIFrameProcessorImpl_->OldImageBuffer.get());
						auto imgdifs = GetDifs(oldimg, newimg);

						for (auto& r : imgdifs) {
							auto padding = (r.left *PixelStride) + ((Width(newimg) - r.right)*PixelStride);
							auto startsrc = GDIFrameProcessorImpl_->NewImageBuffer.get();
							startsrc += (r.left *PixelStride) + (r.top *PixelStride *Width(newimg));

							auto difimg = Create(r, PixelStride, padding, startsrc);
							GDIFrameProcessorImpl_->Data->CaptureDifMonitor(difimg, GDIFrameProcessorImpl_->Data->SelectedMonitor);

						}
					}
					std::swap(GDIFrameProcessorImpl_->NewImageBuffer, GDIFrameProcessorImpl_->OldImageBuffer);
				}
				

			}

			return Ret;
		}

	}
}