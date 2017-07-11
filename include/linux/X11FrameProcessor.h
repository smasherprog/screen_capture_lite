#pragma once
#include "SCCommon.h"
#include <memory>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <sys/shm.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/XShm.h>

namespace SL {
    namespace Screen_Capture {
   
        class X11FrameProcessor: public BaseFrameProcessor {
            
			Display* SelectedDisplay=nullptr;
			XImage* Image=nullptr;
			std::unique_ptr<XShmSegmentInfo> ShmInfo;
            Monitor SelectedMonitor;
            
        public:
            X11FrameProcessor();
            ~X11FrameProcessor();
            DUPL_RETURN Init(std::shared_ptr<Thread_Data> data, Monitor& monitor);
            DUPL_RETURN ProcessFrame(const Monitor& currentmonitorinfo);

        };
    }
}