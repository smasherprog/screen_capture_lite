#include "ScreenCapture.h"
#include "ThreadManager.h"
#include <thread>
#include <atomic>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <cstring>

namespace SL {
	namespace Screen_Capture {

		struct Monitor {
			int Id;
			int Index;
			int Height;
			int Width;
			//Offsets are the number of pixels that a monitor can be from the origin. For example, users can shuffle their monitors around so this affects their offset.
			int OffsetX;
			int OffsetY;
			std::string Name;
		};
		struct Image {
			ImageRect Bounds;
			int Pixelstride;
			int RowPadding;
			char* Data = nullptr;
		};


		class ScreenCaptureManagerImpl {
		public:

			int SleepTime = 100;//in ms
			CaptureCallback CaptureEntireMonitor;
			CaptureCallback CaptureDifMonitor;

			std::thread _Thread;
			std::shared_ptr<std::atomic_bool> _TerminateThread;
			std::vector<std::shared_ptr<Monitor>> Monitors;

			ScreenCaptureManagerImpl() {

			}
			~ScreenCaptureManagerImpl() {
				stop();
			}
			void start() {
				//users must set the monitors to capture before calling start
				assert(!Monitors.empty());
				//users must set at least one callback before starting
				assert(CaptureEntireMonitor || CaptureDifMonitor);

				stop();
				_Thread = std::thread([&]() {
					ThreadManager ThreadMgr;
					auto expected = std::make_shared<std::atomic_bool>(false);
					auto unexpected = std::make_shared<std::atomic_bool>(false);
					_TerminateThread = std::make_shared<std::atomic_bool>(false);

					ThreadMgr.Init(unexpected, expected, _TerminateThread, CaptureEntireMonitor, CaptureDifMonitor, SleepTime, Monitors);

					while (!*_TerminateThread) {

						if (*expected)
						{

							// Terminate other threads
							*_TerminateThread = true;
							ThreadMgr.Join();
							*expected = *unexpected = *_TerminateThread = false;
							// Clean up
							ThreadMgr.Reset();
							std::this_thread::sleep_for(std::chrono::milliseconds(1000));//sleep for 1 second since an error occcured

							ThreadMgr.Init(unexpected, expected, _TerminateThread, CaptureEntireMonitor, CaptureDifMonitor, SleepTime, Monitors);
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(50));
					}
					*_TerminateThread = true;
					ThreadMgr.Join();
				});
			}
			void stop() {
				if (_TerminateThread) {
					*_TerminateThread = false;
				}
				if (_Thread.joinable()) {
					_Thread.join();
				}
			}
		};

		ScreenCaptureManager::ScreenCaptureManager()
		{
			_ScreenCaptureManagerImpl = std::make_unique<ScreenCaptureManagerImpl>();
		}

		ScreenCaptureManager::~ScreenCaptureManager()
		{
			_ScreenCaptureManagerImpl->stop();
		}
		void ScreenCaptureManager::Set_CaptureMonitors(const std::vector<std::shared_ptr<Monitor>>& monitorstocapture)
		{
			_ScreenCaptureManagerImpl->Monitors = monitorstocapture;
		}

		void ScreenCaptureManager::Set_CaptureEntireCallback(CaptureCallback img_cb) {
			_ScreenCaptureManagerImpl->CaptureEntireMonitor = img_cb;
		}
		void ScreenCaptureManager::Set_CaptureDifCallback(CaptureCallback img_cb) {
			_ScreenCaptureManagerImpl->CaptureDifMonitor = img_cb;
		}

		void ScreenCaptureManager::StartCapturing(int min_interval)
		{
			_ScreenCaptureManagerImpl->SleepTime = min_interval;
			_ScreenCaptureManagerImpl->start();
		}
		void ScreenCaptureManager::StopCapturing()
		{
			_ScreenCaptureManagerImpl->stop();
			_ScreenCaptureManagerImpl = std::make_unique<ScreenCaptureManagerImpl>();
		}
		std::shared_ptr<Monitor> CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string & n)
		{
			auto ret = std::make_shared<Monitor>();
			ret->Index = index;
			ret->Height = h;
			ret->Id = id;
			ret->Name = n;
			ret->OffsetX = ox;
			ret->OffsetY = oy;
			ret->Width = w;
			return ret;
		}
		std::shared_ptr<Image> CreateImage(const ImageRect& b, int ps, int rp, char* d) {
			auto ret = std::make_shared<Image>();
			ret->Bounds = b;
			ret->Data = d;
			ret->Pixelstride = ps;
			ret->RowPadding = rp;
			return ret;
		}

		int Index(const Monitor& mointor) { return mointor.Index; }
		int Id(const Monitor& mointor) { return mointor.Id; }
		int OffsetX(const Monitor& mointor) { return mointor.OffsetX; }
		int OffsetY(const Monitor& mointor) { return mointor.OffsetY; }
		const std::string& Name(const Monitor& mointor) { return mointor.Name; }
		int Height(const Monitor& mointor) { return mointor.Height; }
		int Width(const Monitor& mointor) { return mointor.Width; }
		int Height(const ImageRect& rect) { return rect.bottom - rect.top; }
		int Width(const ImageRect& rect) { return rect.right - rect.left; }
		int Height(const Image& img) { return Height(img.Bounds); }
		int Width(const Image& img) { return Width(img.Bounds); }
		const ImageRect& Rect(const Image& img) { return img.Bounds; }

		//number of bytes per row, NOT including the Rowpadding
		int RowStride(const Image& img) { return img.Pixelstride* Width(img); }
		//number of bytes per row of padding
		int RowPadding(const Image& img) { return img.RowPadding; }
		char* StartSrc(const Image& img) { return img.Data; }
		void Copy(const Image& dst, const Image& src) {
			//make sure the copy is going to be valid!
			assert(dst.Bounds.Contains(src.Bounds));

			auto startdst = StartSrc(dst) + (src.Bounds.top *(RowStride(dst) + RowPadding(dst))) + (src.Bounds.left * dst.Pixelstride);
			auto startsrc = StartSrc(src);
			if (src.Bounds == dst.Bounds && RowStride(src) == RowStride(dst) && RowPadding(src) == RowPadding(dst)) {
				//if the bounds and rowstride and padding are the same, the entire copy can be a single memcpy
				memcpy(startdst, startsrc, RowStride(src)*Height(src));
			}
			else {
				for (auto i = 0; i < Height(src); i++) {
					//memset(startdst, 0, RowStride(src));
					memcpy(startdst, startsrc, RowStride(src));

					startdst += RowStride(dst) + RowPadding(dst);//advance to the next row
					startsrc += RowStride(src) + RowPadding(src);//advance to the next row
				}
			}
		}
	}
}


