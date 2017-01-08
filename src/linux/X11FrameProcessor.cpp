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
		

std::shared_ptr<Image> SL::Screen_Capture::CaptureDesktopImage()
{
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

	auto px = Image::CreateImage(height, width, (char*)shminfo.shmaddr, image->bits_per_pixel / 8);
	assert(image->bits_per_pixel == 32); //this should always be true... Ill write a case where it isnt, but for now it should be

	XDestroyImage(image);
	shmdt(shminfo.shmaddr);
	shmctl(shminfo.shmid, IPC_RMID, 0);
	XCloseDisplay(display);

	return px;
}


        struct CGFrameProcessorImpl {

			std::shared_ptr<THREAD_DATA> Data;
			std::unique_ptr<char[]> OldImageBuffer, NewImageBuffer;
			size_t ImageBufferSize;
			bool FirstRun;
		};


		CGFrameProcessor::CGFrameProcessor()
		{
			_CGFrameProcessorImpl = std::make_unique<CGFrameProcessorImpl>();
			_CGFrameProcessorImpl->ImageBufferSize = 0;
			_CGFrameProcessorImpl->FirstRun = true;
		}

		CGFrameProcessor::~CGFrameProcessor()
		{

		}
		DUPL_RETURN CGFrameProcessor::Init(std::shared_ptr<THREAD_DATA> data) {
            auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
            _CGFrameProcessorImpl->Data = data;
            _CGFrameProcessorImpl->ImageBufferSize  = Height(*data->SelectedMonitor)*Width(*data->SelectedMonitor)*PixelStride;
          	if (_CGFrameProcessorImpl->Data->CaptureDifMonitor) {//only need the old buffer if difs are needed. If no dif is needed, then the image is always new
				_CGFrameProcessorImpl->OldImageBuffer = std::make_unique<char[]>(_CGFrameProcessorImpl->ImageBufferSize);
			}
			_CGFrameProcessorImpl->NewImageBuffer = std::make_unique<char[]>(_CGFrameProcessorImpl->ImageBufferSize);
		
            
            
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN CGFrameProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;
            auto imageRef = CGDisplayCreateImage(Id(*_CGFrameProcessorImpl->Data->SelectedMonitor));
        
            auto width = CGImageGetWidth(imageRef);
            auto height = CGImageGetHeight(imageRef);
            auto colorSpace = CGColorSpaceCreateDeviceRGB();
            auto rawData = _CGFrameProcessorImpl->NewImageBuffer.get();
            auto bytesPerPixel = 4;
            auto bytesPerRow = bytesPerPixel * width;
            auto bitsPerComponent = 8;
            auto context = CGBitmapContextCreate(rawData, width, height,
                                                         bitsPerComponent, bytesPerRow, colorSpace,
                                                         kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
            CGColorSpaceRelease(colorSpace);
            
            CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
            CGContextRelease(context);
            CGImageRelease(imageRef);
            ImageRect imgrect;
            imgrect.left =  imgrect.top=0;
            imgrect.right =width;
            imgrect.bottom = height;
            if(_CGFrameProcessorImpl->Data->CaptureEntireMonitor){
           
                auto img = CreateImage(imgrect, PixelStride, 0,_CGFrameProcessorImpl->NewImageBuffer.get());
                _CGFrameProcessorImpl->Data->CaptureEntireMonitor(*img, *_CGFrameProcessorImpl->Data->SelectedMonitor);
            }
			if (_CGFrameProcessorImpl->Data->CaptureDifMonitor) {
				if (_CGFrameProcessorImpl->FirstRun) {
						//first time through, just send the whole image
						auto wholeimgfirst = CreateImage(imgrect, PixelStride, 0, _CGFrameProcessorImpl->NewImageBuffer.get());
						_CGFrameProcessorImpl->Data->CaptureDifMonitor(*wholeimgfirst, *_CGFrameProcessorImpl->Data->SelectedMonitor);
						_CGFrameProcessorImpl->FirstRun = false;
				} else {		
				
				
					//user wants difs, lets do it!
					auto newimg = CreateImage(imgrect, PixelStride, 0, _CGFrameProcessorImpl->NewImageBuffer.get());
					auto oldimg = CreateImage(imgrect, PixelStride, 0, _CGFrameProcessorImpl->OldImageBuffer.get());
					auto imgdifs = GetDifs(*oldimg, *newimg);

					for (auto& r : imgdifs) {
						auto padding = (r.left *PixelStride) + ((Width(*newimg) - r.right)*PixelStride);
						auto startsrc = _CGFrameProcessorImpl->NewImageBuffer.get();
						startsrc += (r.left *PixelStride) + (r.top *PixelStride *Width(*newimg));

						auto difimg = CreateImage(r, PixelStride, padding, startsrc);
						_CGFrameProcessorImpl->Data->CaptureDifMonitor(*difimg, *_CGFrameProcessorImpl->Data->SelectedMonitor);

					}
				}
					std::swap(_CGFrameProcessorImpl->NewImageBuffer, _CGFrameProcessorImpl->OldImageBuffer);
				}
			return Ret;
		}

	}
}