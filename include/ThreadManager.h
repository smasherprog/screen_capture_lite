#pragma once
#include "ScreenCapture.h"
#include "ThreadRunner.h"
#include <thread>
#include <vector>
#include <atomic>

namespace SL {
	namespace Screen_Capture {



		class ThreadManager {

			std::vector<std::thread> m_ThreadHandles;
			std::vector<std::shared_ptr<THREAD_DATA>> m_ThreadData;

		public:
			ThreadManager();
			~ThreadManager();
			void Init(std::shared_ptr<std::atomic_bool>& unexpected, 
				std::shared_ptr<std::atomic_bool>& expected, 
				std::shared_ptr<std::atomic_bool>& terminate, 
				CaptureEntireMonitorCallback& captureentiremonitor,
				CaptureDifMonitorCallback& capturedifmonitor,
				int mininterval);
			void Join();
			void Reset();
		};
	}
}