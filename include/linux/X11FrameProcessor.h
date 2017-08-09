#pragma once
#include "SCCommon.h"
#include <memory>
#include <X11/Xlib.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

namespace SL {
    namespace Screen_Capture {
   
        class X11FrameProcessor: public BaseFrameProcessor {
            
			Display* SelectedDisplay=nullptr;
            XID SelectedWindow = 0;
			XImage* Image=nullptr;
			std::unique_ptr<XShmSegmentInfo> ShmInfo;
            Monitor SelectedMonitor;
            
        public:
            X11FrameProcessor();
            ~X11FrameProcessor();
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, Monitor& monitor);
            DUPL_RETURN ProcessFrame(const Monitor& currentmonitorinfo);
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, const Window& selectedwindow);
            DUPL_RETURN ProcessFrame(Window& selectedwindow);
        };
    }
}