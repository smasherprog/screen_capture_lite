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

namespace SL {
    namespace Screen_Capture {


        class ScreenCaptureManagerImpl {
        public:

            std::shared_ptr<Thread_Data> Thread_Data_;

            std::thread _Thread;
            std::shared_ptr<std::atomic_bool> TerminateThread_;


            ScreenCaptureManagerImpl() {
                TerminateThread_ = std::make_shared<std::atomic_bool>(false);
                Thread_Data_ = std::make_shared<Thread_Data>();
                Thread_Data_->Monitor_Capture_Interval = std::chrono::milliseconds(100);
                Thread_Data_->Mouse_Capture_Interval = std::chrono::milliseconds(50);
            }
            ~ScreenCaptureManagerImpl() {
                *TerminateThread_ = true;
                if (_Thread.joinable()) {
                    _Thread.join();
                }
            }
            void start() {

                //users must set at least one callback before starting
                assert(Thread_Data_->CaptureEntireMonitor || Thread_Data_->CaptureDifMonitor || Thread_Data_->CaptureMouse);
                _Thread = std::thread([&]() {
                    ThreadManager ThreadMgr;
                    Thread_Data_->ExpectedErrorEvent = std::make_shared<std::atomic_bool>(false);
                    Thread_Data_->UnexpectedErrorEvent = std::make_shared<std::atomic_bool>(false);
                    Thread_Data_->TerminateThreadsEvent = TerminateThread_;

                    ThreadMgr.Init(Thread_Data_);

                    while (!*TerminateThread_) {

                        if (*Thread_Data_->ExpectedErrorEvent)
                        {
                            // std::cout<<"Expected Error, Restarting Thread Manager"<<std::endl;
                             // Terminate other threads
                            *TerminateThread_ = true;
                            ThreadMgr.Join();
                            *Thread_Data_->ExpectedErrorEvent = *Thread_Data_->UnexpectedErrorEvent = *TerminateThread_ = false;
                            // Clean up
                            ThreadMgr.Reset();
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));//sleep for 1 second since an error occcured

                            ThreadMgr.Init(Thread_Data_);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                    *TerminateThread_ = true;
                    ThreadMgr.Join();
                });
            }
        };
        void ScreenCaptureManager::setFrameChangeInterval(std::chrono::milliseconds interval) {
            Impl_->Thread_Data_->Monitor_Capture_Interval = interval;
        }
        void ScreenCaptureManager::setMouseChangeInterval(std::chrono::milliseconds interval) {
            Impl_->Thread_Data_->Mouse_Capture_Interval = interval;
        }

        ScreenCaptureManager ScreenCaptureConfiguration::start_capturing() {
            Impl_->start();
            return ScreenCaptureManager(Impl_);
        }
        ScreenCaptureConfiguration ScreenCaptureConfiguration::onNewFrame(const CaptureCallback& cb) {
            assert(!Impl_->Thread_Data_->CaptureEntireMonitor);
            Impl_->Thread_Data_->CaptureEntireMonitor = cb;
            return ScreenCaptureConfiguration(Impl_);
        }
        ScreenCaptureConfiguration ScreenCaptureConfiguration::onFrameChanged(const CaptureCallback& cb) {
            assert(!Impl_->Thread_Data_->CaptureDifMonitor);
            Impl_->Thread_Data_->CaptureDifMonitor = cb;
            return ScreenCaptureConfiguration(Impl_);
        }
        ScreenCaptureConfiguration ScreenCaptureConfiguration::onMouseChanged(const MouseCallback& cb) {
            assert(!Impl_->Thread_Data_->CaptureMouse);
            Impl_->Thread_Data_->CaptureMouse = cb;
            return ScreenCaptureConfiguration(Impl_);
        }
        ScreenCaptureConfiguration CreateScreeCapture(const MonitorCallback& monitorstocapture) {
            auto  impl = std::make_shared<ScreenCaptureManagerImpl>();
            impl->Thread_Data_->MonitorsChanged = monitorstocapture;
            return ScreenCaptureConfiguration(impl);
        }
    }
}


