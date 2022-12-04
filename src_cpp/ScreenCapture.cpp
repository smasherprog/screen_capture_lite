#include "ScreenCapture.h"
#include "ScreenCapture_C_API.h"
#include "internal/SCCommon.h"
#include "internal/ThreadManager.h"
#include <algorithm>
#include <assert.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <thread>
#include <type_traits>

namespace SL::Screen_Capture {

bool IsMonitorInsideBounds(const Monitor *monitors, const int monitorsize, const Monitor &monitor)
{

    auto totalwidth = 0;

    for (int i = 0; i < monitorsize; i++) {
        totalwidth += Width(monitors[i]);
    }

    // if the monitor doesnt exist any more!
    int index = 0;
    while(index < monitorsize && monitors[index].Id != monitor.Id)
        index++;
    if (index == monitorsize) {
        return false;
    } // if the area to capture is outside the dimensions of the desktop!!

    auto &realmonitor = monitors[Index(monitor)];

    if (Height(realmonitor) < Height(monitor) ||          // monitor height check
        totalwidth < Width(monitor) + OffsetX(monitor) || // total width check
        Width(monitor) > Width(realmonitor))              // regular width check
    {
        return false;
    }
    else if (Height(realmonitor) == Height(monitor) && Width(realmonitor) == Width(monitor) &&
             (OffsetX(realmonitor) != OffsetX(monitor) || OffsetY(realmonitor) != OffsetY(monitor))) {
        // if the entire screen is capture and the offsets changed, get out and rebuild
        return false;
    }

    return true;
}

bool isMonitorInsideBounds(const std::vector<Monitor> &monitors, const Monitor &monitor) { return IsMonitorInsideBounds(monitors.data(), (int)monitors.size(), monitor); }

namespace C_API {

    struct ICaptureConfigurationScreenCaptureCallbackWrapper {
        void *const context;
        std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> ptr;
        explicit ICaptureConfigurationScreenCaptureCallbackWrapper(void *context = nullptr) : context(context), ptr() {}
    };

    struct ICaptureConfigurationWindowCaptureCallbackWrapper {
        void *const context;
        std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> ptr;
        explicit ICaptureConfigurationWindowCaptureCallbackWrapper(void *context = nullptr) : context(context), ptr() {}
    };

    struct IScreenCaptureManagerWrapper {
        std::shared_ptr<IScreenCaptureManager> ptr;
    };

}; // namespace C_API

class ScreenCaptureManager : public IScreenCaptureManager {

  public:
    // allreads share the same data!!!
    std::shared_ptr<Thread_Data> Thread_Data_;

    std::thread Thread_;
    bool ShuttingDown = false;
    ScreenCaptureManager()
    {
        Thread_Data_ = std::make_shared<Thread_Data>();
        Thread_Data_->CommonData_.Paused = false;
        Thread_Data_->ScreenCaptureData.FrameTimer = std::make_shared<Timer>(100ms);
        Thread_Data_->ScreenCaptureData.MouseTimer = std::make_shared<Timer>(50ms);
        Thread_Data_->WindowCaptureData.FrameTimer = std::make_shared<Timer>(100ms);
        Thread_Data_->WindowCaptureData.MouseTimer = std::make_shared<Timer>(50ms);
    }

    virtual ~ScreenCaptureManager()
    {
        ShuttingDown = true;
        Thread_Data_->CommonData_.TerminateThreadsEvent = true; // set the exit flag for the threads
        Thread_Data_->CommonData_.Paused = false;               // unpaused the threads to let everything exit
        if (Thread_.get_id() == std::this_thread::get_id()) {
            Thread_.detach();
        }
        else if (Thread_.joinable()) {
            Thread_.join();
        }
    }

