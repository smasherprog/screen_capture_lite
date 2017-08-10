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
            for (auto& m : monitors) {
                totalwidth += Width(m);
            }

            // if the monitor doesnt exist any more!
            if (std::find_if(begin(monitors), end(monitors), [&](auto& m) { return m.Id == monitor.Id; }) == end(monitors)) {
                return false;
            } // if the area to capture is outside the dimensions of the desktop!!
            auto& realmonitor = monitors[Index(monitor)];
            if (Height(realmonitor) < Height(monitor) || // monitor height check
                totalwidth < Width(monitor) + OffsetX(monitor) ||               // total width check
                Width(monitor) > Width(realmonitor)) // regular width check

            {
                return false;
            } // if the entire screen is capture and the offsets changed, get out and rebuild
            else if (Height(realmonitor) == Height(monitor) && Width(realmonitor) == Width(monitor) &&
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
                Thread_Data_->CommonData_.Paused = false;
                Thread_Data_->ScreenCaptureData.FrameTimer = std::make_shared<Timer<long long, std::milli> >(100ms);
                Thread_Data_->ScreenCaptureData.MouseTimer = std::make_shared<Timer<long long, std::milli> >(50ms);
                Thread_Data_->WindowCaptureData.FrameTimer = std::make_shared<Timer<long long, std::milli> >(100ms);
                Thread_Data_->WindowCaptureData.MouseTimer = std::make_shared<Timer<long long, std::milli> >(50ms);
            }
            ~ScreenCaptureManagerImpl()
            {
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
                    ThreadManager ThreadMgr;

                    ThreadMgr.Init(Thread_Data_);

                    while (!Thread_Data_->CommonData_.TerminateThreadsEvent) {

                        if (Thread_Data_->CommonData_.ExpectedErrorEvent) {
                            Thread_Data_->CommonData_.TerminateThreadsEvent = true;
                            ThreadMgr.Join();
                            Thread_Data_->CommonData_.ExpectedErrorEvent = Thread_Data_->CommonData_.UnexpectedErrorEvent = Thread_Data_->CommonData_.TerminateThreadsEvent = false;
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
        };

        class ScreenCaptureManager : public IScreenCaptureManager
        {
            std::shared_ptr<ScreenCaptureManagerImpl> Impl_;
        public:
            ScreenCaptureManager(const std::shared_ptr<ScreenCaptureManagerImpl>& impl) : Impl_(impl) {}
            virtual ~ScreenCaptureManager() {}

            void setFrameChangeInterval(const std::shared_ptr<ITimer>& timer) {
                std::atomic_store(&Impl_->Thread_Data_->ScreenCaptureData.FrameTimer, timer);
                std::atomic_store(&Impl_->Thread_Data_->WindowCaptureData.FrameTimer, timer);
            }
            void setMouseChangeInterval(const std::shared_ptr<ITimer>& timer) {
                std::atomic_store(&Impl_->Thread_Data_->ScreenCaptureData.MouseTimer, timer);
                std::atomic_store(&Impl_->Thread_Data_->WindowCaptureData.MouseTimer, timer);
            }
            virtual  void pause() {
                Impl_->Thread_Data_->CommonData_.Paused = true;
            }
            virtual bool isPaused() const {
                return Impl_->Thread_Data_->CommonData_.Paused;
            }
            virtual void resume() {
                Impl_->Thread_Data_->CommonData_.Paused = false;
            }
        };

        class ScreenCaptureConfiguration : public ICaptureConfiguration<ScreenCaptureCallback>
        {
            std::shared_ptr<ScreenCaptureManagerImpl> Impl_;
        public:
            ScreenCaptureConfiguration(const std::shared_ptr<ScreenCaptureManagerImpl>& impl) : Impl_(impl) {}

            virtual std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> onNewFrame(const ScreenCaptureCallback& cb) override {
                assert(!Impl_->Thread_Data_->ScreenCaptureData.OnNewFrame);
                Impl_->Thread_Data_->ScreenCaptureData.OnNewFrame = cb;
                return std::make_shared<ScreenCaptureConfiguration>(Impl_);
            }
            virtual std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> onFrameChanged(const ScreenCaptureCallback& cb) override {
                assert(!Impl_->Thread_Data_->ScreenCaptureData.OnFrameChanged);
                Impl_->Thread_Data_->ScreenCaptureData.OnFrameChanged = cb;
                return std::make_shared<ScreenCaptureConfiguration>(Impl_);
            }
            virtual std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> onMouseChanged(const MouseCallback& cb)override {
                assert(!Impl_->Thread_Data_->ScreenCaptureData.OnMouseChanged);
                Impl_->Thread_Data_->ScreenCaptureData.OnMouseChanged = cb;
                return std::make_shared<ScreenCaptureConfiguration>(Impl_);
            }
            virtual std::shared_ptr<IScreenCaptureManager> start_capturing() override {
                assert(Impl_->Thread_Data_->ScreenCaptureData.OnMouseChanged || Impl_->Thread_Data_->ScreenCaptureData.OnFrameChanged || Impl_->Thread_Data_->ScreenCaptureData.OnNewFrame);
                Impl_->start();
                return std::make_shared<ScreenCaptureManager>(Impl_);
            }
        };

        class WindowCaptureConfiguration : public ICaptureConfiguration<WindowCaptureCallback>
        {
            std::shared_ptr<ScreenCaptureManagerImpl> Impl_;
        public:
            WindowCaptureConfiguration(const std::shared_ptr<ScreenCaptureManagerImpl>& impl) : Impl_(impl) {}

            virtual std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> onNewFrame(const WindowCaptureCallback& cb) override {
                assert(!Impl_->Thread_Data_->WindowCaptureData.OnNewFrame);
                Impl_->Thread_Data_->WindowCaptureData.OnNewFrame = cb;
                return std::make_shared<WindowCaptureConfiguration>(Impl_);
            }
            virtual std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> onFrameChanged(const WindowCaptureCallback& cb) override {
                assert(!Impl_->Thread_Data_->WindowCaptureData.OnFrameChanged);
                Impl_->Thread_Data_->WindowCaptureData.OnFrameChanged = cb;
                return std::make_shared<WindowCaptureConfiguration>(Impl_);
            }
            virtual std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> onMouseChanged(const MouseCallback& cb)override {

                assert(!Impl_->Thread_Data_->WindowCaptureData.OnMouseChanged);
                Impl_->Thread_Data_->WindowCaptureData.OnMouseChanged = cb;
                return std::make_shared<WindowCaptureConfiguration>(Impl_);
            }
            virtual std::shared_ptr<IScreenCaptureManager> start_capturing() override {
                assert(Impl_->Thread_Data_->WindowCaptureData.OnMouseChanged || Impl_->Thread_Data_->WindowCaptureData.OnFrameChanged || Impl_->Thread_Data_->WindowCaptureData.OnNewFrame);
                Impl_->start();
                return std::make_shared<ScreenCaptureManager>(Impl_);
            }
        };
        std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> CreateCaptureConfiguration(const MonitorCallback& monitorstocapture)
        {
            auto impl = std::make_shared<ScreenCaptureManagerImpl>();
            impl->Thread_Data_->ScreenCaptureData.getThingsToWatch = monitorstocapture;
            return std::make_shared<ScreenCaptureConfiguration>(impl);
        }

        std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> CreateCaptureConfiguration(const WindowCallback& windowtocapture)
        {
            auto impl = std::make_shared<ScreenCaptureManagerImpl>();
            impl->Thread_Data_->WindowCaptureData.getThingsToWatch = windowtocapture;
            return std::make_shared<WindowCaptureConfiguration>(impl);
        }
    }
}
