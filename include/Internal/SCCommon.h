#pragma once
#include "ScreenCapture.h"
#include <thread>
#include <atomic>

// this is internal stuff.. 
namespace SL {
	namespace Screen_Capture {
		struct Base_Thread_Data
		{
			// Used to indicate abnormal error condition
			std::shared_ptr<std::atomic_bool> UnexpectedErrorEvent;
			// Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate the duplication interface
			std::shared_ptr<std::atomic_bool> ExpectedErrorEvent;
			// Used to signal to threads to exit
			std::shared_ptr<std::atomic_bool> TerminateThreadsEvent;
		};

		struct Monitor_Thread_Data: Base_Thread_Data
		{
	
			std::shared_ptr<Monitor> SelectedMonitor;

			int CaptureInterval; //in milliseconds	
			CaptureCallback CaptureEntireMonitor;
			CaptureCallback CaptureDifMonitor;
		};

		struct Mouse_Thread_Data : Base_Thread_Data
		{

			int CaptureInterval; //in milliseconds	
			MouseCallback CaptureCallback;
		};
		enum DUPL_RETURN
		{
			DUPL_RETURN_SUCCESS = 0,
			DUPL_RETURN_ERROR_EXPECTED = 1,
			DUPL_RETURN_ERROR_UNEXPECTED = 2
		};
		const int PixelStride = 4;
		std::shared_ptr<Monitor> CreateMonitor(int index,int id, int h, int w, int ox, int oy, const std::string& n);
	
		std::vector<ImageRect> GetDifs(const Image & oldimg, const Image & newimg);

	}
}