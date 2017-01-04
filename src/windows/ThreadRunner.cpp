#include "ScreenCapture.h"
#include "DXFrameProcessor.h"
#include "GDIFrameProcessor.h"
#include "ThreadManager.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>
#include <string>
#include <iostream>

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
			//TryCapture<GDIFrameProcessor>(data);
			std::cout << "Starting to Capture on Monitor " << data->SelectedMonitor.Name << std::endl;
			std::cout << "Trying DirectX Desktop Duplication " << std::endl;
			if (!TryCapture<DXFrameProcessor>(data)) {//if DX is not supported, fallback to GDI capture
				std::cout << "DirectX Desktop Duplication not supprted, falling back to GDI Capturing . . ." << std::endl;
				TryCapture<GDIFrameProcessor>(data);
			}
		}
	}
}



