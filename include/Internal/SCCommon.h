#pragma once
#include "ScreenCapture.h"
#include <thread>
#include <atomic>

// this is internal stuff.. 
namespace SL {
	namespace Screen_Capture {
	
		struct THREAD_DATA
		{
			// Used to indicate abnormal error condition
			std::shared_ptr<std::atomic_bool> UnexpectedErrorEvent;
			// Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate the duplication interface
			std::shared_ptr<std::atomic_bool> ExpectedErrorEvent;
			// Used to signal to threads to exit
			std::shared_ptr<std::atomic_bool> TerminateThreadsEvent;
			std::shared_ptr<Monitor> SelectedMonitor;
			int CaptureInterval; //in milliseconds	
			CaptureCallback CaptureEntireMonitor;
			CaptureCallback CaptureDifMonitor;
		};

		enum DUPL_RETURN
		{
			DUPL_RETURN_SUCCESS = 0,
			DUPL_RETURN_ERROR_EXPECTED = 1,
			DUPL_RETURN_ERROR_UNEXPECTED = 2
		};
		const int PixelStride = 4;
		std::shared_ptr<Monitor> CreateMonitor(int id, int h, int w, int ox, int oy, const std::string& n);
		std::shared_ptr<Image> CreateImage(const ImageRect& b, int ps, int rp, char* d);
		std::vector<ImageRect> GetDifs(const Image & oldimg, const Image & newimg);

	}
}