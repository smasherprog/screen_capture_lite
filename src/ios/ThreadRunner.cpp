#include "ScreenCapture.h"
#include "ThreadManager.h"
#include "CGFrameProcessor.h"


namespace SL{
    namespace Screen_Capture{
        void RunCapture(std::shared_ptr<THREAD_DATA> data){
            
            
            TryCapture<CGFrameProcessor>(data);
            
        }
    }
}

  
    
