#include "ScreenCaptureManager.h"
#include <assert.h>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>


namespace SL {
	namespace Screen_Capture {

		class ScreenCaptureManagerImpl {
			void run() {
				while (keeprunning) {


				}
			}
		public:

			std::thread thread;

			int sleeptime = 100;//in ms
			ImageCallback callback;
			bool keeprunning = true;

			ScreenCaptureManagerImpl() {


			}
			~ScreenCaptureManagerImpl() {

			}
			void start() {
				thread = std::thread(&SL::Screen_Capture::ScreenCaptureManagerImpl::run, this);
			}
			void stop() {
				if (thread.joinable()) {
		
					thread.join();
				}
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


