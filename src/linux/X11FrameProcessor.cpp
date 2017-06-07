#include "X11FrameProcessor.h"

#include <assert.h>
#include <vector>


namespace SL {
	namespace Screen_Capture {
		X11FrameProcessor::X11FrameProcessor()
		{

		}

		X11FrameProcessor::~X11FrameProcessor()
		{

			if (ShmInfo) {
				shmdt(ShmInfo->shmaddr);
				shmctl(ShmInfo->shmid, IPC_RMID, 0);
				XShmDetach(SelectedDisplay, ShmInfo.get());
			}
			if (Image) {
				XDestroyImage(Image);
			}
			if (SelectedDisplay) {
				XCloseDisplay(SelectedDisplay);
			}
		}
		DUPL_RETURN X11FrameProcessor::Init(std::shared_ptr<Thread_Data> data, Monitor& monitor) {
			auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
			Data = data;
            SelectedMonitor = monitor;
			SelectedDisplay = XOpenDisplay(NULL);
			if (!SelectedDisplay) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}


			RootWindow = XRootWindow(SelectedDisplay, Index(SelectedMonitor));
			if (!RootWindow) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}
			auto visual = DefaultVisual(SelectedDisplay, Index(SelectedMonitor));
			auto depth = DefaultDepth(SelectedDisplay, Index(SelectedMonitor));

			ShmInfo = std::make_unique<XShmSegmentInfo>();

			Image = XShmCreateImage(SelectedDisplay, visual, depth, ZPixmap, NULL, ShmInfo.get(), Width(SelectedMonitor), Height(SelectedMonitor));
			ShmInfo->shmid = shmget(IPC_PRIVATE, Image->bytes_per_line * Image->height, IPC_CREAT | 0777);

			ShmInfo->readOnly = False;
			ShmInfo->shmaddr = Image->data = (char*)shmat(ShmInfo->shmid, 0, 0);

			XShmAttach(SelectedDisplay, ShmInfo.get());


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
			ret.right = Width(SelectedMonitor);
			ret.bottom = Height(SelectedMonitor);
            
            //check to see if the display has changed
            auto t = XOpenDisplay(NULL);
            auto monh = DisplayHeight(t, Id(SelectedMonitor));
            auto monw = DisplayWidth(t, Id(SelectedMonitor));
            XCloseDisplay(t);
            
            if( monh!= ret.bottom || monw!= ret.right){
                   return DUPL_RETURN_ERROR_EXPECTED;  
            }

			if(!XShmGetImage(SelectedDisplay, RootWindow, Image, 0, 0, AllPlanes)){
                return DUPL_RETURN_ERROR_EXPECTED;
            }
			memcpy(NewImageBuffer.get(), Image->data, PixelStride*ret.right*ret.bottom);
            ProcessMonitorCapture(*Data, *this, SelectedMonitor, ret);
			return Ret;
		}

	}
}