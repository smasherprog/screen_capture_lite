#pragma once

#include "ScreenTypes.h"


namespace SL {
	namespace Screen_Capture {
        
		class ScreenCaptureDX {
	
	

		public:
			ScreenCaptureDX();
			~ScreenCaptureDX();
			void StartProcessing(ImageCallback img_cb);
		};
    }
}