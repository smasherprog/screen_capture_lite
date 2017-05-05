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
#include <assert.h>
#include <vector>


namespace SL {
	namespace Screen_Capture {

		struct X11FrameProcessorImpl {

			std::shared_ptr<Monitor_Thread_Data> Data;
			Display* SelectedDisplay;
			Window RootWindow;
			XImage* Image;
			std::unique_ptr<XShmSegmentInfo> ShmInfo;

		};

		X11FrameProcessor::X11FrameProcessor()
		{
			_X11FrameProcessorImpl = std::make_unique<X11FrameProcessorImpl>();
			_X11FrameProcessorImpl->SelectedDisplay = nullptr;
			_X11FrameProcessorImpl->Image = nullptr;
		}

		X11FrameProcessor::~X11FrameProcessor()
		{

			if (_X11FrameProcessorImpl->ShmInfo) {
				shmdt(_X11FrameProcessorImpl->ShmInfo->shmaddr);
				shmctl(_X11FrameProcessorImpl->ShmInfo->shmid, IPC_RMID, 0);
				XShmDetach(_X11FrameProcessorImpl->SelectedDisplay, _X11FrameProcessorImpl->ShmInfo.get());
			}
			if (_X11FrameProcessorImpl->Image) {
				XDestroyImage(_X11FrameProcessorImpl->Image);
			}
			if (_X11FrameProcessorImpl->SelectedDisplay) {
				XCloseDisplay(_X11FrameProcessorImpl->SelectedDisplay);
			}
		}
		DUPL_RETURN X11FrameProcessor::Init(std::shared_ptr<Monitor_Thread_Data> data) {
			auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
			_X11FrameProcessorImpl->Data = data;
		
			_X11FrameProcessorImpl->SelectedDisplay = XOpenDisplay(NULL);
			if (!_X11FrameProcessorImpl->SelectedDisplay) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}


			_X11FrameProcessorImpl->RootWindow = XRootWindow(_X11FrameProcessorImpl->SelectedDisplay, Index(_X11FrameProcessorImpl->Data->SelectedMonitor));
			if (!_X11FrameProcessorImpl->RootWindow) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}
			auto visual = DefaultVisual(_X11FrameProcessorImpl->SelectedDisplay, Index(_X11FrameProcessorImpl->Data->SelectedMonitor));
			auto depth = DefaultDepth(_X11FrameProcessorImpl->SelectedDisplay, Index(_X11FrameProcessorImpl->Data->SelectedMonitor));

			_X11FrameProcessorImpl->ShmInfo = std::make_unique<XShmSegmentInfo>();

			_X11FrameProcessorImpl->Image = XShmCreateImage(_X11FrameProcessorImpl->SelectedDisplay, visual, depth, ZPixmap, NULL, _X11FrameProcessorImpl->ShmInfo.get(), Width(_X11FrameProcessorImpl->Data->SelectedMonitor), Height(_X11FrameProcessorImpl->Data->SelectedMonitor));
			_X11FrameProcessorImpl->ShmInfo->shmid = shmget(IPC_PRIVATE, _X11FrameProcessorImpl->Image->bytes_per_line * _X11FrameProcessorImpl->Image->height, IPC_CREAT | 0777);

			_X11FrameProcessorImpl->ShmInfo->readOnly = False;
			_X11FrameProcessorImpl->ShmInfo->shmaddr = _X11FrameProcessorImpl->Image->data = (char*)shmat(_X11FrameProcessorImpl->ShmInfo->shmid, 0, 0);

			XShmAttach(_X11FrameProcessorImpl->SelectedDisplay, _X11FrameProcessorImpl->ShmInfo.get());


			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN X11FrameProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;
			ImageRect ret;
			ret.left = ret.top = 0;
			ret.right = Width(_X11FrameProcessorImpl->Data->SelectedMonitor);
			ret.bottom = Height(_X11FrameProcessorImpl->Data->SelectedMonitor);
            
            //check to see if the display has changed
            auto t = XOpenDisplay(NULL);
            auto monh = DisplayHeight(t, Id(_X11FrameProcessorImpl->Data->SelectedMonitor));
            auto monw = DisplayWidth(t, Id(_X11FrameProcessorImpl->Data->SelectedMonitor));
            XCloseDisplay(t);
            
            if( monh!= ret.bottom || monw!= ret.right){
                   return DUPL_RETURN_ERROR_EXPECTED;  
            }

			if(!XShmGetImage(_X11FrameProcessorImpl->SelectedDisplay, _X11FrameProcessorImpl->RootWindow, _X11FrameProcessorImpl->Image, 0, 0, AllPlanes)){
                return DUPL_RETURN_ERROR_EXPECTED;
            }
			memcpy(_X11FrameProcessorImpl->Data->NewImageBuffer.get(), _X11FrameProcessorImpl->Image->data, PixelStride*ret.right*ret.bottom);
            ProcessMonitorCapture(*_X11FrameProcessorImpl->Data, ret);
			return Ret;
		}

	}
}