#pragma once
#include "ScreenCapture.h"
#include "SCCommon.h"
#include <thread>
#include <vector>
#include <atomic>
#include <string>
#include <iostream>

using namespace std::chrono_literals;

// this is internal stuff..
namespace SL
{
    namespace Screen_Capture
    {
        class ThreadManager
        {

            std::vector<std::thread> m_ThreadHandles;
            std::shared_ptr<std::atomic_bool> TerminateThreadsEvent;
        public:
            ThreadManager();
            ~ThreadManager();
            void Init(const std::shared_ptr<Thread_Data>& settings);
            void Join();
        };
        template <class T, class F> bool TryCaptureMouse(const F& data)
        {
            T frameprocessor;
            auto ret = frameprocessor.Init(data);
            if (ret != DUPL_RETURN_SUCCESS) {
                return false;
            }
            frameprocessor.ImageBufferSize = frameprocessor.MaxCursurorSize * frameprocessor.MaxCursurorSize * PixelStride;

            frameprocessor.OldImageBuffer = std::make_unique<unsigned char[]>(frameprocessor.ImageBufferSize);
            frameprocessor.NewImageBuffer = std::make_unique<unsigned char[]>(frameprocessor.ImageBufferSize);

            while (!data->TerminateThreadsEvent) {
                // get a copy of the shared_ptr in a safe way
                auto timer = std::atomic_load(&data->Mouse_Capture_Timer);
                timer->start();
                // Process Frame
                ret = frameprocessor.ProcessFrame();
                if (ret != DUPL_RETURN_SUCCESS) {
                    if (ret == DUPL_RETURN_ERROR_EXPECTED) {
                        // The system is in a transition state so request the duplication be restarted
                        data->ExpectedErrorEvent = true;
                        std::cout << "Exiting Thread due to expected error " << std::endl;
                    }
                    else {
                        // Unexpected error so exit the application
                        data->UnexpectedErrorEvent = true;
                        std::cout << "Exiting Thread due to Unexpected error " << std::endl;
                    }
                    return true;
                }
                timer->wait();
                while (data->Paused) {
                    std::this_thread::sleep_for(50ms);
                }
            }
            return true;
        }

        inline bool MonitorsChanged(const std::vector<Monitor>& startmonitors, const std::vector<Monitor>& nowmonitors) {
            if (startmonitors.size() != nowmonitors.size()) return true;
            for (size_t i = 0; i < startmonitors.size(); i++) {
                if (startmonitors[i].Height != nowmonitors[i].Height ||
                    startmonitors[i].Id != nowmonitors[i].Id ||
                    startmonitors[i].Index != nowmonitors[i].Index || 
                    startmonitors[i].OffsetX != nowmonitors[i].OffsetX ||
                    startmonitors[i].OffsetY != nowmonitors[i].OffsetY ||
                    startmonitors[i].Width != nowmonitors[i].Width) return true;
            }
            return false;
        }
        template <class T, class F> bool TryCaptureMonitor(const F& data, Monitor& monitor)
        {
            T frameprocessor;
            auto startmonitors = GetMonitors();
            auto ret = frameprocessor.Init(data, monitor);
            if (ret != DUPL_RETURN_SUCCESS) {
                return false;
            }
            frameprocessor.ImageBufferSize = Width(monitor) * Height(monitor) * PixelStride;
            if (data->CaptureDifMonitor) { // only need the old buffer if difs are needed. If no dif is needed, then the
                                          // image is always new
                frameprocessor.OldImageBuffer = std::make_unique<unsigned char[]>(frameprocessor.ImageBufferSize);
                frameprocessor.NewImageBuffer = std::make_unique<unsigned char[]>(frameprocessor.ImageBufferSize);
            }
            if ((data->CaptureEntireMonitor) && !frameprocessor.NewImageBuffer) {

                frameprocessor.NewImageBuffer = std::make_unique<unsigned char[]>(frameprocessor.ImageBufferSize);
            }
            while (!data->TerminateThreadsEvent) {
                // get a copy of the shared_ptr in a safe way
                auto timer = std::atomic_load(&data->Monitor_Capture_Timer);
                timer->start();
                auto monitors = GetMonitors();
                if (isMonitorInsideBounds(monitors, monitor) && !MonitorsChanged(startmonitors, monitors)) {
                    ret = frameprocessor.ProcessFrame(monitors[Index(monitor)]);
                }
                else {
                    // something happened, rebuild
                    ret = DUPL_RETURN_ERROR_EXPECTED;
                }

                if (ret != DUPL_RETURN_SUCCESS) {
                    if (ret == DUPL_RETURN_ERROR_EXPECTED) {
                        // The system is in a transition state so request the duplication be restarted
                        data->ExpectedErrorEvent = true;
                        std::cout << "Exiting Thread due to expected error " << std::endl;
                    }
                    else {
                        // Unexpected error so exit the application
                        data->UnexpectedErrorEvent = true;
                        std::cout << "Exiting Thread due to Unexpected error " << std::endl;
                    }
                    return true;
                }
                timer->wait();
                while (data->Paused) {
                    std::this_thread::sleep_for(50ms);
                }
            }
            return true;
        }

        void RunCaptureMonitor(std::shared_ptr<Thread_Data> data, Monitor monitor);
        void RunCaptureMouse(std::shared_ptr<Thread_Data> data);
    }
}
