#pragma once
#include "SCCommon.h"
#include <memory>

namespace SL {
    namespace Screen_Capture {
        struct X11FrameProcessorImpl;
        class X11FrameProcessor {
            std::unique_ptr<X11FrameProcessorImpl> _X11FrameProcessorImpl;
        public:
            X11FrameProcessor();
            ~X11FrameProcessor();
            DUPL_RETURN Init(std::shared_ptr<Monitor_Thread_Data> data);
            DUPL_RETURN ProcessFrame();

        };

    }
}