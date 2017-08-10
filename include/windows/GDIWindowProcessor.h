#pragma once
#include "ScreenCapture.h"
#include "SCCommon.h"
#include <memory>
#include "GDIHelpers.h"

namespace SL {
    namespace Screen_Capture {

        class GDIWindowProcessor : public BaseFrameProcessor {
            HDCWrapper MonitorDC;
            HDCWrapper CaptureDC;
            HBITMAPWrapper CaptureBMP;
            Window Window_;
            HWND SelectedWindow;
            std::shared_ptr<Thread_Data> Data;
        public:
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, Window selectedwindow);
            DUPL_RETURN ProcessFrame(Window selectedwindow);

        };
    }
}