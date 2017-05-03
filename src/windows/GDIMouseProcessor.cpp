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
            GDIMouseProcessorImpl_ = std::make_unique<GDIMouseProcessorImpl>();
            GDIMouseProcessorImpl_->ImageBufferSize = 0;
            GDIMouseProcessorImpl_->FirstRun = true;
            GDIMouseProcessorImpl_->Last_x = GDIMouseProcessorImpl_->Last_y = 0;
        }

        GDIMouseProcessor::~GDIMouseProcessor()
        {

        }
        DUPL_RETURN GDIMouseProcessor::Init(std::shared_ptr<Mouse_Thread_Data> data) {
            auto Ret = DUPL_RETURN_SUCCESS;
            GDIMouseProcessorImpl_->MonitorDC.DC = GetDC(NULL);
            GDIMouseProcessorImpl_->CaptureDC.DC = CreateCompatibleDC(GDIMouseProcessorImpl_->MonitorDC.DC);

            if (!GDIMouseProcessorImpl_->MonitorDC.DC || !GDIMouseProcessorImpl_->CaptureDC.DC) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }
            GDIMouseProcessorImpl_->Data = data;
            GDIMouseProcessorImpl_->ImageBufferSize = GDIMouseProcessorImpl_->MaxCursurorSize* GDIMouseProcessorImpl_->MaxCursurorSize* PixelStride;
            GDIMouseProcessorImpl_->NewImageBuffer = std::make_unique<char[]>(GDIMouseProcessorImpl_->ImageBufferSize);
            GDIMouseProcessorImpl_->LastImageBuffer = std::make_unique<char[]>(GDIMouseProcessorImpl_->ImageBufferSize);
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
            bitmap.Bitmap = CreateCompatibleBitmap(GDIMouseProcessorImpl_->MonitorDC.DC, GDIMouseProcessorImpl_->MaxCursurorSize, GDIMouseProcessorImpl_->MaxCursurorSize);

            auto originalBmp = SelectObject(GDIMouseProcessorImpl_->CaptureDC.DC, bitmap.Bitmap);
            if (DrawIcon(GDIMouseProcessorImpl_->CaptureDC.DC, 0, 0, cursorInfo.hCursor) == FALSE) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }

            ImageRect ret;
            ret.left = ret.top = 0;
            ret.bottom = GDIMouseProcessorImpl_->MaxCursurorSize;
            ret.right = GDIMouseProcessorImpl_->MaxCursurorSize;

            BITMAPINFOHEADER bi;
            memset(&bi, 0, sizeof(bi));

            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = ret.right;
            bi.biHeight = -ret.bottom;
            bi.biPlanes = 1;
            bi.biBitCount = PixelStride * 8; //always 32 bits damnit!!!
            bi.biCompression = BI_RGB;
            bi.biSizeImage = ((ret.right * bi.biBitCount + 31) / (PixelStride * 8)) * PixelStride* ret.bottom;

            GetDIBits(GDIMouseProcessorImpl_->MonitorDC.DC, bitmap.Bitmap, 0, (UINT)ret.bottom, GDIMouseProcessorImpl_->NewImageBuffer.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

            SelectObject(GDIMouseProcessorImpl_->CaptureDC.DC, originalBmp);

            auto wholeimg = Create(ret, PixelStride, 0, GDIMouseProcessorImpl_->NewImageBuffer.get());

            //need to make sure the alpha channel is correct
            if (ii.wResID == 32513) { // when its just the i beam
                auto ptr = (unsigned int*)GDIMouseProcessorImpl_->NewImageBuffer.get();
                for (auto i = 0; i < RowStride(wholeimg) *Height(wholeimg) / 4; i++) {
                    if (ptr[i] != 0) {
                        ptr[i] = 0xff000000;
                    }
                }
            }
            //else if (ii.hbmMask != nullptr && ii.hbmColor == nullptr) {// just 
            //	auto ptr = (unsigned int*)GDIMouseProcessorImpl_->NewImageBuffer.get();
            //	for (auto i = 0; i < RowStride(*wholeimg) *Height(*wholeimg) / 4; i++) {
            //		if (ptr[i] != 0) {
            //			ptr[i] = ptr[i] | 0xffffffff;
            //		}
            //	}
            //}

            if (GDIMouseProcessorImpl_->Data->CaptureCallback) {
                int lastx = static_cast<int>(cursorInfo.ptScreenPos.x - ii.xHotspot);
                int lasty = static_cast<int>(cursorInfo.ptScreenPos.y - ii.yHotspot);
                //if the mouse image is different, send the new image and swap the data 
                if (memcmp(GDIMouseProcessorImpl_->NewImageBuffer.get(), GDIMouseProcessorImpl_->LastImageBuffer.get(), bi.biSizeImage) != 0) {
                    GDIMouseProcessorImpl_->Data->CaptureCallback(&wholeimg, lastx, lasty);
                    std::swap(GDIMouseProcessorImpl_->NewImageBuffer, GDIMouseProcessorImpl_->LastImageBuffer);
                }
                else if (GDIMouseProcessorImpl_->Last_x != lastx || GDIMouseProcessorImpl_->Last_y != lasty) {
                    GDIMouseProcessorImpl_->Data->CaptureCallback(nullptr, lastx, lasty);
                }
                GDIMouseProcessorImpl_->Last_x = lastx;
                GDIMouseProcessorImpl_->Last_y = lasty;
            }
            return Ret;
        }

    }
}