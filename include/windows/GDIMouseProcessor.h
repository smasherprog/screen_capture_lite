#pragma once
#include "ScreenCapture.h"
#include "internal/SCCommon.h"
#include <memory>
#include "GDIHelpers.h"

namespace SL {
    namespace Screen_Capture {

        class GDIMouseProcessor : public BaseMouseProcessor { 
            HDCWrapper MonitorDC;
            HDCWrapper CaptureDC; 
   
        public:

            const int MaxCursurorSize = 32;
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data);
            DUPL_RETURN ProcessFrame();

      
        };
    }
}