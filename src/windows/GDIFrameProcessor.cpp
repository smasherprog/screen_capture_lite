#include "GDIFrameProcessor.h"
#include "GDIHelpers.h"

namespace SL {
    namespace Screen_Capture {

        struct GDIFrameProcessorImpl {

            HDCWrapper MonitorDC;
            HDCWrapper CaptureDC;
            HBITMAPWrapper CaptureBMP;

            std::shared_ptr<Monitor_Thread_Data> Data;

        };


        GDIFrameProcessor::GDIFrameProcessor()
        {
            GDIFrameProcessorImpl_ = std::make_unique<GDIFrameProcessorImpl>();
        }

        GDIFrameProcessor::~GDIFrameProcessor()
        {

        }
        DUPL_RETURN GDIFrameProcessor::Init(std::shared_ptr<Monitor_Thread_Data> data) {
            auto Ret = DUPL_RETURN_SUCCESS;

            GDIFrameProcessorImpl_->MonitorDC.DC = CreateDCA(Name(data->SelectedMonitor), NULL, NULL, NULL);
            GDIFrameProcessorImpl_->CaptureDC.DC = CreateCompatibleDC(GDIFrameProcessorImpl_->MonitorDC.DC);
            GDIFrameProcessorImpl_->CaptureBMP.Bitmap = CreateCompatibleBitmap(GDIFrameProcessorImpl_->MonitorDC.DC, Width(data->SelectedMonitor), Height(data->SelectedMonitor));

            if (!GDIFrameProcessorImpl_->MonitorDC.DC || !GDIFrameProcessorImpl_->CaptureDC.DC || !GDIFrameProcessorImpl_->CaptureBMP.Bitmap) {
                return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
            }

            GDIFrameProcessorImpl_->Data = data;
            if ((data->CaptureEntireMonitor) && !data->NewImageBuffer) {
                data->NewImageBuffer = std::make_unique<char[]>(data->ImageBufferSize);
            }
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
            if (EnumDisplaySettingsA(Name(GDIFrameProcessorImpl_->Data->SelectedMonitor), ENUM_CURRENT_SETTINGS, &devMode) == TRUE) {
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
                GetDIBits(GDIFrameProcessorImpl_->MonitorDC.DC, GDIFrameProcessorImpl_->CaptureBMP.Bitmap, 0, (UINT)ret.bottom, GDIFrameProcessorImpl_->Data->NewImageBuffer.get(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
                SelectObject(GDIFrameProcessorImpl_->CaptureDC.DC, originalBmp);
                ProcessMonitorCapture(*GDIFrameProcessorImpl_->Data, ret);
            }

            return Ret;
        }

    }
}