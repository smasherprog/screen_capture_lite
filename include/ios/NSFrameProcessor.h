#pragma once
#include "SCCommon.h"

namespace SL {
    namespace Screen_Capture {
        class NSFrameProcessor;
        struct NSFrameProcessorImpl;
        NSFrameProcessorImpl* CreateNSFrameProcessorImpl();
        void DestroyNSFrameProcessorImpl(NSFrameProcessorImpl*);
        DUPL_RETURN Init(NSFrameProcessorImpl* createdimpl, NSFrameProcessor* parent);
        
        class NSFrameProcessor : public BaseFrameProcessor {
            NSFrameProcessorImpl* NSFrameProcessorImpl_ = nullptr;
        public:
            NSFrameProcessor();
            ~NSFrameProcessor();
            Monitor SelectedMonitor;
            
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, Monitor& monitor);
            DUPL_RETURN ProcessFrame(const Monitor& curentmonitorinfo);
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, Window& window);
            DUPL_RETURN ProcessFrame(const Window& window);
            
        };
    }
}
