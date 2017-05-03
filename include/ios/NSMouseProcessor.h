#pragma once
#include "SCCommon.h"
#include <memory>

namespace SL {
    namespace Screen_Capture {
        struct NSMouseProcessorImpl;
        class NSMouseProcessor {
            std::unique_ptr<NSMouseProcessorImpl> _NSMouseProcessorImpl;
        public:
            NSMouseProcessor();
            ~NSMouseProcessor();
            DUPL_RETURN Init(std::shared_ptr<Mouse_Thread_Data> data);
            DUPL_RETURN ProcessFrame();

        };

    }
}