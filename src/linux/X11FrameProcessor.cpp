#include "X11FrameProcessor.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <sys/shm.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/XShm.h>

namespace SL {
	namespace Screen_Capture {
		
        struct X11FrameProcessorImpl {

			std::shared_ptr<THREAD_DATA> Data;
			std::unique_ptr<char[]> OldImageBuffer, NewImageBuffer;
			size_t ImageBufferSize;
			bool FirstRun;
		};


		X11FrameProcessor::X11FrameProcessor()
		{
			_X11FrameProcessorImpl = std::make_unique<X11FrameProcessorImpl>();
			_X11FrameProcessorImpl->ImageBufferSize = 0;
			_X11FrameProcessorImpl->FirstRun = true;
		}

		X11FrameProcessor::~X11FrameProcessor()
		{

		}
		DUPL_RETURN X11FrameProcessor::Init(std::shared_ptr<THREAD_DATA> data) {
            auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
            _X11FrameProcessorImpl->Data = data;
            _X11FrameProcessorImpl->ImageBufferSize  = Height(*data->SelectedMonitor)*Width(*data->SelectedMonitor)*PixelStride;
          	if (_X11FrameProcessorImpl->Data->CaptureDifMonitor) {//only need the old buffer if difs are needed. If no dif is needed, then the image is always new
				_X11FrameProcessorImpl->OldImageBuffer = std::make_unique<char[]>(_X11FrameProcessorImpl->ImageBufferSize);
			}
			_X11FrameProcessorImpl->NewImageBuffer = std::make_unique<char[]>(_X11FrameProcessorImpl->ImageBufferSize);
		
            
            
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN X11FrameProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;
            
            
	auto display = XOpenDisplay(NULL);
	auto root = DefaultRootWindow(display);
	auto screen = XDefaultScreen(display);
	auto visual = DefaultVisual(display, screen);
	auto depth = DefaultDepth(display, screen);

	XWindowAttributes gwa;
	XGetWindowAttributes(display, root, &gwa);
	auto width = gwa.width;
	auto height = gwa.height;

	XShmSegmentInfo shminfo;
	auto image = XShmCreateImage(display, visual, depth, ZPixmap, NULL, &shminfo, width, height);
	shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);

	shminfo.readOnly = False;
	shminfo.shmaddr = image->data = (char*)shmat(shminfo.shmid, 0, 0);

	XShmAttach(display, &shminfo);

	XShmGetImage(display, root, image, 0, 0, AllPlanes);

	XShmDetach(display, &shminfo);

	//auto px = Image::CreateImage(height, width, (char*)shminfo.shmaddr, image->bits_per_pixel / 8);
	//assert(image->bits_per_pixel == 32); //this should always be true... Ill write a case where it isnt, but for now it should be

	XDestroyImage(image);
	shmdt(shminfo.shmaddr);
	shmctl(shminfo.shmid, IPC_RMID, 0);
	XCloseDisplay(display);

            ImageRect imgrect;
            imgrect.left =  imgrect.top=0;
            imgrect.right =width;
            imgrect.bottom = height;
            if(_X11FrameProcessorImpl->Data->CaptureEntireMonitor){
           
                auto img = CreateImage(imgrect, PixelStride, 0,_X11FrameProcessorImpl->NewImageBuffer.get());
                _X11FrameProcessorImpl->Data->CaptureEntireMonitor(*img, *_X11FrameProcessorImpl->Data->SelectedMonitor);
            }
			if (_X11FrameProcessorImpl->Data->CaptureDifMonitor) {
				if (_X11FrameProcessorImpl->FirstRun) {
						//first time through, just send the whole image
						auto wholeimgfirst = CreateImage(imgrect, PixelStride, 0, _X11FrameProcessorImpl->NewImageBuffer.get());
						_X11FrameProcessorImpl->Data->CaptureDifMonitor(*wholeimgfirst, *_X11FrameProcessorImpl->Data->SelectedMonitor);
						_X11FrameProcessorImpl->FirstRun = false;
				} else {		
				
				
					//user wants difs, lets do it!
					auto newimg = CreateImage(imgrect, PixelStride, 0, _X11FrameProcessorImpl->NewImageBuffer.get());
					auto oldimg = CreateImage(imgrect, PixelStride, 0, _X11FrameProcessorImpl->OldImageBuffer.get());
					auto imgdifs = GetDifs(*oldimg, *newimg);

					for (auto& r : imgdifs) {
						auto padding = (r.left *PixelStride) + ((Width(*newimg) - r.right)*PixelStride);
						auto startsrc = _X11FrameProcessorImpl->NewImageBuffer.get();
						startsrc += (r.left *PixelStride) + (r.top *PixelStride *Width(*newimg));

						auto difimg = CreateImage(r, PixelStride, padding, startsrc);
						_X11FrameProcessorImpl->Data->CaptureDifMonitor(*difimg, *_X11FrameProcessorImpl->Data->SelectedMonitor);

					}
				}
					std::swap(_X11FrameProcessorImpl->NewImageBuffer, _X11FrameProcessorImpl->OldImageBuffer);
				}
			return Ret;
		}

	}
}