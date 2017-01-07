#include "GDIFrameProcessor.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace SL {
	namespace Screen_Capture {
		class HDCWrapper {
		public:
			HDCWrapper() : DC(nullptr) {}
			~HDCWrapper() { if (DC != nullptr) { DeleteDC(DC); } }
			HDC DC;
		};
		class HBITMAPWrapper {
		public:
			HBITMAPWrapper() : Bitmap(nullptr) {}
			~HBITMAPWrapper() { if (Bitmap != nullptr) { DeleteObject(Bitmap); } }
			HBITMAP Bitmap;
		};

		struct GDIFrameProcessorImpl {
			
			HDCWrapper MonitorDC;
			HDCWrapper CaptureDC;
			HBITMAPWrapper CaptureBMP;
			std::shared_ptr<THREAD_DATA> Data;
			std::unique_ptr<char[]> OldImageBuffer, NewImageBuffer;
			size_t ImageBufferSize;
		};


		GDIFrameProcessor::GDIFrameProcessor()
		{
			_GDIFrameProcessorImpl = std::make_unique<GDIFrameProcessorImpl>();
			_GDIFrameProcessorImpl->ImageBufferSize = 0;
		}

		GDIFrameProcessor::~GDIFrameProcessor()
		{

		}
		DUPL_RETURN GDIFrameProcessor::Init(std::shared_ptr<THREAD_DATA> data) {
			auto Ret = DUPL_RETURN_SUCCESS;
			auto name = Name(*data->SelectedMonitor);
			_GDIFrameProcessorImpl->MonitorDC.DC = CreateDCA(name.c_str(), NULL, NULL, NULL);
			_GDIFrameProcessorImpl->CaptureDC.DC = CreateCompatibleDC(_GDIFrameProcessorImpl->MonitorDC.DC);
			_GDIFrameProcessorImpl->CaptureBMP.Bitmap = CreateCompatibleBitmap(_GDIFrameProcessorImpl->MonitorDC.DC, Width(*data->SelectedMonitor), Height(*data->SelectedMonitor));

			if (!_GDIFrameProcessorImpl->MonitorDC.DC || !_GDIFrameProcessorImpl->CaptureDC.DC || !_GDIFrameProcessorImpl->CaptureBMP.Bitmap) {
				Ret = DUPL_RETURN::DUPL_RETURN_ERROR_UNEXPECTED;
			}
			_GDIFrameProcessorImpl->Data = data;
			_GDIFrameProcessorImpl->ImageBufferSize = Width(*data->SelectedMonitor)* Height(*data->SelectedMonitor)* PixelStride;
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
			ret.bottom = Height(*_GDIFrameProcessorImpl->Data->SelectedMonitor);
			ret.right = Width(*_GDIFrameProcessorImpl->Data->SelectedMonitor);

			// Selecting an object into the specified DC
			auto originalBmp = SelectObject(_GDIFrameProcessorImpl->CaptureDC.DC, _GDIFrameProcessorImpl->CaptureBMP.Bitmap);
			
			if (BitBlt(_GDIFrameProcessorImpl->CaptureDC.DC, 0, 0, ret.right, ret.bottom, _GDIFrameProcessorImpl->MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
				//if the screen cannot be captured, return
				SelectObject(_GDIFrameProcessorImpl->CaptureDC.DC, originalBmp);
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
			
				GetDIBits(_GDIFrameProcessorImpl->MonitorDC.DC, _GDIFrameProcessorImpl->CaptureBMP.Bitmap, 0, (UINT)ret.bottom, _GDIFrameProcessorImpl->NewImageBuffer.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

				SelectObject(_GDIFrameProcessorImpl->CaptureDC.DC, originalBmp);
				if (_GDIFrameProcessorImpl->Data->CaptureEntireMonitor) {
				
					auto wholeimg = CreateImage(ret, PixelStride, 0, _GDIFrameProcessorImpl->NewImageBuffer.get());
					_GDIFrameProcessorImpl->Data->CaptureEntireMonitor(*wholeimg, *_GDIFrameProcessorImpl->Data->SelectedMonitor);
				}
				if (_GDIFrameProcessorImpl->Data->CaptureDifMonitor) {
					//user wants difs, lets do it!
					auto newimg = CreateImage(ret, PixelStride, 0, _GDIFrameProcessorImpl->NewImageBuffer.get());
					auto oldimg = CreateImage(ret, PixelStride, 0, _GDIFrameProcessorImpl->OldImageBuffer.get());
					auto imgdifs = GetDifs(*oldimg, *newimg);

					for (auto& r : imgdifs) {
						auto padding = (r.left *PixelStride) + ((Width(*newimg) - r.right)*PixelStride);
						auto startsrc = _GDIFrameProcessorImpl->NewImageBuffer.get();
						startsrc += (r.left *PixelStride) + (r.top *PixelStride *Width(*newimg));

						auto difimg = CreateImage(r, PixelStride, padding, startsrc);
						_GDIFrameProcessorImpl->Data->CaptureDifMonitor(*difimg, *_GDIFrameProcessorImpl->Data->SelectedMonitor);

					}
					std::swap(_GDIFrameProcessorImpl->NewImageBuffer, _GDIFrameProcessorImpl->OldImageBuffer);
				}
				
			}

			return Ret;
		}

	}
}