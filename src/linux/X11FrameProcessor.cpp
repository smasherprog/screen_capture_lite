#include "X11FrameProcessor.h"
#include <X11/extensions/Xinerama.h>
#include <assert.h>
#include <vector>

namespace SL
{
namespace Screen_Capture
{
    X11FrameProcessor::X11FrameProcessor()
    {
    }

    X11FrameProcessor::~X11FrameProcessor()
    {

        if(ShmInfo) {
            shmdt(ShmInfo->shmaddr);
            shmctl(ShmInfo->shmid, IPC_RMID, 0);
            XShmDetach(SelectedDisplay, ShmInfo.get());
        }
        if(Image) {
            XDestroyImage(Image);
        }
        if(SelectedDisplay) {
            XCloseDisplay(SelectedDisplay);
        }
    }
    DUPL_RETURN X11FrameProcessor::Init(std::shared_ptr<Thread_Data> data, Monitor& monitor)
    {
        auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
        Data = data;
        SelectedMonitor = monitor;
        SelectedDisplay = XOpenDisplay(NULL);
        if(!SelectedDisplay) {
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
        }
        int scr = XDefaultScreen(SelectedDisplay);

        ShmInfo = std::make_unique<XShmSegmentInfo>();

        Image = XShmCreateImage(SelectedDisplay,
                                DefaultVisual(SelectedDisplay, scr),
                                DefaultDepth(SelectedDisplay, scr),
                                ZPixmap,
                                NULL,
                                ShmInfo.get(),
                                Width(SelectedMonitor),
                                Height(SelectedMonitor));
        ShmInfo->shmid = shmget(IPC_PRIVATE, Image->bytes_per_line * Image->height, IPC_CREAT | 0777);

        ShmInfo->readOnly = False;
        ShmInfo->shmaddr = Image->data = (char*)shmat(ShmInfo->shmid, 0, 0);

        XShmAttach(SelectedDisplay, ShmInfo.get());

        return ret;
    }
 
    DUPL_RETURN X11FrameProcessor::ProcessFrame(const Monitor& currentmonitorinfo)
    {
        auto Ret = DUPL_RETURN_SUCCESS;
        ImageRect ret;
        ret.left = ret.top = 0;
        ret.right = Width(SelectedMonitor);
        ret.bottom = Height(SelectedMonitor);
        
        if(!XShmGetImage(SelectedDisplay,
                         RootWindow(SelectedDisplay, DefaultScreen(SelectedDisplay)),
                         Image,
                         OffsetX(SelectedMonitor),
                         OffsetY(SelectedMonitor),
                         AllPlanes)) {
            return DUPL_RETURN_ERROR_EXPECTED;
        }

        if(Data->CaptureEntireMonitor && !Data->CaptureDifMonitor) {

            auto wholeimg = Create(ret, PixelStride, 0, reinterpret_cast<unsigned char*>(Image->data));
            Data->CaptureEntireMonitor(wholeimg, SelectedMonitor);
        } else {
            memcpy(NewImageBuffer.get(), Image->data, PixelStride * ret.right * ret.bottom);
            ProcessMonitorCapture(*Data, *this, SelectedMonitor, ret);
        }
        return Ret;
    }
}
}