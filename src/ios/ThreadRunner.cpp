#include "ScreenCapture.h"
#include "ThreadManager.h"
#include "CGFrameProcessor.h"
#include "NSMouseProcessor.h"

namespace SL{
    namespace Screen_Capture{
        void RunCaptureMouse(std::shared_ptr<Thread_Data> data) {
            TryCaptureMouse<NSMouseProcessor>(data);
        }
        void RunCaptureMonitor(std::shared_ptr<Thread_Data> data, Monitor monitor){
            TryCaptureMonitor<CGFrameProcessor>(data, monitor);
        }
    }
}

  
    
