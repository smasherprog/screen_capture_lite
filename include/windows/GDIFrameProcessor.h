#pragma once
#include "ScreenCapture.h"
#include "SCCommon.h"
#include <memory>
#include "GDIHelpers.h"

namespace SL {
    namespace Screen_Capture {

        class GDIFrameProcessor : public BaseFrameProcessor {
            HDCWrapper MonitorDC;
            HDCWrapper CaptureDC;
            HBITMAPWrapper CaptureBMP;
            Monitor SelectedMonitor;
            std::shared_ptr<Thread_Data> Data;
        public:
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, Monitor& monitor);
            DUPL_RETURN ProcessFrame();

        };
    }
}