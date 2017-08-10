#include "X11FrameProcessor.h"
#include <X11/Xutil.h>
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
    
    DUPL_RETURN X11FrameProcessor::Init(std::shared_ptr<Thread_Data> data, const Window& selectedwindow){
        
        auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
        Data = data; 
        SelectedDisplay = XOpenDisplay(NULL);
        SelectedWindow = selectedwindow.Handle;
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
                                selectedwindow.Size.x,
                                selectedwindow.Size.y);
        ShmInfo->shmid = shmget(IPC_PRIVATE, Image->bytes_per_line * Image->height, IPC_CREAT | 0777);

        ShmInfo->readOnly = False;
        ShmInfo->shmaddr = Image->data = (char*)shmat(ShmInfo->shmid, 0, 0);

        XShmAttach(SelectedDisplay, ShmInfo.get());

        return ret;
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

        if(Data->ScreenCaptureData.OnNewFrame && !Data->ScreenCaptureData.OnFrameChanged) {

            auto wholeimg = Create(ret, PixelStride, 0, reinterpret_cast<unsigned char*>(Image->data));
            Data->ScreenCaptureData.OnNewFrame(wholeimg, SelectedMonitor);
        } else {
            memcpy(NewImageBuffer.get(), Image->data, PixelStride * ret.right * ret.bottom);
            ProcessCapture(Data->ScreenCaptureData, *this, SelectedMonitor, ret);
        }
        return Ret;
    }
    DUPL_RETURN X11FrameProcessor::ProcessFrame(Window& selectedwindow){
        
        auto Ret = DUPL_RETURN_SUCCESS;
        ImageRect ret;
        ret.left = ret.top = 0;
        ret.right = selectedwindow.Size.x;
        ret.bottom = selectedwindow.Size.y;
        XWindowAttributes wndattr;
        if(XGetWindowAttributes(SelectedDisplay, SelectedWindow, &wndattr) ==0){
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;//window might not be valid any more
        }
        if(wndattr.width != ret.right || wndattr.height != ret.bottom){
            return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;//window size changed. This will rebuild everything
        }
        if(!XShmGetImage(SelectedDisplay,
                         selectedwindow.Handle,
                         Image,
                         0,
                         0,
                         AllPlanes)) {
            return DUPL_RETURN_ERROR_EXPECTED;
        }

        if(Data->WindowCaptureData.OnNewFrame && !Data->WindowCaptureData.OnFrameChanged) {

            auto wholeimg = Create(ret, PixelStride, 0, reinterpret_cast<unsigned char*>(Image->data));
            Data->WindowCaptureData.OnNewFrame(wholeimg, selectedwindow);
        } else {
            memcpy(NewImageBuffer.get(), Image->data, PixelStride * ret.right * ret.bottom);
            ProcessCapture(Data->WindowCaptureData, *this, selectedwindow, ret);
        }
        return Ret;
    }
}
}