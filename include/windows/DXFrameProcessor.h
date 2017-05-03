#pragma once
#include "SCCommon.h"
#include <memory>

namespace SL {
    namespace Screen_Capture {
        struct DXFrameProcessorImpl;
        class DXFrameProcessor {
            std::unique_ptr<DXFrameProcessorImpl> DXFrameProcessorImpl_;
        public:
            DXFrameProcessor();
            ~DXFrameProcessor();
            DUPL_RETURN Init(std::shared_ptr<Monitor_Thread_Data> data);
            DUPL_RETURN ProcessFrame();

        };

    }
}