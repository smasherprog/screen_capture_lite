#include "GDIMouseProcessor.h"
#include "GDIHelpers.h"

namespace SL {
	namespace Screen_Capture {

		struct GDIMouseProcessorImpl {
			const int MaxCursurorSize = 32;
			HDCWrapper MonitorDC;
			HDCWrapper CaptureDC;
			std::shared_ptr<Mouse_Thread_Data> Data;
			std::unique_ptr<char[]> NewImageBuffer, LastImageBuffer;
			size_t ImageBufferSize;
			bool FirstRun;
			int Last_x, Last_y;

		};


		GDIMouseProcessor::GDIMouseProcessor()
		{
			_GDIMouseProcessorImpl = std::make_unique<GDIMouseProcessorImpl>();
			_GDIMouseProcessorImpl->ImageBufferSize = 0;
			_GDIMouseProcessorImpl->FirstRun = true;
			_GDIMouseProcessorImpl->Last_x = _GDIMouseProcessorImpl->Last_y = 0;
		}

		GDIMouseProcessor::~GDIMouseProcessor()
		{

		}
		DUPL_RETURN GDIMouseProcessor::Init(std::shared_ptr<Mouse_Thread_Data> data) {
			auto Ret = DUPL_RETURN_SUCCESS;
			_GDIMouseProcessorImpl->MonitorDC.DC = GetDC(NULL);
			_GDIMouseProcessorImpl->CaptureDC.DC = CreateCompatibleDC(_GDIMouseProcessorImpl->MonitorDC.DC);

			if (!_GDIMouseProcessorImpl->MonitorDC.DC || !_GDIMouseProcessorImpl->CaptureDC.DC) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}
			_GDIMouseProcessorImpl->Data = data;
			_GDIMouseProcessorImpl->ImageBufferSize = _GDIMouseProcessorImpl->MaxCursurorSize* _GDIMouseProcessorImpl->MaxCursurorSize* PixelStride;
			_GDIMouseProcessorImpl->NewImageBuffer = std::make_unique<char[]>(_GDIMouseProcessorImpl->ImageBufferSize);
			_GDIMouseProcessorImpl->LastImageBuffer = std::make_unique<char[]>(_GDIMouseProcessorImpl->ImageBufferSize);
			return Ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN GDIMouseProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;


			CURSORINFO cursorInfo;
			memset(&cursorInfo, 0, sizeof(cursorInfo));
			cursorInfo.cbSize = sizeof(cursorInfo);

			if (GetCursorInfo(&cursorInfo) == FALSE) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}
			if (!(cursorInfo.flags & CURSOR_SHOWING)) {
				return DUPL_RETURN_SUCCESS;// the mouse cursor is hidden no need to do anything.
			}
			ICONINFOEXA ii;
			memset(&ii, 0, sizeof(ii));
			ii.cbSize = sizeof(ii);
			if (GetIconInfoExA(cursorInfo.hCursor, &ii) == FALSE) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}
			HBITMAPWrapper colorbmp, maskbmp;
			colorbmp.Bitmap = ii.hbmColor;
			maskbmp.Bitmap = ii.hbmMask;
			HBITMAPWrapper bitmap;
			bitmap.Bitmap = CreateCompatibleBitmap(_GDIMouseProcessorImpl->MonitorDC.DC, _GDIMouseProcessorImpl->MaxCursurorSize, _GDIMouseProcessorImpl->MaxCursurorSize);

			auto originalBmp = SelectObject(_GDIMouseProcessorImpl->CaptureDC.DC, bitmap.Bitmap);
			if (DrawIcon(_GDIMouseProcessorImpl->CaptureDC.DC, 0, 0, cursorInfo.hCursor) == FALSE) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}

			ImageRect ret;
			ret.left = ret.top = 0;
			ret.bottom = _GDIMouseProcessorImpl->MaxCursurorSize;
			ret.right = _GDIMouseProcessorImpl->MaxCursurorSize;

			BITMAPINFOHEADER bi;
			memset(&bi, 0, sizeof(bi));

			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = ret.right;
			bi.biHeight = -ret.bottom;
			bi.biPlanes = 1;
			bi.biBitCount = PixelStride * 8; //always 32 bits damnit!!!
			bi.biCompression = BI_RGB;
			bi.biSizeImage = ((ret.right * bi.biBitCount + 31) / (PixelStride * 8)) * PixelStride* ret.bottom;

			GetDIBits(_GDIMouseProcessorImpl->MonitorDC.DC, bitmap.Bitmap, 0, (UINT)ret.bottom, _GDIMouseProcessorImpl->NewImageBuffer.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

			SelectObject(_GDIMouseProcessorImpl->CaptureDC.DC, originalBmp);

			auto wholeimg = Create(ret, PixelStride, 0, _GDIMouseProcessorImpl->NewImageBuffer.get());

			//need to make sure the alpha channel is correct
			if (ii.wResID == 32513) { // when its just the i beam
				auto ptr = (unsigned int*)_GDIMouseProcessorImpl->NewImageBuffer.get();
				for (auto i = 0; i < RowStride(wholeimg) *Height(wholeimg) / 4; i++) {
					if (ptr[i] != 0) {
						ptr[i] = 0xff000000;
					}
				}
			}
			//else if (ii.hbmMask != nullptr && ii.hbmColor == nullptr) {// just 
			//	auto ptr = (unsigned int*)_GDIMouseProcessorImpl->NewImageBuffer.get();
			//	for (auto i = 0; i < RowStride(*wholeimg) *Height(*wholeimg) / 4; i++) {
			//		if (ptr[i] != 0) {
			//			ptr[i] = ptr[i] | 0xffffffff;
			//		}
			//	}
			//}

			if (_GDIMouseProcessorImpl->Data->CaptureCallback) {
				int lastx = static_cast<int>(cursorInfo.ptScreenPos.x - ii.xHotspot);
				int lasty = static_cast<int>(cursorInfo.ptScreenPos.y - ii.yHotspot);
				//if the mouse image is different, send the new image and swap the data 
				if (memcmp(_GDIMouseProcessorImpl->NewImageBuffer.get(), _GDIMouseProcessorImpl->LastImageBuffer.get(), bi.biSizeImage) != 0) {
					_GDIMouseProcessorImpl->Data->CaptureCallback(&wholeimg, lastx, lasty);
					std::swap(_GDIMouseProcessorImpl->NewImageBuffer, _GDIMouseProcessorImpl->LastImageBuffer);
				}
				else if(_GDIMouseProcessorImpl->Last_x != lastx || _GDIMouseProcessorImpl->Last_y != lasty){
					_GDIMouseProcessorImpl->Data->CaptureCallback(nullptr, lastx, lasty);
				}
				_GDIMouseProcessorImpl->Last_x = lastx;
				_GDIMouseProcessorImpl->Last_y = lasty;
			}
			return Ret;
		}

	}
}