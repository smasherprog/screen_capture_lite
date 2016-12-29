#pragma once
#include "ScreenCapture.h"
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
			DUPL_RETURN Init(std::shared_ptr<std::atomic_bool>& unexpected, std::shared_ptr<std::atomic_bool>& expected, std::shared_ptr<std::atomic_bool>& terminate, ImageCallback& CallBack);
			void Join();
			void Reset();
		};
	}
}