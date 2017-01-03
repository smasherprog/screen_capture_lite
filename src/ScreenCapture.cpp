#include "ScreenCapture.h"
#include "ThreadManager.h"
#include <thread>
#include <atomic>
#include <algorithm>

namespace SL {
	namespace Screen_Capture {

		class ScreenCaptureManagerImpl {
		public:

			int SleepTime = 100;//in ms
			CaptureEntireMonitorCallback CaptureEntireMonitor;
			CaptureDifMonitorCallback CaptureDifMonitor;

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
					auto sleeptime = 50;

					while (!*_TerminateThread) {
						//if an expected error occurs, make sure to sleep a little longer each loop that the error orrcurs with a max of 1500 ms
						if (*expected) {
							sleeptime *= 2;
							sleeptime = std::min(1500, sleeptime);
						}
						else {//if no expected error has occured lower the sleep time a little bit each loop until we reach 50 ms wait time. 
							sleeptime -= 50;
							sleeptime = std::max(50, sleeptime);
						}

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
							ThreadMgr.Init(unexpected, expected, _TerminateThread,  CaptureEntireMonitor, CaptureDifMonitor,SleepTime);
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
		void ScreenCaptureManager::Set_CapturCallback(CaptureEntireMonitorCallback img_cb) {
			_ScreenCaptureManagerImpl->CaptureEntireMonitor = img_cb;
		}
		void ScreenCaptureManager::Set_CapturCallback(CaptureDifMonitorCallback img_cb) {
			_ScreenCaptureManagerImpl->CaptureDifMonitor = img_cb;
		}

		void ScreenCaptureManager::StartCapturing(int min_interval)
		{
			_ScreenCaptureManagerImpl->SleepTime = min_interval;
			_ScreenCaptureManagerImpl->start();
		}
		void ScreenCaptureManager::StopCapturing()
		{
			_ScreenCaptureManagerImpl->stop();
			_ScreenCaptureManagerImpl = std::make_unique<ScreenCaptureManagerImpl>();
		}
	}
}


