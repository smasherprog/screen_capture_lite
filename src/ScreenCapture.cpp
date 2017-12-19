#include "SCCommon.h"
#include "ScreenCapture.h"
#include "ThreadManager.h"
#include <algorithm>
#include <assert.h>
#include <atomic>
#include <cstring>
#include <iostream>
#include <memory>
#include <thread>

namespace SL {
namespace Screen_Capture {

    void Extract(const Image &img, unsigned char *dst, size_t dst_size)
    {
        assert(dst_size >= static_cast<size_t>(RowStride(img) * Height(img)));
        auto startdst = dst;
        auto startsrc = StartSrc(img);
        if (RowPadding(img) == 0) { // no padding, the entire copy can be a single memcpy call
            memcpy(startdst, startsrc, RowStride(img) * Height(img));
        }
        else {
            for (auto i = 0; i < Height(img); i++) {
                memcpy(startdst, startsrc, RowStride(img));
                startdst += RowStride(img);                   // advance to the next row
                startsrc += RowStride(img) + RowPadding(img); // advance to the next row
            }
        }
    }

    void ExtractAndConvertToRGBA(const Image &img, unsigned char *dst, size_t dst_size)
    {

        assert(dst_size >= static_cast<size_t>(RowStride(img) * Height(img)));
        auto imgsrc = StartSrc(img);
        auto imgdist = dst;
        for (auto h = 0; h < Height(img); h++) {
            for (auto w = 0; w < Width(img); w++) {
                *imgdist++ = *(imgsrc + 2);
                *imgdist++ = *(imgsrc + 1);
                *imgdist++ = *(imgsrc);
                *imgdist++ = 0; // alpha should be zero
                imgsrc += img.Pixelstride;
            }
            imgsrc += RowPadding(img);
        }
    }
    void ExtractAndConvertToRGB(const Image &img, unsigned char *dst, size_t dst_size)
    {
        assert(dst_size >= static_cast<size_t>(Width(img) * 3 * Height(img)));
        auto imgsrc = StartSrc(img);
        auto imgdist = dst;
        for (auto h = 0; h < Height(img); h++) {
            for (auto w = 0; w < Width(img); w++) {
                *imgdist++ = *(imgsrc + 2);
                *imgdist++ = *(imgsrc + 1);
                *imgdist++ = *(imgsrc);
                imgsrc += img.Pixelstride;
            }
            imgsrc += RowPadding(img);
        }
    }

    void ExtractAndConvertToRGB565(const Image &img, unsigned char *dst, size_t dst_size)
    {
        assert(dst_size >= static_cast<size_t>(Width(img) * 2 * Height(img)));
        auto imgsrc = StartSrc(img);
        auto imgdist = dst;
        for (auto h = 0; h < Height(img); h++) {
            for (auto w = 0; w < Width(img); w++) {
                unsigned char r = (*(imgsrc + 2)) & 0xF8;
                unsigned char g = (*(imgsrc + 1)) & 0xFC;
                unsigned char b = (*imgsrc) & 0xF8;
                int short rgb = (r << 8) | (g << 3) | (b >> 3);
                *imgdist++ = static_cast<unsigned char>(rgb);
                *imgdist++ = static_cast<unsigned char>(rgb >> 8);
                imgsrc += img.Pixelstride;
            }
            imgsrc += RowPadding(img);
        }
    }

    bool isMonitorInsideBounds(const std::vector<Monitor> &monitors, const Monitor &monitor)
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
    static bool ScreenCaptureManagerExists = false;
    class ScreenCaptureManager : public IScreenCaptureManager {
        

      public:
        // allreads share the same data!!!
        std::shared_ptr<Thread_Data> Thread_Data_;

        std::thread Thread_;

        ScreenCaptureManager()
        {
            //you must ONLY HAVE ONE INSTANCE RUNNING AT A TIME. Destroy the first instance then create one!
            assert(!ScreenCaptureManagerExists);
            ScreenCaptureManagerExists = true;
            Thread_Data_ = std::make_shared<Thread_Data>();
            Thread_Data_->CommonData_.Paused = false;
            Thread_Data_->ScreenCaptureData.FrameTimer = std::make_shared<Timer<long long, std::milli>>(100ms);
            Thread_Data_->ScreenCaptureData.MouseTimer = std::make_shared<Timer<long long, std::milli>>(50ms);
            Thread_Data_->WindowCaptureData.FrameTimer = std::make_shared<Timer<long long, std::milli>>(100ms);
            Thread_Data_->WindowCaptureData.MouseTimer = std::make_shared<Timer<long long, std::milli>>(50ms);
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
        virtual void setFrameChangeInterval(const std::shared_ptr<ITimer> &timer) override
        {
            std::atomic_store(&Thread_Data_->ScreenCaptureData.FrameTimer, timer);
            std::atomic_store(&Thread_Data_->WindowCaptureData.FrameTimer, timer);
        }
        virtual void setMouseChangeInterval(const std::shared_ptr<ITimer> &timer) override
        {
            std::atomic_store(&Thread_Data_->ScreenCaptureData.MouseTimer, timer);
            std::atomic_store(&Thread_Data_->WindowCaptureData.MouseTimer, timer);
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
            assert(!Impl_->Thread_Data_->ScreenCaptureData.OnNewFrame);
            Impl_->Thread_Data_->ScreenCaptureData.OnNewFrame = cb;
            return std::make_shared<ScreenCaptureConfiguration>(Impl_);
        }
        virtual std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> onFrameChanged(const ScreenCaptureCallback &cb) override
        {
            assert(!Impl_->Thread_Data_->ScreenCaptureData.OnFrameChanged);
            Impl_->Thread_Data_->ScreenCaptureData.OnFrameChanged = cb;
            return std::make_shared<ScreenCaptureConfiguration>(Impl_);
        }
        virtual std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> onMouseChanged(const MouseCallback &cb) override
        {
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
