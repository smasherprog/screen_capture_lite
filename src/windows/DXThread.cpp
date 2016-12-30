#include "DXThread.h"
#include "DXCommon.h"
#include "DXFrameProcessor.h"
#include "DXDuplicationManager.h"
#include <string>


namespace SL {
	namespace Screen_Capture {

		void DXThread(std::shared_ptr<THREAD_DATA> data) {


			DX_RESOURCES res;

			auto Ret = Initialize(res);
			if (Ret != DUPL_RETURN_SUCCESS) return;// get out

			DXFrameProcessor DispMgr(res, data->CallBack);
			DXDuplicationManager DuplMgr;

			// Make duplication manager
			Ret = DuplMgr.InitDupl(res.Device.Get(), data->SelectedMonitor.Index);
			if (Ret != DUPL_RETURN_SUCCESS) return;// get out

			// Get output description
			DXGI_OUTPUT_DESC DesktopDesc;
			RtlZeroMemory(&DesktopDesc, sizeof(DXGI_OUTPUT_DESC));
			DuplMgr.GetOutputDesc(&DesktopDesc);


			FRAME_DATA Currendata;
			Currendata.SrcreenIndex = data->SelectedMonitor.Index;
			while (!*data->TerminateThreadsEvent)
			{
				auto start = std::chrono::high_resolution_clock::now();
				bool TimeOut;

				// Get new frame from desktop duplication
				Ret = DuplMgr.GetFrame(&Currendata, &TimeOut);
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

				// Process new frame
				DispMgr.ProcessFrame(&Currendata, &DesktopDesc);
				// Release frame back to desktop duplication
				Ret = DuplMgr.DoneWithFrame();
				if (Ret != DUPL_RETURN_SUCCESS)
				{
					break;
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