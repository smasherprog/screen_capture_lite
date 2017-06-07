#pragma once
#include "ScreenCapture.h"
#include "SCCommon.h"
#include <thread>
#include <vector>
#include <atomic>
#include <string>
#include <iostream>


// this is internal stuff.. 
namespace SL {
    namespace Screen_Capture {
        class ThreadManager {

            std::vector<std::thread> m_ThreadHandles;
            std::shared_ptr<std::atomic_bool> TerminateThreadsEvent;

        public:
            ThreadManager();
            ~ThreadManager();
            void Init(const std::shared_ptr<Thread_Data>& settings);
            void Join();
            void Reset();
        };

        template<class T, typename CAPTUREINTERVALTYPE>DUPL_RETURN RunThread(const std::shared_ptr<std::atomic_bool>& termintethreads, CAPTUREINTERVALTYPE captureinterval, T& frameprocessor) {
            while (!*termintethreads)
            {
                auto start = std::chrono::high_resolution_clock::now();
                //Process Frame
                auto Ret = frameprocessor.ProcessFrame();
                if (Ret != DUPL_RETURN_SUCCESS)
                {
                    return Ret;
                }
                auto mspassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);

                std::string msg = "took ";
                msg += std::to_string(mspassed.count()) + "ms for output ";
                //std::cout << msg << std::endl;

                auto timetowait = captureinterval - mspassed;
                if (timetowait.count() > 0) {
                    std::this_thread::sleep_for(timetowait);
                }
            }
            return DUPL_RETURN_SUCCESS;
        }

        template<class T, class F>bool TryCaptureMouse(const F& data) {
            T frameprocessor;
            auto ret = frameprocessor.Init(data);
            if (ret != DUPL_RETURN_SUCCESS) {
                return false;
            }
            frameprocessor.ImageBufferSize = frameprocessor.MaxCursurorSize*frameprocessor.MaxCursurorSize* PixelStride;

            frameprocessor.OldImageBuffer = std::make_unique<char[]>(frameprocessor.ImageBufferSize);
            frameprocessor.NewImageBuffer = std::make_unique<char[]>(frameprocessor.ImageBufferSize);

            ret = RunThread(data->TerminateThreadsEvent, data->Mouse_Capture_Interval, frameprocessor);
            if (ret != DUPL_RETURN_SUCCESS)
            {
                if (ret == DUPL_RETURN_ERROR_EXPECTED)
                {
                    // The system is in a transition state so request the duplication be restarted
                    *data->ExpectedErrorEvent = true;
                    std::cout << "Exiting Thread due to expected error " << std::endl;
                }
                else
                {
                    // Unexpected error so exit the application
                    *data->UnexpectedErrorEvent = true;
                    std::cout << "Exiting Thread due to Unexpected error " << std::endl;
                }
            }

            return true;
        }
        template<class T, class F>bool TryCaptureMonitor(const F& data, Monitor& monitor) {
            T frameprocessor;
            auto ret = frameprocessor.Init(data, monitor);
            if (ret != DUPL_RETURN_SUCCESS) {
                return false;
            }
            frameprocessor.ImageBufferSize = Width(monitor)* Height(monitor)* PixelStride;
            if (data->CaptureDifMonitor) {//only need the old buffer if difs are needed. If no dif is needed, then the image is always new
                frameprocessor.OldImageBuffer = std::make_unique<char[]>(frameprocessor.ImageBufferSize);
                frameprocessor.NewImageBuffer = std::make_unique<char[]>(frameprocessor.ImageBufferSize);
            }
            if ((data->CaptureEntireMonitor) && !frameprocessor.NewImageBuffer) {
                frameprocessor.NewImageBuffer = std::make_unique<char[]>(frameprocessor.ImageBufferSize);
            }
            ret = RunThread(data->TerminateThreadsEvent, data->Monitor_Capture_Interval, frameprocessor);
            if (ret != DUPL_RETURN_SUCCESS)
            {
                if (ret == DUPL_RETURN_ERROR_EXPECTED)
                {
                    // The system is in a transition state so request the duplication be restarted
                    *data->ExpectedErrorEvent = true;
                    std::cout << "Exiting Thread due to expected error " << std::endl;
                }
                else
                {
                    // Unexpected error so exit the application
                    *data->UnexpectedErrorEvent = true;
                    std::cout << "Exiting Thread due to Unexpected error " << std::endl;
                }
            }

            return true;
        }

        void RunCaptureMonitor(std::shared_ptr<Thread_Data> data, Monitor monitor);
        void RunCaptureMouse(std::shared_ptr<Thread_Data> data);
    }
}