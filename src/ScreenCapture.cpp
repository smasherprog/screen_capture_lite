#include "ScreenCapture.h"
#include "ThreadManager.h"
#include <thread>

namespace SL {
	namespace Screen_Capture {

		class ScreenCaptureManagerImpl {
		public:

			int sleeptime = 100;//in ms
			ImageCallback callback;

			std::thread _Thread;
			std::shared_ptr<std::atomic_bool> _TerminateThread;

			ScreenCaptureManagerImpl() {

			}
			~ScreenCaptureManagerImpl() {
				stop();
			}
			void start() {
				stop();
				_Thread = std::thread([&]() {
					ThreadManager ThreadMgr;
					auto expected = std::make_shared<std::atomic_bool>(false);
					auto unexpected = std::make_shared<std::atomic_bool>(false);
					_TerminateThread = std::make_shared<std::atomic_bool>(false);

					bool FirstTime = true;

					while (*_TerminateThread) {
						DUPL_RETURN Ret = DUPL_RETURN_SUCCESS;
						if (FirstTime || *expected)
						{
							if (!FirstTime)
							{
								// Terminate other threads
								*_TerminateThread = true;
								ThreadMgr.Join();
								*expected = *unexpected = *_TerminateThread = false;
								// Clean up
								ThreadMgr.Reset();
							}
							else
							{
								// First time through the loop so nothing to clean up
								FirstTime = false;
							}
							Ret = ThreadMgr.Init(unexpected, expected, _TerminateThread, callback);
						}

						// Check if for errors
						if (Ret != DUPL_RETURN_SUCCESS)
						{
							if (Ret == DUPL_RETURN_ERROR_EXPECTED)
							{
								// Some type of system transition is occurring so retry
								*expected = true;
							}
							else
							{
								// Unexpected error so exit
								break;
							}
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(sleeptime));
					}
					*_TerminateThread = true;
					ThreadMgr.Join();
				});
			}
			void stop() {
				if (_TerminateThread) {
					*_TerminateThread = false;
				}
				if (_Thread.joinable()) {
					_Thread.join();
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