    void start()
    {
        Thread_ = std::thread([&]() {
            if(ShuttingDown) return;
            ThreadManager ThreadMgr;
            ThreadMgr.Init(Thread_Data_);

            while (!Thread_Data_->CommonData_.TerminateThreadsEvent && !ShuttingDown) {
                if (Thread_Data_->CommonData_.ExpectedErrorEvent) {
                    Thread_Data_->CommonData_.TerminateThreadsEvent = true;
                    ThreadMgr.Join();
                    Thread_Data_->CommonData_.ExpectedErrorEvent = Thread_Data_->CommonData_.UnexpectedErrorEvent =
                        Thread_Data_->CommonData_.TerminateThreadsEvent = false;
                    
                    // Clean up and rebuild
                    if(!ShuttingDown){
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // sleep for 1 second since an error occcured
                        ThreadMgr.Init(Thread_Data_);
                    } else { 
                        Thread_Data_->CommonData_.TerminateThreadsEvent = true;
                    }
                }  
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            Thread_Data_->CommonData_.TerminateThreadsEvent = true;
            ThreadMgr.Join();
        });
    }

    virtual void setFrameChangeInterval(const std::shared_ptr<Timer> &timer) override
    {
#if defined(_WIN32) && defined(__cplusplus) && __cplusplus >= 202002L && !defined(__MINGW32__)
            Thread_Data_->ScreenCaptureData.FrameTimer.store(timer);
            Thread_Data_->WindowCaptureData.FrameTimer.store(timer);
#else
            std::atomic_store(&Thread_Data_->ScreenCaptureData.FrameTimer, timer);
            std::atomic_store(&Thread_Data_->WindowCaptureData.FrameTimer, timer);
#endif  
    }

    virtual void setMouseChangeInterval(const std::shared_ptr<Timer> &timer) override
    {
#if defined(_WIN32) && defined(__cplusplus) && __cplusplus >= 202002L && !defined(__MINGW32__)
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

}; // namespace SL::Screen_Capture

int SCL_GetMonitors(SCL_MonitorRef monitors, int monitors_size)
{
    auto local_monitors = SL::Screen_Capture::GetMonitors();
    auto maxelements = std::clamp(static_cast<int>(local_monitors.size()), 0, monitors_size);
    memcpy(monitors, local_monitors.data(), maxelements * sizeof(SL::Screen_Capture::Monitor));
    return static_cast<int>(local_monitors.size());
}

int SCL_GetWindows(SCL_WindowRef windows, int monitors_size)
{
    auto local_windows = SL::Screen_Capture::GetWindows();
    auto maxelements = std::clamp(static_cast<int>(local_windows.size()), 0, monitors_size);
    memcpy(windows, local_windows.data(), maxelements * sizeof(SL::Screen_Capture::Window));
    return static_cast<int>(local_windows.size());
}

int SCL_IsMonitorInsideBounds(SCL_MonitorRefConst monitors, const int monitorsize, SCL_MonitorRefConst monitor)
{
    return int(IsMonitorInsideBounds(monitors, monitorsize, *monitor));
}

void SCL_MonitorOnNewFrame(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_ScreenCaptureCallback cb)
{
    ptr->ptr = ptr->ptr->onNewFrame([=](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Monitor &monitor) { cb(&img, &monitor); });
}

void SCL_MonitorOnNewFrameWithContext(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_ScreenCaptureCallbackWithContext cb)
{
    ptr->ptr = ptr->ptr->onNewFrame(
        [=](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Monitor &monitor) { cb(&img, &monitor, ptr->context); });
}

void SCL_MonitorOnFrameChanged(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_ScreenCaptureCallback cb)
{
    ptr->ptr =
        ptr->ptr->onFrameChanged([=](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Monitor &monitor) { cb(&img, &monitor); });
}

void SCL_MonitorOnFrameChangedWithContext(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_ScreenCaptureCallbackWithContext cb)
{
    ptr->ptr = ptr->ptr->onFrameChanged(
        [=](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Monitor &monitor) { cb(&img, &monitor, ptr->context); });
}

void SCL_MonitorOnMouseChanged(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_MouseCaptureCallback cb)
{
    ptr->ptr = ptr->ptr->onMouseChanged(
        [=](const SL::Screen_Capture::Image *img, const SL::Screen_Capture::MousePoint &mousepoint) { cb(img, &mousepoint); });
}

void SCL_MonitorOnMouseChangedWithContext(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr, SCL_MouseCaptureCallbackWithContext cb)
{
    ptr->ptr = ptr->ptr->onMouseChanged(
        [=](const SL::Screen_Capture::Image *img, const SL::Screen_Capture::MousePoint &mousepoint) { cb(img, &mousepoint, ptr->context); });
}

SCL_IScreenCaptureManagerWrapperRef SCL_MonitorStartCapturing(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr)
{
    auto p = new SL::Screen_Capture::C_API::IScreenCaptureManagerWrapper{ptr->ptr->start_capturing()};
    return p;
}

void SCL_SetFrameChangeInterval(SCL_IScreenCaptureManagerWrapperRef ptr, int milliseconds)
{
    ptr->ptr->setFrameChangeInterval(std::chrono::milliseconds(milliseconds));
}

void SCL_SetMouseChangeInterval(SCL_IScreenCaptureManagerWrapperRef ptr, int milliseconds)
{
    ptr->ptr->setMouseChangeInterval(std::chrono::milliseconds(milliseconds));
}

void SCL_PauseCapturing(SCL_IScreenCaptureManagerWrapperRef ptr) { ptr->ptr->pause(); }

int SCL_IsPaused(SCL_IScreenCaptureManagerWrapperRef ptr) { return int(ptr->ptr->isPaused()); }

void SCL_Resume(SCL_IScreenCaptureManagerWrapperRef ptr) { ptr->ptr->resume(); }

void SCL_FreeIScreenCaptureManagerWrapper(SCL_IScreenCaptureManagerWrapperRef ptr) { delete ptr; }

void SCL_WindowOnNewFrame(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_WindowCaptureCallback cb)
{
    ptr->ptr = ptr->ptr->onNewFrame([=](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Window &window) { cb(&img, &window); });
}

void SCL_WindowOnNewFrameWithContext(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_WindowCaptureCallbackWithContext cb)
{
    ptr->ptr = ptr->ptr->onNewFrame(
        [=](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Window &window) { cb(&img, &window, ptr->context); });
}

void SCL_WindowOnFrameChanged(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_WindowCaptureCallback cb)
{
    ptr->ptr = ptr->ptr->onFrameChanged([=](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Window &window) { cb(&img, &window); });
}

void SCL_WindowOnFrameChangedWithContext(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_WindowCaptureCallbackWithContext cb)
{
    ptr->ptr = ptr->ptr->onFrameChanged(
        [=](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Window &window) { cb(&img, &window, ptr->context); });
}

void SCL_WindowOnMouseChanged(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_MouseCaptureCallback cb)
{
    ptr->ptr = ptr->ptr->onMouseChanged(
        [=](const SL::Screen_Capture::Image *img, const SL::Screen_Capture::MousePoint &mousepoint) { cb(img, &mousepoint); });
}

void SCL_WindowOnMouseChangedWithContext(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr, SCL_MouseCaptureCallbackWithContext cb)
{
    ptr->ptr = ptr->ptr->onMouseChanged(
        [=](const SL::Screen_Capture::Image *img, const SL::Screen_Capture::MousePoint &mousepoint) { cb(img, &mousepoint, ptr->context); });
}

SCL_IScreenCaptureManagerWrapperRef SCL_WindowStartCapturing(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr)
{
    auto p = new SL::Screen_Capture::C_API::IScreenCaptureManagerWrapper{ptr->ptr->start_capturing()};
    return p;
}

unsigned char *SCL_Utility_CopyToContiguous(unsigned char *dst, SCL_ImageRefConst image)
{

    constexpr auto PIXEL_DEPTH = sizeof(SL::Screen_Capture::ImageBGRA);
    static_assert(PIXEL_DEPTH == 4);

    auto tmp(dst);

    if (image->isContiguous) {
        auto size(Width(image->Bounds) * Height(image->Bounds) * PIXEL_DEPTH);
        std::memcpy(tmp, image->Data, size);
        std::advance(tmp, size);
        return tmp;
    }
    else {

        auto const cols(SL::Screen_Capture::Width(image->Bounds));
        auto const rows(SL::Screen_Capture::Height(image->Bounds));
        auto const stride = image->RowStrideInBytes / PIXEL_DEPTH;

        for (int row = 0; row < rows; ++row) {
            auto count = cols * PIXEL_DEPTH;
            auto start = image->Data + row * stride;
            std::memcpy(tmp, start, count);
            std::advance(tmp, count);
        }

        return tmp;
    }
}

SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef SCL_CreateMonitorCaptureConfiguration(SCL_MonitorCallback monitorstocapture)
{

    static auto maxbuffersize = 16;
    auto p = new SL::Screen_Capture::C_API::ICaptureConfigurationScreenCaptureCallbackWrapper();

    p->ptr = SL::Screen_Capture::CreateCaptureConfiguration([=]() {
        std::vector<SL::Screen_Capture::Monitor> buffer;

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

SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef
SCL_CreateMonitorCaptureConfigurationWithContext(SCL_MonitorCallbackWithContext monitorstocapture, void *context)
{

    auto p = new SL::Screen_Capture::C_API::ICaptureConfigurationScreenCaptureCallbackWrapper(context);
    static auto maxbuffersize = 16;

    p->ptr = SL::Screen_Capture::CreateCaptureConfiguration([=]() {
        std::vector<SL::Screen_Capture::Monitor> buffer;

        auto sizeguess = maxbuffersize;
        buffer.resize(sizeguess);
        auto sizeneeded = monitorstocapture(buffer.data(), sizeguess, context);
        if (sizeguess < sizeneeded) {
            sizeguess = sizeneeded;
            maxbuffersize = std::max(sizeneeded, maxbuffersize);
            buffer.resize(sizeguess);
            sizeneeded = monitorstocapture(buffer.data(), sizeguess, context);
            sizeguess = std::min(sizeguess, sizeneeded);
            buffer.resize(sizeguess);
            return buffer;
        }
        buffer.resize(sizeneeded);
        return buffer;
    });
    return p;
}

SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef SCL_CreateWindowCaptureConfiguration(SCL_WindowCallback windowstocapture)
{

    static auto maxbuffersize = 16;
    auto p = new SL::Screen_Capture::C_API::ICaptureConfigurationWindowCaptureCallbackWrapper();

    p->ptr = SL::Screen_Capture::CreateCaptureConfiguration([=]() {
        std::vector<SL::Screen_Capture::Window> buffer;
        auto sizeguess = maxbuffersize;
        buffer.resize(sizeguess);
        auto sizeneeded = windowstocapture(buffer.data(), sizeguess);
        if (sizeguess < sizeneeded) {
            sizeguess = sizeneeded;
            maxbuffersize = std::max(sizeneeded, maxbuffersize);
            buffer.resize(sizeguess);
            sizeneeded = windowstocapture(buffer.data(), sizeguess);
            sizeguess = std::min(sizeguess, sizeneeded);
            buffer.resize(sizeguess);
            return buffer;
        }
        buffer.resize(sizeneeded);
        return buffer;
    });

    return p;
}

SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef
SCL_CreateWindowCaptureConfigurationWithContext(SCL_WindowCallbackWithContext windowstocapture, void *context)
{
    static auto maxbuffersize = 16;
    auto p = new SL::Screen_Capture::C_API::ICaptureConfigurationWindowCaptureCallbackWrapper(context);
    p->ptr = SL::Screen_Capture::CreateCaptureConfiguration([=]() {
        std::vector<SL::Screen_Capture::Window> buffer;
        auto sizeguess = maxbuffersize;
        buffer.resize(sizeguess);
        auto sizeneeded = windowstocapture(buffer.data(), sizeguess, context);
        if (sizeguess < sizeneeded) {
            sizeguess = sizeneeded;
            maxbuffersize = std::max(sizeneeded, maxbuffersize);
            buffer.resize(sizeguess);
            sizeneeded = windowstocapture(buffer.data(), sizeguess, context);
            sizeguess = std::min(sizeguess, sizeneeded);
            buffer.resize(sizeguess);
            return buffer;
        }
        buffer.resize(sizeneeded);
        return buffer;
    });
    return p;
}

void SCL_FreeWindowCaptureConfiguration(SCL_ICaptureConfigurationWindowCaptureCallbackWrapperRef ptr) { delete ptr; }

void SCL_FreeMonitorCaptureConfiguration(SCL_ICaptureConfigurationScreenCaptureCallbackWrapperRef ptr) { delete ptr; }
