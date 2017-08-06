#include "GDIWindowProcessor.h"

namespace SL {
    namespace Screen_Capture {

        DUPL_RETURN GDIWindowProcessor::Init(std::shared_ptr<Thread_Data> data, Window selectedwindow) {
            SelectedWindow = reinterpret_cast<HWND>(selectedwindow.Handle);
            auto Ret = DUPL_RETURN_SUCCESS;

            MonitorDC.DC = GetWindowDC(SelectedWindow);
            CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);
            RECT r;
            GetWindowRect(SelectedWindow, &r);
            auto width = r.right - r.left;
            auto height = r.bottom - r.top;

            CaptureBMP.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, width, height);

            if (!MonitorDC.DC || !CaptureDC.DC || !CaptureBMP.Bitmap) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }

            Data = data;
            return Ret;
        }



        DUPL_RETURN GDIWindowProcessor::ProcessFrame(Window selectedwindow)
        {

            auto Ret = DUPL_RETURN_SUCCESS;
            RECT r;
            GetWindowRect(SelectedWindow, &r);
            ImageRect ret;
            ret.left = ret.top = 0;
            ret.bottom = r.bottom - r.top;
            ret.right = r.right - r.left;
            if (selectedwindow.Height != ret.bottom || selectedwindow.Width != ret.right) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;//window size changed. This will rebuild everything
            }

            // Selecting an object into the specified DC
            auto originalBmp = SelectObject(CaptureDC.DC, CaptureBMP.Bitmap);

            if (BitBlt(CaptureDC.DC, 0, 0, ret.right, ret.bottom, MonitorDC.DC, 0, 0, SRCCOPY | CAPTUREBLT) == FALSE) {
                //if the screen cannot be captured, return
                SelectObject(CaptureDC.DC, originalBmp);
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
                GetDIBits(MonitorDC.DC, CaptureBMP.Bitmap, 0, (UINT)ret.bottom, NewImageBuffer.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
                SelectObject(CaptureDC.DC, originalBmp);
                ProcessWindowCapture(*Data, *this, ret);
            }

            return Ret;
        }

    }
}