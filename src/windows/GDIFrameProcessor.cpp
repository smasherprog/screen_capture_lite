#include "GDIFrameProcessor.h"

namespace SL {
    namespace Screen_Capture {

        DUPL_RETURN GDIFrameProcessor::Init(std::shared_ptr<Thread_Data> data, Monitor& monitor) {
            SelectedMonitor = monitor;
            auto Ret = DUPL_RETURN_SUCCESS;

            MonitorDC.DC = CreateDCA(Name(SelectedMonitor), NULL, NULL, NULL);
            CaptureDC.DC = CreateCompatibleDC(MonitorDC.DC);
            CaptureBMP.Bitmap = CreateCompatibleBitmap(MonitorDC.DC, Width(SelectedMonitor), Height(SelectedMonitor));

            if (!MonitorDC.DC || !CaptureDC.DC || !CaptureBMP.Bitmap) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }

            Data = data;
            return Ret;
        }
        //
        // Process a given frame and its metadata
        //
        DUPL_RETURN GDIFrameProcessor::ProcessFrame(const Monitor& currentmonitorinfo)
        {
            
            auto Ret = DUPL_RETURN_SUCCESS;

            ImageRect ret;
            ret.left = ret.top = 0;
            ret.bottom = Height(SelectedMonitor);
            ret.right = Width(SelectedMonitor);

            DEVMODEA devMode;
            devMode.dmSize = sizeof(devMode);
            if (EnumDisplaySettingsA(Name(SelectedMonitor), ENUM_CURRENT_SETTINGS, &devMode) == FALSE) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }

            // Selecting an object into the specified DC
            auto originalBmp = SelectObject(CaptureDC.DC, CaptureBMP.Bitmap);

            if (BitBlt(CaptureDC.DC, 0, 0, ret.right, ret.bottom, MonitorDC.DC, OffsetX(SelectedMonitor) - devMode.dmPosition.x, OffsetY(SelectedMonitor), SRCCOPY | CAPTUREBLT) == FALSE) {
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
                ProcessMonitorCapture(*Data, *this, SelectedMonitor, ret);
            }

            return Ret;
        }

    }
}