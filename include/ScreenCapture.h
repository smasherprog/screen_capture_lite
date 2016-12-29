#pragma once
#include <memory>
#include <functional>
#include <vector>

namespace SL {
	namespace Screen_Capture {
		struct CapturedImage {
			std::shared_ptr<char> Data;
			int Height = 0;
			int Width = 0;
			int RelativeTop = 0;
			int RelativeLeft = 0;
			int AbsoluteTop = 0;
			int AbsoluteLeft = 0;
			int ScreenIndex;
			const int PixelStride = 4;//in bytes
		};
		struct Monitor{
			int Index;
			int Height;
			int Width;
		};
		std::vector<Monitor> GetMonitors();
		typedef std::function<void(const CapturedImage& img)> ImageCallback;
		enum DUPL_RETURN
		{
			DUPL_RETURN_SUCCESS = 0,
			DUPL_RETURN_ERROR_EXPECTED = 1,
			DUPL_RETURN_ERROR_UNEXPECTED = 2
		};

		class ScreenCaptureManagerImpl;
		class ScreenCaptureManager {
			std::unique_ptr<ScreenCaptureManagerImpl> _ScreenCaptureManagerImpl;

		public:
			ScreenCaptureManager();
			~ScreenCaptureManager();
			void StartCapturing(ImageCallback img_cb, int min_interval);
			void StopCapturing();
		};
		struct THREAD_DATA
		{
			// Used to indicate abnormal error condition
			std::shared_ptr<std::atomic_bool> UnexpectedErrorEvent;
			// Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate the duplication interface
			std::shared_ptr<std::atomic_bool> ExpectedErrorEvent;
			// Used by WinProc to signal to threads to exit
			std::shared_ptr<std::atomic_bool> TerminateThreadsEvent;
			Monitor SelectedMonitor;
			ImageCallback CallBack;
		};
    }
}