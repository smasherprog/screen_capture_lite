#include "ScreenCapture.h"
#include "ThreadManager.h"
#include "CGFrameProcessor.h"
#include "NSMouseProcessor.h"

namespace SL{
    namespace Screen_Capture{
        void RunCaptureMouse(std::shared_ptr<Mouse_Thread_Data> data) {
            TryCapture<NSMouseProcessor>(data);
        }
        void RunCapture(std::shared_ptr<Monitor_Thread_Data> data){
            TryCapture<CGFrameProcessor>(data);
        }
    }
}

  
    
