#include "ScreenCaptureDX.h"
#include "CommonTypes.h"
#include <thread>
#include "DXDuplicationManager.h"
#include "DXFrameProcessor.h"
#include "DXThreadManager.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {

		ScreenCaptureDX::ScreenCaptureDX()
		{
			// Event used by the threads to signal an unexpected error and we want to quit the app
	

		}
		ScreenCaptureDX::~ScreenCaptureDX()
		{
		
		}

		void ScreenCaptureDX::StartProcessing(ImageCallback img_cb)
		{
			auto thrdmanager = std::make_unique<DXThreadManager>();

			// Event used by the threads to signal an unexpected error and we want to quit the app
			auto UnexpectedErrorEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
			// Event for when a thread encounters an expected error
			auto ExpectedErrorEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
			// Event to tell spawned threads to quit
			auto TerminateThreadsEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);



		}

	}
}


