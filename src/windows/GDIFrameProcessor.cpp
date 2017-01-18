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
			_GDIFrameProcessorImpl = std::make_unique<GDIFrameProcessorImpl>();
			_GDIFrameProcessorImpl->ImageBufferSize = 0;
			_GDIFrameProcessorImpl->FirstRun = true;
		}

		GDIFrameProcessor::~GDIFrameProcessor()
		{

		}
		DUPL_RETURN GDIFrameProcessor::Init(std::shared_ptr<Monitor_Thread_Data> data) {
			auto Ret = DUPL_RETURN_SUCCESS;
			_GDIFrameProcessorImpl->MonitorName = Name(data->SelectedMonitor);
			_GDIFrameProcessorImpl->MonitorDC.DC = CreateDCA(_GDIFrameProcessorImpl->MonitorName.c_str(), NULL, NULL, NULL);
			_GDIFrameProcessorImpl->CaptureDC.DC = CreateCompatibleDC(_GDIFrameProcessorImpl->MonitorDC.DC);
			_GDIFrameProcessorImpl->CaptureBMP.Bitmap = CreateCompatibleBitmap(_GDIFrameProcessorImpl->MonitorDC.DC, Width(data->SelectedMonitor), Height(data->SelectedMonitor));

			if (!_GDIFrameProcessorImpl->MonitorDC.DC || !_GDIFrameProcessorImpl->CaptureDC.DC || !_GDIFrameProcessorImpl->CaptureBMP.Bitmap) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}
			_GDIFrameProcessorImpl->Data = data;
			_GDIFrameProcessorImpl->ImageBufferSize = Width(data->SelectedMonitor)* Height(data->SelectedMonitor)* PixelStride;
			if (_GDIFrameProcessorImpl->Data->CaptureDifMonitor) {//only need the old buffer if difs are needed. If no dif is needed, then the image is always new
				_GDIFrameProcessorImpl->OldImageBuffer = std::make_unique<char[]>(_GDIFrameProcessorImpl->ImageBufferSize);
			}
			_GDIFrameProcessorImpl->NewImageBuffer = std::make_unique<char[]>(_GDIFrameProcessorImpl->ImageBufferSize);

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
			ret.bottom = Height(_GDIFrameProcessorImpl->Data->SelectedMonitor);
			ret.right = Width(_GDIFrameProcessorImpl->Data->SelectedMonitor);

			DEVMODEA devMode;
			devMode.dmSize = sizeof(devMode);
			if(EnumDisplaySettingsA(_GDIFrameProcessorImpl->MonitorName.c_str(), ENUM_CURRENT_SETTINGS, &devMode)==TRUE){
				if (static_cast<int>(devMode.dmPelsHeight) != ret.bottom || static_cast<int>(devMode.dmPelsWidth) != ret.right) {
					return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
				}
			}

			// Selecting an object into the specified DC
			auto originalBmp = SelectObject(_GDIFrameProcessorImpl->CaptureDC.DC, _GDIFrameProcessorImpl->CaptureBMP.Bitmap);

			if (BitBlt(_GDIFrameProcessorImpl->CaptureDC.DC, 0, 0, ret.right, ret.bottom, _GDIFrameProcessorImpl->MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
				//if the screen cannot be captured, return
				SelectObject(_GDIFrameProcessorImpl->CaptureDC.DC, originalBmp);
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

				GetDIBits(_GDIFrameProcessorImpl->MonitorDC.DC, _GDIFrameProcessorImpl->CaptureBMP.Bitmap, 0, (UINT)ret.bottom, _GDIFrameProcessorImpl->NewImageBuffer.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

				SelectObject(_GDIFrameProcessorImpl->CaptureDC.DC, originalBmp);
				if (_GDIFrameProcessorImpl->Data->CaptureEntireMonitor) {

					auto wholeimg = Create(ret, PixelStride, 0, _GDIFrameProcessorImpl->NewImageBuffer.get());
					_GDIFrameProcessorImpl->Data->CaptureEntireMonitor(wholeimg, _GDIFrameProcessorImpl->Data->SelectedMonitor);
				}
				if (_GDIFrameProcessorImpl->Data->CaptureDifMonitor) {
					if (_GDIFrameProcessorImpl->FirstRun) {
						//first time through, just send the whole image
						auto wholeimgfirst = Create(ret, PixelStride, 0, _GDIFrameProcessorImpl->NewImageBuffer.get());
						_GDIFrameProcessorImpl->Data->CaptureDifMonitor(wholeimgfirst, _GDIFrameProcessorImpl->Data->SelectedMonitor);
						_GDIFrameProcessorImpl->FirstRun = false;
					}
					else {
						//user wants difs, lets do it!
						auto newimg = Create(ret, PixelStride, 0, _GDIFrameProcessorImpl->NewImageBuffer.get());
						auto oldimg = Create(ret, PixelStride, 0, _GDIFrameProcessorImpl->OldImageBuffer.get());
						auto imgdifs = GetDifs(oldimg, newimg);

						for (auto& r : imgdifs) {
							auto padding = (r.left *PixelStride) + ((Width(newimg) - r.right)*PixelStride);
							auto startsrc = _GDIFrameProcessorImpl->NewImageBuffer.get();
							startsrc += (r.left *PixelStride) + (r.top *PixelStride *Width(newimg));

							auto difimg = Create(r, PixelStride, padding, startsrc);
							_GDIFrameProcessorImpl->Data->CaptureDifMonitor(difimg, _GDIFrameProcessorImpl->Data->SelectedMonitor);

						}
					}

					std::swap(_GDIFrameProcessorImpl->NewImageBuffer, _GDIFrameProcessorImpl->OldImageBuffer);
				}

			}

			return Ret;
		}

	}
}