#include "ScreenCapture.h"
#include "ThreadRunner.h"
#include "DXCommon.h"
#include "DXFrameProcessor.h"
#include "GDIFrameProcessor.h"

#include <memory>
#include <string>
#include <iostream>

namespace SL {
	namespace Screen_Capture {


		template<class T>DUPL_RETURN RunThread(std::shared_ptr<THREAD_DATA> data, T& frameprocessor) {
			while (!*data->TerminateThreadsEvent)
			{
				auto start = std::chrono::high_resolution_clock::now();
				//Process Frame
				auto Ret = frameprocessor.ProcessFrame();
				if (Ret != DUPL_RETURN_SUCCESS)
				{
					return Ret;
				}
				auto mspassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
				std::string msg = "took ";
				msg += std::to_string(mspassed) + "ms for output ";
				msg += std::to_string(data->SelectedMonitor.Index) + "\n";
				OutputDebugStringA(msg.c_str());
				auto timetowait = data->CaptureInterval - mspassed;
				if (timetowait > 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(timetowait));
				}
			}
			return DUPL_RETURN_SUCCESS;
		}

		template<class T>bool TryCapture(std::shared_ptr<THREAD_DATA> data) {
			T frameprocessor;

			// Make duplication manager
			auto ret = frameprocessor.Init(data->CallBack, data->SelectedMonitor);
			if (ret != DUPL_RETURN_SUCCESS) {//Directx duplication is NOT supported!
				return false;
			}
			else {
				ret = RunThread(data, frameprocessor);
				if (ret != DUPL_RETURN_SUCCESS)
				{
					if (ret == DUPL_RETURN_ERROR_EXPECTED)
					{
						// The system is in a transition state so request the duplication be restarted
						*data->ExpectedErrorEvent = true;
					}
					else
					{
						// Unexpected error so exit the application
						*data->UnexpectedErrorEvent = true;
					}
				}
				OutputDebugStringA("Exiting Thread\n");
				return true;
			}
		}
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
		
			std::cout << "Starting to Capture on Monitor " << data->SelectedMonitor.Name << std::endl;
			std::cout << "Trying DirectX Desktop Duplication " << std::endl;
			if (!TryCapture<DXFrameProcessor>(data)) {//if DX is not supported, fallback to GDI capture
				std::cout << "DirectX Desktop Duplication not supprted, falling back to GDI Capturing . . ." << std::endl;
				TryCapture<GDIFrameProcessor>(data);
			}
		}
	}
}



