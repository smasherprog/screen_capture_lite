#pragma once
#include "ScreenCapture.h"
#include <thread>
#include <atomic>

// this is INTERNAL DO NOT USE!
namespace SL {
    namespace Screen_Capture {

        struct ScreenCapture_Settings {
            //min interval between frames that are captured
            int Monitor_Capture_Interval;
            //set this if you want to capture the entire monitor each interval
            CaptureCallback CaptureEntireMonitor;
            //set this if you want to receive difs each interval on what has changed
            CaptureCallback CaptureDifMonitor;
            //min interval between mouse captures
            int Mouse_Capture_Interval;
            //the function to be called on each mouse interval. If a the mouse image has changed, img will not be null, otherwise, the only change is new mouse coords
            MouseCallback CaptureMouse;
            //get monitors to watch
            MonitorCallback MonitorsChanged;
        };

        struct Base_Thread_Data
        {
            // Used to indicate abnormal error condition
            std::shared_ptr<std::atomic_bool> UnexpectedErrorEvent;
            // Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate the duplication interface
            std::shared_ptr<std::atomic_bool> ExpectedErrorEvent;
            // Used to signal to threads to exit
            std::shared_ptr<std::atomic_bool> TerminateThreadsEvent;
        };

        struct Monitor_Thread_Data : Base_Thread_Data
        {

            Monitor SelectedMonitor;

            int CaptureInterval; //in milliseconds	
            CaptureCallback CaptureEntireMonitor;
            CaptureCallback CaptureDifMonitor;

            std::unique_ptr<char[]> OldImageBuffer, NewImageBuffer;
            size_t ImageBufferSize = 0;
            bool FirstRun = true;
        };

        struct Mouse_Thread_Data : Base_Thread_Data
        {

            int CaptureInterval; //in milliseconds	
            MouseCallback CaptureCallback;
        };
        enum DUPL_RETURN
        {
            DUPL_RETURN_SUCCESS = 0,
            DUPL_RETURN_ERROR_EXPECTED = 1,
            DUPL_RETURN_ERROR_UNEXPECTED = 2
        };
        const int PixelStride = 4;
        std::shared_ptr<Monitor> CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string& n);

        Image Create(const ImageRect& imgrect, int pixelstride, int rowpadding, char* data);
        //this function will copy data from the src into the dst. The only requirement is that src must not be larger than dst, but it can be smaller
       // void Copy(const Image& dst, const Image& src);

        std::vector<ImageRect> GetDifs(const Image & oldimg, const Image & newimg);
        void ProcessMonitorCapture(Monitor_Thread_Data& data, ImageRect& imageract);

    }
}