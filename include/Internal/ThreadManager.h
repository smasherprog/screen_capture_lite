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
            void Init(const Base_Thread_Data& data, const ScreenCapture_Settings& settings);
            void Join();
            void Reset();
        };
            
        template<class T, class F>DUPL_RETURN RunThread(std::shared_ptr<F> data, T& frameprocessor) {
            while (!*data->TerminateThreadsEvent)
            {
                auto start = std::chrono::high_resolution_clock::now();
                //Process Frame
                auto Ret = frameprocessor.ProcessFrame();
                if (Ret != DUPL_RETURN_SUCCESS)
                {
                    return Ret;
                }
                auto mspassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
                
                std::string msg = "took ";
                msg += std::to_string(mspassed) + "ms for output ";
                //std::cout << msg << std::endl;

                auto timetowait = data->CaptureInterval - mspassed;
                if (timetowait > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(timetowait));
                }
            }
            return DUPL_RETURN_SUCCESS;
        }

        template<class T, class F >bool TryCapture(std::shared_ptr<F> data) {
            T frameprocessor;

            // Make duplication manager
            auto ret = frameprocessor.Init(data);
            if (ret != DUPL_RETURN_SUCCESS) {//Directx duplication is NOT supported!
                return false;
            }
            else {
                ret = RunThread(data, frameprocessor);
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
        }
    

        void RunCapture(std::shared_ptr<Monitor_Thread_Data> data);
        void RunCaptureMouse(std::shared_ptr<Mouse_Thread_Data> data);
    }
}