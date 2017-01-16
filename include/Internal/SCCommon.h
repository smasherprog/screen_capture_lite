#pragma once
#include "ScreenCapture.h"
#include <thread>
#include <atomic>

// this is internal stuff.. 
namespace SL {
	namespace Screen_Capture {

		struct ScreenCapture_Settings {
			//min interval between frames that are captured
			int Monitor_Capture_Interval;
			//the monitors that are captured each interval
			std::vector<Monitor> Monitors;
			//set this if you want to capture the entire monitor each interval
			CaptureCallback CaptureEntireMonitor;
			//set this if you want to receive difs each interval on what has changed
			CaptureCallback CaptureDifMonitor;
			//min interval between mouse captures
			int Mouse_Capture_Interval;
			//the function to be called on each mouse interval. If a the mouse image has changed, img will not be null, otherwise, the only change is new mouse coords
			MouseCallback CaptureMouse;

		};

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
	
			Monitor SelectedMonitor;

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
		Monitor CreateMonitor(int index,int id, int h, int w, int ox, int oy, const std::string& n);
	
		std::vector<ImageRect> GetDifs(const Image & oldimg, const Image & newimg);

	}
}