#include "ScreenCapture.h"
#include "ThreadManager.h"
#include "X11FrameProcessor.h"
#include "X11MouseProcessor.h"

namespace SL{
    namespace Screen_Capture{	
        void RunCaptureMouse(std::shared_ptr<Mouse_Thread_Data> data) {
            TryCapture<X11MouseProcessor>(data);
        }
        void RunCapture(std::shared_ptr<Monitor_Thread_Data> data){
            TryCapture<X11FrameProcessor>(data);
        }
    }
}

  
    
