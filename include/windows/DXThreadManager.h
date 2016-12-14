#pragma once
#include "CommonTypes.h"
#include "ScreenTypes.h"
#include <thread>
#include <vector>

namespace SL {
	namespace Screen_Capture {

		class DXThreadManager {
			std::shared_ptr<PTR_INFO> m_PtrInfo;

			int m_ThreadCount;
			std::vector<HANDLE> m_ThreadHandles;
			std::vector<std::shared_ptr<THREAD_DATA>> m_ThreadData;

		public:
			DXThreadManager();
			~DXThreadManager();

			void Clean();
			DUPL_RETURN Initialize(std::shared_ptr<std::atomic_bool> UnexpectedErrorEvent, std::shared_ptr<std::atomic_bool> ExpectedErrorEvent, std::shared_ptr<std::atomic_bool> TerminateThreadsEvent, ImageCallback& cb);
			PTR_INFO* GetPointerInfo();
			void WaitForThreadTermination();

		private:
			DUPL_RETURN InitializeDx( DX_RESOURCES* Data);
		};
	}
}