#include "ScreenCapture.h"
#include "ThreadManager.h"
#include <thread>
#include <atomic>
#include <algorithm>
#include <memory>
#include <assert.h>

namespace SL {
	namespace Screen_Capture {

		class ScreenCaptureManagerImpl {
		public:

			int SleepTime = 100;//in ms
			CaptureEntireMonitorCallback CaptureEntireMonitor;
			CaptureDifMonitorCallback CaptureDifMonitor;

			std::thread _Thread;
			std::shared_ptr<std::atomic_bool> _TerminateThread;
			std::vector<Monitor> Monitors;

			ScreenCaptureManagerImpl() {

			}
			~ScreenCaptureManagerImpl() {
				stop();
			}
			void start() {
				//users must set the monitors to capture before calling start
				assert(!Monitors.empty());
				//users must set at least one callback before starting
				assert(CaptureEntireMonitor || CaptureDifMonitor);

				stop();
				_Thread = std::thread([&]() {
					ThreadManager ThreadMgr;
					auto expected = std::make_shared<std::atomic_bool>(false);
					auto unexpected = std::make_shared<std::atomic_bool>(false);
					_TerminateThread = std::make_shared<std::atomic_bool>(false);

					ThreadMgr.Init(unexpected, expected, _TerminateThread, CaptureEntireMonitor, CaptureDifMonitor, SleepTime, Monitors);

					while (!*_TerminateThread) {

						if (*expected)
						{

							// Terminate other threads
							*_TerminateThread = true;
							ThreadMgr.Join();
							*expected = *unexpected = *_TerminateThread = false;
							// Clean up
							ThreadMgr.Reset();
							std::this_thread::sleep_for(std::chrono::milliseconds(1000));//sleep for 1 second since an error occcured

							ThreadMgr.Init(unexpected, expected, _TerminateThread, CaptureEntireMonitor, CaptureDifMonitor, SleepTime, Monitors);
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
		void ScreenCaptureManager::Set_CaptureMonitors(const std::vector<Monitor>& monitorstocapture)
		{
			_ScreenCaptureManagerImpl->Monitors = monitorstocapture;
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


