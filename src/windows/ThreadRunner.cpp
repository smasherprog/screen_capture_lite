#include "ScreenCapture.h"
#include "DXFrameProcessor.h"
#include "GDIFrameProcessor.h"
#include "GDIMouseProcessor.h"
#include "ThreadManager.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>
#include <string>
#include <iostream>

namespace SL {
    namespace Screen_Capture {


        template<class T>void ProcessExit(DUPL_RETURN Ret, T* TData) {
            if (Ret != DUPL_RETURN_SUCCESS)
            {
                if (Ret == DUPL_RETURN_ERROR_EXPECTED)
                {
                    // The system is in a transition state so request the duplication be restarted
                    *TData->ExpectedErrorEvent = true;
                }
                else
                {
                    // Unexpected error so exit the application
                    *TData->UnexpectedErrorEvent = true;
                }
            }
        }
        template<class T>bool SwitchToInputDesktop(const std::shared_ptr<T> data) {
            HDESK CurrentDesktop = nullptr;
            CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
            if (!CurrentDesktop)
            {
                // We do not have access to the desktop so request a retry
                *data->ExpectedErrorEvent = true;
                ProcessExit(DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED, data.get());
                return false;
            }

            // Attach desktop to this thread
            bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
            CloseDesktop(CurrentDesktop);
            CurrentDesktop = nullptr;
            if (!DesktopAttached)
            {
                // We do not have access to the desktop so request a retry
                *data->ExpectedErrorEvent = true;
                ProcessExit(DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED, data.get());
                return false;
            }
            return true;
        }
        void RunCaptureMouse(std::shared_ptr<Thread_Data> data) {
            if (!SwitchToInputDesktop(data)) return;
            TryCaptureMouse<GDIMouseProcessor>(data);
        } 
        void RunCaptureMonitor(std::shared_ptr<Thread_Data> data, Monitor monitor) {
            //need to switch to the input desktop for capturing...
            if (!SwitchToInputDesktop(data)) return;
            std::cout << "Starting to Capture on Monitor " << Name(monitor) << std::endl;
            std::cout << "Trying DirectX Desktop Duplication " << std::endl;
            //TryCapture<GDIFrameProcessor>(data);
            
            if (!TryCaptureMonitor<DXFrameProcessor>(data, monitor)) {//if DX is not supported, fallback to GDI capture
                std::cout << "DirectX Desktop Duplication not supprted, falling back to GDI Capturing . . ." << std::endl;
                TryCaptureMonitor<GDIFrameProcessor>(data, monitor);
            }
        }
    }
}



