#include "ScreenCapture.h"
#include "internal/SCCommon.h"
#include "internal/ThreadManager.h"
#include <algorithm>
#include <assert.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <memory>
#include <span>
#include <thread>

namespace SL {
namespace Screen_Capture {
    namespace C_API {
        class ICaptureConfigurationScreenCaptureCallbackWrapper {
          public:
            std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> ptr;
        };
        class ICaptureConfigurationWindowCaptureCallbackWrapper {
          public:
            std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> ptr;
        };
        class IScreenCaptureManagerWrapper {
          public:
            std::shared_ptr<IScreenCaptureManager> ptr;
        };
        int GetMonitors(Monitor *monitors, int monitors_size)
        {
            auto local_monitors = Screen_Capture::GetMonitors();
            auto maxelements = std::clamp(static_cast<int>(local_monitors.size()), 0, monitors_size);
            memcpy(monitors, local_monitors.data(), maxelements * sizeof(Monitor));
            return static_cast<int>(local_monitors.size());
        }

        int GetWindows(Window *windows, int monitors_size)
        {
            auto local_windows = Screen_Capture::GetWindows();
            auto maxelements = std::clamp(static_cast<int>(local_windows.size()), 0, monitors_size);
            memcpy(windows, local_windows.data(), maxelements * sizeof(Window));
            return static_cast<int>(local_windows.size());
        }

    }; // namespace C_API
    template <class MONITORTPE> bool isMonitorInsideBounds(MONITORTPE monitors, const Monitor &monitor)
    {
        auto totalwidth = 0;
        for (auto &m : monitors) {
            totalwidth += Width(m);
        }
        // if the monitor doesnt exist any more!
        if (std::find_if(begin(monitors), end(monitors), [&](auto &m) { return m.Id == monitor.Id; }) == end(monitors)) {
            return false;
        } // if the area to capture is outside the dimensions of the desktop!!
        auto &realmonitor = monitors[Index(monitor)];
        if (Height(realmonitor) < Height(monitor) ||          // monitor height check
            totalwidth < Width(monitor) + OffsetX(monitor) || // total width check
            Width(monitor) > Width(realmonitor))              // regular width check

        {
            return false;
        } // if the entire screen is capture and the offsets changed, get out and rebuild
        else if (Height(realmonitor) == Height(monitor) && Width(realmonitor) == Width(monitor) &&
                 (OffsetX(realmonitor) != OffsetX(monitor) || OffsetY(realmonitor) != OffsetY(monitor))) {
            return false;
        }
        return true;
    }

    bool isMonitorInsideBounds(const std::vector<Monitor> &monitors, const Monitor &monitor)
    {
        return isMonitorInsideBounds(std::span(monitors.data(), monitors.size()), monitor);
    }
    namespace C_API {
        bool isMonitorInsideBounds(const Monitor *monitors, const int monitorsize, const Monitor *monitor)
        {
            return isMonitorInsideBounds(std::span(monitors, monitorsize), *monitor);
        }
    }; // namespace C_API
    static bool ScreenCaptureManagerExists = false;
    class ScreenCaptureManager : public IScreenCaptureManager {

      public:
        // allreads share the same data!!!
        std::shared_ptr<Thread_Data> Thread_Data_;

        std::thread Thread_;

