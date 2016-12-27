#pragma once

#include "ScreenTypes.h"
#include <thread>

namespace SL {
	namespace Screen_Capture {
        
		class ScreenCaptureDX {
	
			std::thread _Thread;
			bool _KeepRunning;

		public:
			ScreenCaptureDX();
			~ScreenCaptureDX();
			void StartProcessing(ImageCallback& img_cb);
			void StopProcessing();
		};
    }
}