#include "DXThread.h"
#include "DXCommon.h"
#include "DXFrameProcessor.h"
#include <string>


namespace SL {
	namespace Screen_Capture {

		void DXThread(std::shared_ptr<THREAD_DATA> data) {


			DXFrameProcessor DispMgr;

			// Make duplication manager
			auto Ret = DispMgr.Init(data->CallBack, data->SelectedMonitor.Index);
			if (Ret != DUPL_RETURN_SUCCESS) return;// get out

			while (!*data->TerminateThreadsEvent)
			{
				auto start = std::chrono::high_resolution_clock::now();
				bool TimeOut;

				//Process Frame
				Ret = DispMgr.ProcessFrame(&TimeOut);
				if (Ret != DUPL_RETURN_SUCCESS)
				{
					// An error occurred getting the next frame drop out of loop which
					// will check if it was expected or not
					break;
				}

				// Check for timeout
				if (TimeOut)
				{
					// No new frame at the moment
					continue;
				}

				auto mspassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
				std::string msg = "DX took ";
				msg += std::to_string(mspassed) + "ms for output ";
				msg += std::to_string(data->SelectedMonitor.Index) + "\n";
				OutputDebugStringA(msg.c_str());
				auto timetowait = data->CaptureInterval - mspassed;
				if (timetowait > 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(timetowait));
				}
			}
			if (Ret != DUPL_RETURN_SUCCESS)
			{
				if (Ret == DUPL_RETURN_ERROR_EXPECTED)
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

		}
	}
}