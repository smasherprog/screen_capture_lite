#pragma once
#include "ScreenCapture.h"
#include <memory>
#include <atomic>

namespace SL {
	namespace Screen_Capture {

struct THREAD_DATA
		{
			// Used to indicate abnormal error condition
			std::shared_ptr<std::atomic_bool> UnexpectedErrorEvent;
			// Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate the duplication interface
			std::shared_ptr<std::atomic_bool> ExpectedErrorEvent;
			// Used by WinProc to signal to threads to exit
			std::shared_ptr<std::atomic_bool> TerminateThreadsEvent;
			Monitor SelectedMonitor;
			int CaptureInterval; //in milliseconds
			ImageCallback CallBack;
		};
		void RunCapture(std::shared_ptr<THREAD_DATA> data);

    }
}