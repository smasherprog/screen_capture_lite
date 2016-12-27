#include "ScreenCaptureManager.h"
#include <thread>

#ifdef WIN32
#include "ScreenCaptureDX.h"
#endif // DEBUG

namespace SL {
	namespace Screen_Capture {

		class ScreenCaptureManagerImpl {
#ifdef WIN32
			SL::Screen_Capture::ScreenCaptureDX FrameGrabber;
#endif
		public:

			int sleeptime = 100;//in ms
			ImageCallback callback;

			ScreenCaptureManagerImpl() {
			
			}
			~ScreenCaptureManagerImpl() {
				stop();
			}
			void start() {
				FrameGrabber.StartProcessing(callback);
			}
			void stop() {
				FrameGrabber.StopProcessing();
			}
		};

		ScreenCaptureManager::ScreenCaptureManager()
		{
			_ScreenCaptureManagerImpl = std::make_unique<ScreenCaptureManagerImpl>();
		}

		ScreenCaptureManager::~ScreenCaptureManager()
		{
			_ScreenCaptureManagerImpl->stop();
		}
		void ScreenCaptureManager::StartCapturing(ImageCallback img_cb, int min_interval)
		{
			_ScreenCaptureManagerImpl->callback = img_cb;
			_ScreenCaptureManagerImpl->sleeptime = min_interval;
			_ScreenCaptureManagerImpl->start();
		}
		void ScreenCaptureManager::StopCapturing()
		{
			_ScreenCaptureManagerImpl->stop();
			_ScreenCaptureManagerImpl = std::make_unique<ScreenCaptureManagerImpl>();
		}
	}
}


