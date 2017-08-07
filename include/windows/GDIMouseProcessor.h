#pragma once
#include "ScreenCapture.h"
#include "SCCommon.h"
#include <memory>
#include "GDIHelpers.h"

namespace SL {
    namespace Screen_Capture {

        class GDIMouseProcessor : public BaseFrameProcessor {

            HDCWrapper MonitorDC;
            HDCWrapper CaptureDC;
            std::shared_ptr<Thread_Data> Data;
            HWND SelectedWindow;

            int Last_x = 0;
            int Last_y = 0;
            DUPL_RETURN Process(const Window* wnd);

        public:

            const int MaxCursurorSize = 32;
            
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Window& selectedwindow);
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data);

            DUPL_RETURN ProcessFrame(const Window& selectedwindow);
            DUPL_RETURN ProcessFrame();

      
        };
    }
}