#include "ScreenCapture.h"
#include "SCCommon.h"
#include "ThreadManager.h"
#include <thread>
#include <atomic>
#include <algorithm>
#include <memory>
#include <assert.h>
#include <cstring>
#include <iostream>

namespace SL
{
namespace Screen_Capture
{
    bool isMonitorInsideBounds(const std::vector<Monitor>& monitors, const Monitor& monitor)
    {

        auto totalwidth = 0;
        for(auto& m : monitors) {
            totalwidth += Width(m);
        }

        // if the monitor doesnt exist any more!
        if(std::find_if(begin(monitors), end(monitors), [&](auto& m) { return m.Id == monitor.Id; }) == end(monitors)) {
            return false;
        } // if the area to capture is outside the dimensions of the desktop!!
        auto& realmonitor = monitors[Index(monitor)];
        if(Height(realmonitor) < Height(monitor) || // monitor height check
           totalwidth < Width(monitor) + OffsetX(monitor) ||               // total width check
           Width(monitor) > Width(realmonitor)) // regular width check
           
        {
            return false;
        } // if the entire screen is capture and the offsets changed, get out and rebuild
        else if(Height(realmonitor) == Height(monitor) && Width(realmonitor) == Width(monitor) &&
                (OffsetX(realmonitor) != OffsetX(monitor) || OffsetY(realmonitor) != OffsetY(monitor))) {
            return false;
        }
        return true;
    }
    class ScreenCaptureManagerImpl
    {
    public:
        // allreads share the same data!!!
        std::shared_ptr<Thread_Data> Thread_Data_;

        std::thread Thread_;

        ScreenCaptureManagerImpl()
        {
            Thread_Data_ = std::make_shared<Thread_Data>();
            Thread_Data_->Paused = false;
            Thread_Data_->Monitor_Capture_Timer = std::make_shared<Timer<long long, std::milli> >(100ms);
            Thread_Data_->Mouse_Capture_Timer = std::make_shared<Timer<long long, std::milli> >(50ms);
        }
        ~ScreenCaptureManagerImpl()
        {
            Thread_Data_->TerminateThreadsEvent = true; // set the exit flag for the threads
            Thread_Data_->Paused = false;               // unpaused the threads to let everything exit
            if(Thread_.joinable()) {
                Thread_.join();
            }
        }
        void start()
        {

            // users must set at least one callback before starting
            assert(Thread_Data_->CaptureEntireMonitor || Thread_Data_->CaptureDifMonitor || Thread_Data_->CaptureMouse);
            Thread_ = std::thread([&]() {
                ThreadManager ThreadMgr;

                ThreadMgr.Init(Thread_Data_);

                while(!Thread_Data_->TerminateThreadsEvent) {

                    if(Thread_Data_->ExpectedErrorEvent) {
                        Thread_Data_->TerminateThreadsEvent = true;
                        ThreadMgr.Join();
                        Thread_Data_->ExpectedErrorEvent = Thread_Data_->UnexpectedErrorEvent =
                            Thread_Data_->TerminateThreadsEvent = false;
                        // Clean up
                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(1000)); // sleep for 1 second since an error occcured

                        ThreadMgr.Init(Thread_Data_);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                Thread_Data_->TerminateThreadsEvent = true;
                ThreadMgr.Join();
            });
        }
    };
    void ScreenCaptureManager::setFrameChangeInterval_(const std::shared_ptr<ITimer>& timer)
    {
        std::atomic_store(&Impl_->Thread_Data_->Monitor_Capture_Timer, timer);
    }
    void ScreenCaptureManager::setMouseChangeInterval_(const std::shared_ptr<ITimer>& timer)
    {
        std::atomic_store(&Impl_->Thread_Data_->Mouse_Capture_Timer, timer);
    }

    ScreenCaptureManager ScreenCaptureConfiguration::start_capturing()
    {
        Impl_->start();
        return ScreenCaptureManager(Impl_);
    }
    void ScreenCaptureManager::pause()
    {
        Impl_->Thread_Data_->Paused = true;
    }
    bool ScreenCaptureManager::isPaused() const
    {
        return Impl_->Thread_Data_->Paused;
    }
    void ScreenCaptureManager::resume()
    {
        Impl_->Thread_Data_->Paused = false;
    }

    ScreenCaptureConfiguration ScreenCaptureConfiguration::onNewFrame(const CaptureCallback& cb)
    {
        assert(!Impl_->Thread_Data_->CaptureEntireMonitor);
        Impl_->Thread_Data_->CaptureEntireMonitor = cb;
        return ScreenCaptureConfiguration(Impl_);
    }
    ScreenCaptureConfiguration ScreenCaptureConfiguration::onFrameChanged(const CaptureCallback& cb)
    {
        assert(!Impl_->Thread_Data_->CaptureDifMonitor);
        Impl_->Thread_Data_->CaptureDifMonitor = cb;
        return ScreenCaptureConfiguration(Impl_);
    }
    ScreenCaptureConfiguration ScreenCaptureConfiguration::onMouseChanged(const MouseCallback& cb)
    {
        assert(!Impl_->Thread_Data_->CaptureMouse);
        Impl_->Thread_Data_->CaptureMouse = cb;
        return ScreenCaptureConfiguration(Impl_);
    }
    ScreenCaptureConfiguration CreateScreeCapture(const MonitorCallback& monitorstocapture)
    {
        auto impl = std::make_shared<ScreenCaptureManagerImpl>();
        impl->Thread_Data_->MonitorsChanged = monitorstocapture;
        return ScreenCaptureConfiguration(impl);
    }
}
}
