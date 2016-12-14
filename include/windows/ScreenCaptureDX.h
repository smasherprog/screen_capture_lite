#pragma once

#include "ScreenTypes.h"
#include "InterruptableSleeper.h"

namespace SL {
	namespace Screen_Capture {
        
		class ScreenCaptureDX {
	
			std::thread _Thread;
			InterruptableSleeper _InterruptableSleeper;
			bool _KeepRunning;

		public:
			ScreenCaptureDX();
			~ScreenCaptureDX();
			void StartProcessing(ImageCallback& img_cb);
			void StopProcessing();
		};
    }
}