        ScreenCaptureManager()
        {
            // you must ONLY HAVE ONE INSTANCE RUNNING AT A TIME. Destroy the first instance then create one!
            assert(!ScreenCaptureManagerExists);
            ScreenCaptureManagerExists = true;
            Thread_Data_ = std::make_shared<Thread_Data>();
            Thread_Data_->CommonData_.Paused = false;
            Thread_Data_->ScreenCaptureData.FrameTimer = std::make_shared<Timer>(100ms);
            Thread_Data_->ScreenCaptureData.MouseTimer = std::make_shared<Timer>(50ms);
            Thread_Data_->WindowCaptureData.FrameTimer = std::make_shared<Timer>(100ms);
            Thread_Data_->WindowCaptureData.MouseTimer = std::make_shared<Timer>(50ms);
        }
        virtual ~ScreenCaptureManager()
        {
            Thread_Data_->CommonData_.TerminateThreadsEvent = true; // set the exit flag for the threads
            Thread_Data_->CommonData_.Paused = false;               // unpaused the threads to let everything exit
            if (Thread_.get_id() == std::this_thread::get_id()) {
                Thread_.detach();
            }
            else if (Thread_.joinable()) {
                Thread_.join();
            }
            ScreenCaptureManagerExists = false;
        }
        void start()
        {
            Thread_ = std::thread([&]() {
                ThreadManager ThreadMgr;

                ThreadMgr.Init(Thread_Data_);

                while (!Thread_Data_->CommonData_.TerminateThreadsEvent) {

                    if (Thread_Data_->CommonData_.ExpectedErrorEvent) {
                        Thread_Data_->CommonData_.TerminateThreadsEvent = true;
                        ThreadMgr.Join();
                        Thread_Data_->CommonData_.ExpectedErrorEvent = Thread_Data_->CommonData_.UnexpectedErrorEvent =
                            Thread_Data_->CommonData_.TerminateThreadsEvent = false;
                        // Clean up
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // sleep for 1 second since an error occcured

                        ThreadMgr.Init(Thread_Data_);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                Thread_Data_->CommonData_.TerminateThreadsEvent = true;
                ThreadMgr.Join();
            });
        }
        virtual void setFrameChangeInterval(const std::shared_ptr<Timer> &timer) override
        {
#if defined(_WIN32) && defined(__cplusplus) && __cplusplus >= 202002L
            Thread_Data_->ScreenCaptureData.FrameTimer.store(timer);
            Thread_Data_->WindowCaptureData.FrameTimer.store(timer);
#else
            std::atomic_store(&Thread_Data_->ScreenCaptureData.FrameTimer, timer);
            std::atomic_store(&Thread_Data_->WindowCaptureData.FrameTimer, timer);
#endif  
        }
        virtual void setMouseChangeInterval(const std::shared_ptr<Timer> &timer) override
        {
#if defined(_WIN32) && defined(__cplusplus) && __cplusplus >= 202002L
            Thread_Data_->ScreenCaptureData.MouseTimer.store(timer);
            Thread_Data_->WindowCaptureData.MouseTimer.store(timer);
#else
            std::atomic_store(&Thread_Data_->ScreenCaptureData.MouseTimer, timer);
            std::atomic_store(&Thread_Data_->WindowCaptureData.MouseTimer, timer);
#endif  
        }
        virtual void pause() override { Thread_Data_->CommonData_.Paused = true; }
        virtual bool isPaused() const override { return Thread_Data_->CommonData_.Paused; }
        virtual void resume() override { Thread_Data_->CommonData_.Paused = false; }
    };

    class ScreenCaptureConfiguration : public ICaptureConfiguration<ScreenCaptureCallback> {
        std::shared_ptr<ScreenCaptureManager> Impl_;

      public:
        ScreenCaptureConfiguration(const std::shared_ptr<ScreenCaptureManager> &impl) : Impl_(impl) {}

        virtual std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> onNewFrame(const ScreenCaptureCallback &cb) override
        {
            assert(cb);
            assert(!Impl_->Thread_Data_->ScreenCaptureData.OnNewFrame);
            Impl_->Thread_Data_->ScreenCaptureData.OnNewFrame = cb;
            return std::make_shared<ScreenCaptureConfiguration>(Impl_);
        }
        virtual std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> onFrameChanged(const ScreenCaptureCallback &cb) override
        {
            assert(cb);
            assert(!Impl_->Thread_Data_->ScreenCaptureData.OnFrameChanged);
            Impl_->Thread_Data_->ScreenCaptureData.OnFrameChanged = cb;
            return std::make_shared<ScreenCaptureConfiguration>(Impl_);
        }
        virtual std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> onMouseChanged(const MouseCallback &cb) override
        {
            assert(cb);
            assert(!Impl_->Thread_Data_->ScreenCaptureData.OnMouseChanged);
            Impl_->Thread_Data_->ScreenCaptureData.OnMouseChanged = cb;
            return std::make_shared<ScreenCaptureConfiguration>(Impl_);
        }
        virtual std::shared_ptr<IScreenCaptureManager> start_capturing() override
        {
            assert(Impl_->Thread_Data_->ScreenCaptureData.OnMouseChanged || Impl_->Thread_Data_->ScreenCaptureData.OnFrameChanged ||
                   Impl_->Thread_Data_->ScreenCaptureData.OnNewFrame);
            Impl_->start();
            return Impl_;
        }
    };
    namespace C_API {
        void MonitoronNewFrame(ICaptureConfigurationScreenCaptureCallbackWrapper *ptr, C_API_ScreenCaptureCallback cb)
        {
            ptr->ptr = ptr->ptr->onNewFrame(cb);
        }
        void MonitoronFrameChanged(ICaptureConfigurationScreenCaptureCallbackWrapper *ptr, C_API_ScreenCaptureCallback cb)
        {
            ptr->ptr = ptr->ptr->onFrameChanged(cb);
        }
        void MonitoronMouseChanged(ICaptureConfigurationScreenCaptureCallbackWrapper *ptr, C_API_MouseCaptureCallback cb)
        {
            ptr->ptr = ptr->ptr->onMouseChanged(cb);
        } 
        IScreenCaptureManagerWrapper *Monitorstart_capturing(ICaptureConfigurationScreenCaptureCallbackWrapper *ptr)
        {
            auto p = new IScreenCaptureManagerWrapper{ptr->ptr->start_capturing()};
            FreeMonitorCaptureConfiguration(ptr);
            return p;
        }

        void setFrameChangeInterval(IScreenCaptureManagerWrapper *ptr, int milliseconds)
        {
            ptr->ptr->setFrameChangeInterval(std::chrono::milliseconds(milliseconds));
        }
        void setMouseChangeInterval(IScreenCaptureManagerWrapper *ptr, int milliseconds)
        {
            ptr->ptr->setMouseChangeInterval(std::chrono::milliseconds(milliseconds));
        }
        void pausecapturing(IScreenCaptureManagerWrapper *ptr) { ptr->ptr->pause(); }
        bool isPaused(IScreenCaptureManagerWrapper *ptr) { return ptr->ptr->isPaused(); }
        void resume(IScreenCaptureManagerWrapper *ptr) { ptr->ptr->resume(); }

        void FreeIScreenCaptureManagerWrapper(IScreenCaptureManagerWrapper *ptr) { delete ptr; }

        void WindowonNewFrame(ICaptureConfigurationWindowCaptureCallbackWrapper *ptr, C_API_WindowCaptureCallback cb)
        {
            ptr->ptr = ptr->ptr->onNewFrame(cb);
        }
        void WindowonFrameChanged(ICaptureConfigurationWindowCaptureCallbackWrapper *ptr, C_API_WindowCaptureCallback cb)
        {
            ptr->ptr = ptr->ptr->onFrameChanged(cb);
        }
        void WindowonMouseChanged(ICaptureConfigurationWindowCaptureCallbackWrapper *ptr, C_API_MouseCaptureCallback cb)
        {
            ptr->ptr = ptr->ptr->onMouseChanged(cb);
        } 
        IScreenCaptureManagerWrapper *Windowstart_capturing(ICaptureConfigurationWindowCaptureCallbackWrapper *ptr)
        {
            auto p = new IScreenCaptureManagerWrapper{ptr->ptr->start_capturing()};
            FreeWindowCaptureConfiguration(ptr);
            return p;
        }

    }; // namespace C_API

    class WindowCaptureConfiguration : public ICaptureConfiguration<WindowCaptureCallback> {
        std::shared_ptr<ScreenCaptureManager> Impl_;

      public:
        WindowCaptureConfiguration(const std::shared_ptr<ScreenCaptureManager> &impl) : Impl_(impl) {}

        virtual std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> onNewFrame(const WindowCaptureCallback &cb) override
        {
            assert(!Impl_->Thread_Data_->WindowCaptureData.OnNewFrame);
            Impl_->Thread_Data_->WindowCaptureData.OnNewFrame = cb;
            return std::make_shared<WindowCaptureConfiguration>(Impl_);
        }
        virtual std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> onFrameChanged(const WindowCaptureCallback &cb) override
        {
            assert(!Impl_->Thread_Data_->WindowCaptureData.OnFrameChanged);
            Impl_->Thread_Data_->WindowCaptureData.OnFrameChanged = cb;
            return std::make_shared<WindowCaptureConfiguration>(Impl_);
        }
        virtual std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> onMouseChanged(const MouseCallback &cb) override
        {

            assert(!Impl_->Thread_Data_->WindowCaptureData.OnMouseChanged);
            Impl_->Thread_Data_->WindowCaptureData.OnMouseChanged = cb;
            return std::make_shared<WindowCaptureConfiguration>(Impl_);
        }
        virtual std::shared_ptr<IScreenCaptureManager> start_capturing() override
        {
            assert(Impl_->Thread_Data_->WindowCaptureData.OnMouseChanged || Impl_->Thread_Data_->WindowCaptureData.OnFrameChanged ||
                   Impl_->Thread_Data_->WindowCaptureData.OnNewFrame);
            Impl_->start();
            return Impl_;
        }
    };

    namespace C_API {
        ICaptureConfigurationScreenCaptureCallbackWrapper *CreateMonitorCaptureConfiguration(C_API_MonitorCallback monitorstocapture)
        {
            auto p = new ICaptureConfigurationScreenCaptureCallbackWrapper();
            static auto maxbuffersize = 16;
            p->ptr = Screen_Capture::CreateCaptureConfiguration([=]() {
                std::vector<Monitor> buffer;
                auto sizeguess = maxbuffersize;
                buffer.resize(sizeguess);
                auto sizeneeded = monitorstocapture(buffer.data(), sizeguess);
                if (sizeguess < sizeneeded) {
                    sizeguess = sizeneeded;
                    maxbuffersize = std::max(sizeneeded, maxbuffersize);
                    buffer.resize(sizeguess);
                    sizeneeded = monitorstocapture(buffer.data(), sizeguess);
                    sizeguess = std::min(sizeguess, sizeneeded);
                    buffer.resize(sizeguess);
                    return buffer;
                }
                buffer.resize(sizeneeded);
                return buffer;
            });
            return p;
        }
        ICaptureConfigurationWindowCaptureCallbackWrapper *CreateWindowCaptureConfiguration(C_API_WindowCallback monitorstocapture)
        {
            auto p = new ICaptureConfigurationWindowCaptureCallbackWrapper();
            static auto maxbuffersize = 16;
            p->ptr = Screen_Capture::CreateCaptureConfiguration([=]() {
                std::vector<Window> buffer;
                auto sizeguess = maxbuffersize;
                buffer.resize(sizeguess);
                auto sizeneeded = monitorstocapture(buffer.data(), sizeguess);
                if (sizeguess < sizeneeded) {
                    sizeguess = sizeneeded;
                    maxbuffersize = std::max(sizeneeded, maxbuffersize);
                    buffer.resize(sizeguess);
                    sizeneeded = monitorstocapture(buffer.data(), sizeguess);
                    sizeguess = std::min(sizeguess, sizeneeded);
                    buffer.resize(sizeguess);
                    return buffer;
                }
                buffer.resize(sizeneeded);
                return buffer;
            });
            return p;
        }
        void FreeMonitorCaptureConfiguration(ICaptureConfigurationScreenCaptureCallbackWrapper *ptr) { delete ptr; }
        void FreeWindowCaptureConfiguration(ICaptureConfigurationWindowCaptureCallbackWrapper *ptr) { delete ptr; }
    }; // namespace C_API

    std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> CreateCaptureConfiguration(const MonitorCallback &monitorstocapture)
    {
        assert(monitorstocapture);
        auto impl = std::make_shared<ScreenCaptureManager>();
        impl->Thread_Data_->ScreenCaptureData.getThingsToWatch = monitorstocapture;
        return std::make_shared<ScreenCaptureConfiguration>(impl);
    }
    std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> CreateCaptureConfiguration(const WindowCallback &windowtocapture)
    {
        auto impl = std::make_shared<ScreenCaptureManager>();
        impl->Thread_Data_->WindowCaptureData.getThingsToWatch = windowtocapture;
        return std::make_shared<WindowCaptureConfiguration>(impl);
    }
} // namespace Screen_Capture
} // namespace SL
