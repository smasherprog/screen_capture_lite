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

			ScreenCapture_Settings Settings;

			std::thread _Thread;
			std::shared_ptr<std::atomic_bool> _TerminateThread;
			

			ScreenCaptureManagerImpl() {

			}
			~ScreenCaptureManagerImpl() {
				stop();
			}
			void start() {
				//users must set the monitors to capture before calling start
				assert(!Settings.Monitors.empty());
				//users must set at least one callback before starting
				assert(Settings.CaptureEntireMonitor || Settings.CaptureDifMonitor || Settings.CaptureMouse);

				stop();
				_Thread = std::thread([&]() {
					ThreadManager ThreadMgr;
					Base_Thread_Data data;
					data.ExpectedErrorEvent = std::make_shared<std::atomic_bool>(false);
					data.UnexpectedErrorEvent = std::make_shared<std::atomic_bool>(false);
					data.TerminateThreadsEvent = _TerminateThread = std::make_shared<std::atomic_bool>(false);

					ThreadMgr.Init(data, Settings);

					while (!*_TerminateThread) {

						if (*data.ExpectedErrorEvent)
						{

							// Terminate other threads
							*_TerminateThread = true;
							ThreadMgr.Join();
							*data.ExpectedErrorEvent = *data.UnexpectedErrorEvent = *_TerminateThread = false;
							// Clean up
							ThreadMgr.Reset();
							std::this_thread::sleep_for(std::chrono::milliseconds(1000));//sleep for 1 second since an error occcured

							ThreadMgr.Init(data, Settings);
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

		void ScreenCaptureManager::StartCapturing(const ScreenCapture_Settings& s)
		{
			_ScreenCaptureManagerImpl->Settings = s;
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


