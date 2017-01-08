#include "ScreenCapture.h"
#include "ThreadManager.h"
#include "X11FrameProcessor.h"


namespace SL{
    namespace Screen_Capture{
        void RunCapture(std::shared_ptr<THREAD_DATA> data){
            
            
            TryCapture<X11FrameProcessor>(data);
            
        }
    }
}

  
    
