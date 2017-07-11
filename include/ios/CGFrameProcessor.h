#pragma once
#include "SCCommon.h"
#include <memory>

namespace SL {
    namespace Screen_Capture {
        class CGFrameProcessor: public BaseFrameProcessor {
            Monitor SelectedMonitor;
        public:
            
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, Monitor& monitor);
            DUPL_RETURN ProcessFrame(const Monitor& curentmonitorinfo);

        };

    }
}
