#include "NSFrameProcessor.h"

namespace SL {
    namespace Screen_Capture {
   
        NSFrameProcessor::NSFrameProcessor(){
            NSFrameProcessorImpl_ = nullptr;
        }
        NSFrameProcessor::~NSFrameProcessor(){
            DestroyNSFrameProcessorImpl(NSFrameProcessorImpl_);
        }
        DUPL_RETURN NSFrameProcessor::Init(std::shared_ptr<Thread_Data> data, Monitor &monitor)
        {
            Data = data;
            SelectedMonitor = monitor;
            NSFrameProcessorImpl_ = CreateNSFrameProcessorImpl();
            return  Screen_Capture::Init(NSFrameProcessorImpl_, this);
        }
         
        DUPL_RETURN NSFrameProcessor::ProcessFrame(const Monitor &curentmonitorinfo)
        {
            return DUPL_RETURN_SUCCESS;
        }
    }
}
