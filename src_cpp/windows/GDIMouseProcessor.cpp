#include "GDIMouseProcessor.h"
#include "GDIHelpers.h"

namespace SL {
namespace Screen_Capture {

    DUPL_RETURN GDIMouseProcessor::Init(std::shared_ptr<Thread_Data> data)
    {
        auto Ret = DUPL_RETURN_SUCCESS;
        MonitorDC.DC = GetDC(NULL);
        CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);
        NewImageBuffer = std::make_unique<unsigned char[]>(ImageBufferSize);
        ImageBuffer = std::make_unique<unsigned char[]>(ImageBufferSize);
        if (!MonitorDC.DC || !CaptureDC.DC) {
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
        }
        Data = data;
        return Ret;
    }

    DUPL_RETURN GDIMouseProcessor::ProcessFrame()
    {
        auto Ret = DUPL_RETURN_SUCCESS;
        if (Data->ScreenCaptureData.OnMouseChanged || Data->WindowCaptureData.OnMouseChanged) {
            CURSORINFO cursorInfo;
            memset(&cursorInfo, 0, sizeof(cursorInfo));
            cursorInfo.cbSize = sizeof(cursorInfo);

            if (GetCursorInfo(&cursorInfo) == FALSE) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }
            if (!(cursorInfo.flags & CURSOR_SHOWING)) {
                return DUPL_RETURN_SUCCESS; // the mouse cursor is hidden no need to do anything.
            }
            ICONINFOEXA ii;
            memset(&ii, 0, sizeof(ii));
            ii.cbSize = sizeof(ii);
            if (GetIconInfoExA(cursorInfo.hCursor, &ii) == FALSE) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }

            BITMAP bmpCursor = {0};
            ::GetObject(ii.hbmColor, sizeof(bmpCursor), &bmpCursor);

            HBITMAPWrapper colorbmp, maskbmp;
            colorbmp.Bitmap = ii.hbmColor;
            maskbmp.Bitmap = ii.hbmMask;
            HBITMAPWrapper bitmap;
            bitmap.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, bmpCursor.bmWidth, bmpCursor.bmHeight); 

            auto originalBmp = SelectObject(CaptureDC.DC, bitmap.Bitmap); 
            if (DrawIcon(CaptureDC.DC, 0, 0, cursorInfo.hCursor) == FALSE) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }

            ImageRect ret;
            ret.left = ret.top = 0;
            ret.bottom = bmpCursor.bmHeight;
            ret.right = bmpCursor.bmWidth;

            BITMAPINFOHEADER bi;
            memset(&bi, 0, sizeof(bi));

            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = ret.right;
            bi.biHeight = -ret.bottom;
            bi.biPlanes = 1;
            bi.biBitCount = sizeof(ImageBGRA) * 8; // always 32 bits damnit!!!
            bi.biCompression = BI_RGB;
            bi.biSizeImage = ((ret.right * bi.biBitCount + 31) / (sizeof(ImageBGRA) * 8)) * sizeof(ImageBGRA) * ret.bottom;

            auto newsize = sizeof(ImageBGRA) * ret.right * ret.bottom;
            if (static_cast<int>(newsize) > ImageBufferSize || !ImageBuffer || !NewImageBuffer) {
                NewImageBuffer = std::make_unique<unsigned char[]>(newsize);
                ImageBuffer = std::make_unique<unsigned char[]>(newsize);
                ImageBufferSize = newsize;
            }

            GetDIBits(MonitorDC.DC, bitmap.Bitmap, 0, (UINT)ret.bottom, NewImageBuffer.get(), (BITMAPINFO *)&bi, DIB_RGB_COLORS);

            SelectObject(CaptureDC.DC, originalBmp);

            auto wholeimg = CreateImage(ret, ret.right * sizeof(ImageBGRA), (ImageBGRA *)NewImageBuffer.get());

            // need to make sure the alpha channel is correct
            if (ii.wResID == 32513) { // when its just the i beam
                auto ptr = (unsigned int *)NewImageBuffer.get();
                for (auto i = 0; i < Width(wholeimg) * Height(wholeimg); i++) {
                    if (ptr[i] != 0) {
                        ptr[i] = 0xff000000;
                    }
                }
            }
            // else if (ii.hbmMask != nullptr && ii.hbmColor == nullptr) {// just
            //	auto ptr = (unsigned int*)NewImageBuffer.get();
            //	for (auto i = 0; i < RowStride(*wholeimg) *Height(*wholeimg) / 4; i++) {
            //		if (ptr[i] != 0) {
            //			ptr[i] = ptr[i] | 0xffffffff;
            //		}
            //	}
            //}

            int lastx = static_cast<int>(cursorInfo.ptScreenPos.x);
            int lasty = static_cast<int>(cursorInfo.ptScreenPos.y);
            MousePoint mousepoint = {};
            mousepoint.Position = Point{lastx, lasty};
            mousepoint.HotSpot = Point{static_cast<int>(ii.xHotspot), static_cast<int>(ii.yHotspot)};

            // if the mouse image is different, send the new image and swap the data
            if (memcmp(NewImageBuffer.get(), ImageBuffer.get(), bi.biSizeImage) != 0) {
                if (Data->WindowCaptureData.OnMouseChanged) {
                    Data->WindowCaptureData.OnMouseChanged(&wholeimg, mousepoint);
                }
                if (Data->ScreenCaptureData.OnMouseChanged) {
                    Data->ScreenCaptureData.OnMouseChanged(&wholeimg, mousepoint);
                }
                std::swap(NewImageBuffer, ImageBuffer);
            }
            else if (Last_x != lastx || Last_y != lasty) {
                if (Data->WindowCaptureData.OnMouseChanged) {
                    Data->WindowCaptureData.OnMouseChanged(nullptr, mousepoint);
                }
                if (Data->ScreenCaptureData.OnMouseChanged) {
                    Data->ScreenCaptureData.OnMouseChanged(nullptr, mousepoint);
                }
            }

            Last_x = lastx;
            Last_y = lasty;
        }

        return Ret;
    }

} // namespace Screen_Capture
} // namespace SL