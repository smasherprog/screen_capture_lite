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

		}
		ScreenCaptureDX::~ScreenCaptureDX()
		{
			StopProcessing();
		}
		void ScreenCaptureDX::StopProcessing()
		{
			_KeepRunning = false;
			_InterruptableSleeper.wake();
			if (_Thread.joinable()) {
				_Thread.join();
			}
		}

		void ScreenCaptureDX::StartProcessing(ImageCallback& img_cb)
		{
			StopProcessing();
			_KeepRunning = true;

			_Thread = std::thread([&]() {

				DXThreadManager ThreadMgr;
				auto expectederror = std::make_shared<std::atomic_bool>(false);
				auto unexpectederror = std::make_shared<std::atomic_bool>(false);
				auto terminatethreads = std::make_shared<std::atomic_bool>(false);

				bool FirstTime = true;

				while (_KeepRunning) {
					DUPL_RETURN Ret = DUPL_RETURN_SUCCESS;
					if (FirstTime || *expectederror)
					{
						if (!FirstTime)
						{
							// Terminate other threads
							*terminatethreads = true;
							ThreadMgr.WaitForThreadTermination();
							*unexpectederror = *expectederror = *terminatethreads = false;

							// Clean up
							ThreadMgr.Clean();

						}
						else
						{
							// First time through the loop so nothing to clean up
							FirstTime = false;
						}
						Ret = ThreadMgr.Initialize(unexpectederror, expectederror, terminatethreads, img_cb);
					}
					else
					{

					}

					// Check if for errors
					if (Ret != DUPL_RETURN_SUCCESS)
					{
						if (Ret == DUPL_RETURN_ERROR_EXPECTED)
						{
							// Some type of system transition is occurring so retry
							*expectederror = true;
						}
						else
						{
							// Unexpected error so exit
							break;
						}
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					//_InterruptableSleeper.sleepFor(std::chrono::milliseconds(50));
				}
				*terminatethreads = true;
				ThreadMgr.WaitForThreadTermination();
			});
		}


	}
}


