#include "GDIMouseProcessor.h"
#include "GDIHelpers.h"

namespace SL {
    namespace Screen_Capture {

        DUPL_RETURN GDIMouseProcessor::Init(std::shared_ptr<Thread_Data> data) {
            auto Ret = DUPL_RETURN_SUCCESS;
            MonitorDC.DC = GetDC(NULL);
            CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);

            if (!MonitorDC.DC || !CaptureDC.DC) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }
            Data = data;
            return Ret;
        }
        DUPL_RETURN GDIMouseProcessor::Init(std::shared_ptr<Thread_Data> data, const Window& selectedwindow) {
            SelectedWindow = reinterpret_cast<HWND>(selectedwindow.Handle);
            return Init(data);
        }

        DUPL_RETURN GDIMouseProcessor::Process(const Window* wnd)
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
            bitmap.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, MaxCursurorSize, MaxCursurorSize);

            auto originalBmp = SelectObject(CaptureDC.DC, bitmap.Bitmap);
            if (DrawIcon(CaptureDC.DC, 0, 0, cursorInfo.hCursor) == FALSE) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }

            ImageRect ret;
            ret.left = ret.top = 0;
            ret.bottom = MaxCursurorSize;
            ret.right = MaxCursurorSize;

            BITMAPINFOHEADER bi;
            memset(&bi, 0, sizeof(bi));

            bi.biSize = sizeof(BITMAPINFOHEADER);
            bi.biWidth = ret.right;
            bi.biHeight = -ret.bottom;
            bi.biPlanes = 1;
            bi.biBitCount = PixelStride * 8; //always 32 bits damnit!!!
            bi.biCompression = BI_RGB;
            bi.biSizeImage = ((ret.right * bi.biBitCount + 31) / (PixelStride * 8)) * PixelStride* ret.bottom;

            GetDIBits(MonitorDC.DC, bitmap.Bitmap, 0, (UINT)ret.bottom, NewImageBuffer.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

            SelectObject(CaptureDC.DC, originalBmp);

            auto wholeimg = Create(ret, PixelStride, 0, NewImageBuffer.get());

            //need to make sure the alpha channel is correct
            if (ii.wResID == 32513) { // when its just the i beam
                auto ptr = (unsigned int*)NewImageBuffer.get();
                for (auto i = 0; i < RowStride(wholeimg) *Height(wholeimg) / 4; i++) {
                    if (ptr[i] != 0) {
                        ptr[i] = 0xff000000;
                    }
                }
            }
            //else if (ii.hbmMask != nullptr && ii.hbmColor == nullptr) {// just 
            //	auto ptr = (unsigned int*)NewImageBuffer.get();
            //	for (auto i = 0; i < RowStride(*wholeimg) *Height(*wholeimg) / 4; i++) {
            //		if (ptr[i] != 0) {
            //			ptr[i] = ptr[i] | 0xffffffff;
            //		}
            //	}
            //}
            int lastx = static_cast<int>(cursorInfo.ptScreenPos.x - ii.xHotspot);
            int lasty = static_cast<int>(cursorInfo.ptScreenPos.y - ii.yHotspot);

            if (wnd == nullptr && Data->ScreenCaptureData.OnMouseChanged) {
                //if the mouse image is different, send the new image and swap the data 
                if (memcmp(NewImageBuffer.get(), OldImageBuffer.get(), bi.biSizeImage) != 0) {
                    Data->ScreenCaptureData.OnMouseChanged(&wholeimg, Point{ lastx, lasty  });
                    std::swap(NewImageBuffer, OldImageBuffer);
                }
                else if (Last_x != lastx || Last_y != lasty) {
                    Data->ScreenCaptureData.OnMouseChanged(nullptr, Point{ lastx, lasty });
                }

            }
            else if (wnd != nullptr && Data->WindowCaptureData.OnMouseChanged) {

                RECT windowRect;
                GetWindowRect(SelectedWindow, &windowRect);
                POINT cursorPos = { cursorInfo.ptScreenPos.x , cursorInfo.ptScreenPos.y };
      
                //if the mouse is over the window continue!
                if (PtInRect(&windowRect, cursorPos) == TRUE) {
                    if (memcmp(NewImageBuffer.get(), OldImageBuffer.get(), bi.biSizeImage) != 0) {
                        Data->WindowCaptureData.OnMouseChanged(&wholeimg, Point{ lastx - windowRect.left, lasty - windowRect.bottom }, *wnd);
                        std::swap(NewImageBuffer, OldImageBuffer);
                    } if (Last_x != lastx || Last_y != lasty) {
                        Data->WindowCaptureData.OnMouseChanged(nullptr, Point{ lastx - windowRect.left, lasty - windowRect.bottom }, *wnd);
                    }
                }
                else  if (memcmp(NewImageBuffer.get(), OldImageBuffer.get(), bi.biSizeImage) != 0) {
                    Data->WindowCaptureData.OnMouseChanged(&wholeimg, Point{ lastx - windowRect.left, lasty - windowRect.bottom }, *wnd);
                    std::swap(NewImageBuffer, OldImageBuffer);
                }
            }

            Last_x = lastx;
            Last_y = lasty;
            return Ret;
        }
        DUPL_RETURN GDIMouseProcessor::ProcessFrame(const Window& selectedwindow)
        {
            return Process(&selectedwindow);
        }
        DUPL_RETURN GDIMouseProcessor::ProcessFrame()
        {
            return Process(nullptr);
        }
    }
}