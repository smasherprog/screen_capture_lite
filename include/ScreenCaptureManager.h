#pragma once
#include <memory>
#include <functional>
#include "ScreenTypes.h"

namespace SL {
	namespace Screen_Capture {

		class ScreenCaptureManagerImpl;
		class ScreenCaptureManager {
			std::unique_ptr<ScreenCaptureManagerImpl> _ScreenCaptureManagerImpl;

		public:
			ScreenCaptureManager();
			~ScreenCaptureManager();
			void StartCapturing(ImageCallback img_cb, int min_interval);
			void StopCapturing();
		};
	}
};
