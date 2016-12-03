#pragma once
#include "CommonTypes.h"
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
			DUPL_RETURN Initialize(INT SingleOutput, UINT OutputCount, HANDLE UnexpectedErrorEvent, HANDLE ExpectedErrorEvent, HANDLE TerminateThreadsEvent, HANDLE SharedHandle, _In_ RECT* DesktopDim);
			PTR_INFO* GetPointerInfo();
			void WaitForThreadTermination();

		private:
			DUPL_RETURN InitializeDx( DX_RESOURCES* Data);
		};
	}
}