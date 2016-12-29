#pragma once
#include "ScreenCapture.h"

namespace SL {
	namespace Screen_Capture {
        
		class ScreenCaptureWindowsImpl;
		class ScreenCaptureWindows {
			std::unique_ptr<ScreenCaptureWindowsImpl> _ScreenCaptureWindowsImpl;
		public:
			ScreenCaptureWindows(std::shared_ptr<THREAD_DATA>& data);
			~ScreenCaptureWindows();
			
		};
    }
}