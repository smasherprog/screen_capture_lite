#include "ScreenCapture.h"
#include "ThreadRunner.h"
#include "DXCommon.h"
#include "DXThread.h"
#include "GDIThread.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {

		void ProcessExit(DUPL_RETURN Ret, THREAD_DATA* TData) {
			if (Ret != DUPL_RETURN_SUCCESS)
			{
				if (Ret == DUPL_RETURN_ERROR_EXPECTED)
				{
					// The system is in a transition state so request the duplication be restarted
					*TData->ExpectedErrorEvent = true;
				}
				else
				{
					// Unexpected error so exit the application
					*TData->UnexpectedErrorEvent = true;
				}
			}
		}

		void SL::Screen_Capture::RunCapture(std::shared_ptr<THREAD_DATA> data) {
			//need to switch to the input desktop for capturing...
			HDESK CurrentDesktop = nullptr;
			CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
			if (!CurrentDesktop)
			{
				// We do not have access to the desktop so request a retry
				*data->ExpectedErrorEvent = true;
				return ProcessExit(DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED, data.get());
			}

			// Attach desktop to this thread
			bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
			CloseDesktop(CurrentDesktop);
			CurrentDesktop = nullptr;
			if (!DesktopAttached)
			{
				// We do not have access to the desktop so request a retry
				*data->ExpectedErrorEvent = true;
				return ProcessExit(DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED, data.get());
			}
			//return GDIThread(data);
			if (DesktopDuplicationSupported() == DUPL_RETURN::DUPL_RETURN_SUCCESS) {
				return DXThread(data);
			}
			else {
				return GDIThread(data);
			}
		}

	}

}



