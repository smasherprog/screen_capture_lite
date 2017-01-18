#include "ScreenCapture.h"
#include "SCCommon.h"
#include "ThreadManager.h"
#include <thread>
#include <atomic>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <cstring>
#include <iostream>

namespace SL {
	namespace Screen_Capture {


		class ScreenCaptureManagerImpl {
		public:

			ScreenCapture_Settings Settings;

			std::thread _Thread;
			std::shared_ptr<std::atomic_bool> _TerminateThread;


			ScreenCaptureManagerImpl() {
				_TerminateThread = std::make_shared<std::atomic_bool>(false);
			}
			~ScreenCaptureManagerImpl() {
				stop(true);
			}
			void start() {
			
				//users must set at least one callback before starting
				assert(Settings.CaptureEntireMonitor || Settings.CaptureDifMonitor || Settings.CaptureMouse);

				stop(true);
				_Thread = std::thread([&]() {
					ThreadManager ThreadMgr;
					Base_Thread_Data data;
					data.ExpectedErrorEvent = std::make_shared<std::atomic_bool>(false);
					data.UnexpectedErrorEvent = std::make_shared<std::atomic_bool>(false);
					data.TerminateThreadsEvent = _TerminateThread;

					ThreadMgr.Init(data, Settings);

					while (!*_TerminateThread) {

						if (*data.ExpectedErrorEvent)
						{
							// std::cout<<"Expected Error, Restarting Thread Manager"<<std::endl;
							 // Terminate other threads
							*_TerminateThread = true;
							ThreadMgr.Join();
							*data.ExpectedErrorEvent = *data.UnexpectedErrorEvent = *_TerminateThread = false;
							// Clean up
							ThreadMgr.Reset();
							std::this_thread::sleep_for(std::chrono::milliseconds(1000));//sleep for 1 second since an error occcured

							ThreadMgr.Init(data, Settings);
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
					}
					*_TerminateThread = true;
					ThreadMgr.Join();
				});
			}
			void stop(bool block) {
				*_TerminateThread = false;
				if (block) {
					if (_Thread.joinable()) {
						_Thread.join();
					}
				}

			}
		};

		ScreenCaptureManager::ScreenCaptureManager()
		{
			_ScreenCaptureManagerImpl = std::make_unique<ScreenCaptureManagerImpl>();
		}

		ScreenCaptureManager::~ScreenCaptureManager()
		{
			_ScreenCaptureManagerImpl->stop(true);
		}

		void ScreenCaptureManager::Start()
		{
			_ScreenCaptureManagerImpl->start();
		}
		void ScreenCaptureManager::Stop()
		{
			_ScreenCaptureManagerImpl->stop(false);
		}
		void ScreenCaptureManager::setMonitorsToCapture(MonitorCallback& cb) {
			_ScreenCaptureManagerImpl->Settings.MonitorsChanged = cb;
		}	
		void ScreenCaptureManager::setMonitorsToCapture(const MonitorCallback& cb) {
			_ScreenCaptureManagerImpl->Settings.MonitorsChanged = cb;
		}
		void ScreenCaptureManager::setFrameChangeInterval(int interval) {
			_ScreenCaptureManagerImpl->Settings.Monitor_Capture_Interval = interval;
		}
		void ScreenCaptureManager::onNewFrame(CaptureCallback& cb) {
			_ScreenCaptureManagerImpl->Settings.CaptureEntireMonitor = cb;
		}
		void ScreenCaptureManager::onNewFrame(const CaptureCallback& cb) {
			_ScreenCaptureManagerImpl->Settings.CaptureEntireMonitor = cb;
		}
		void ScreenCaptureManager::onFrameChanged(CaptureCallback& cb) {
			_ScreenCaptureManagerImpl->Settings.CaptureDifMonitor = cb;
		}
		void ScreenCaptureManager::onFrameChanged(const CaptureCallback& cb) {
			_ScreenCaptureManagerImpl->Settings.CaptureDifMonitor = cb;
		}
		void ScreenCaptureManager::onMouseChanged(MouseCallback& cb) {
			_ScreenCaptureManagerImpl->Settings.CaptureMouse = cb;
		}
		void ScreenCaptureManager::onMouseChanged(const MouseCallback& cb) {
			_ScreenCaptureManagerImpl->Settings.CaptureMouse = cb;
		}
		void ScreenCaptureManager::setMouseChangeInterval(int interval) {
			_ScreenCaptureManagerImpl->Settings.Mouse_Capture_Interval = interval;
		}

	}
}


