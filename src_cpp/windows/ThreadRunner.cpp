#include "DXFrameProcessor.h"
#include "GDIFrameProcessor.h"
#include "GDIMouseProcessor.h"
#include "ScreenCapture.h"
#include "internal/ThreadManager.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace SL {
namespace Screen_Capture {

    template <class T> void ProcessExit(DUPL_RETURN Ret, T *TData)
    {
        if (Ret != DUPL_RETURN_SUCCESS) {
            if (Ret == DUPL_RETURN_ERROR_EXPECTED) {
                // The system is in a transition state so request the duplication be restarted
                TData->CommonData_.ExpectedErrorEvent = true;
            }
            else {
                // Unexpected error so exit the application
                TData->CommonData_.UnexpectedErrorEvent = true;
            }
        }
    }

    void RequestScreenCapture() {}
    bool CanRequestScreenCapture() { return false; }

    bool IsScreenCaptureEnabled()
    {
        HDESK CurrentDesktop = nullptr;
        CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
        if (!CurrentDesktop) {
            // We do not have access to the desktop so request a retry
            return false;
        }

        // Attach desktop to this thread
        bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
        CloseDesktop(CurrentDesktop);
        CurrentDesktop = nullptr;
        if (!DesktopAttached) {
            // We do not have access to the desktop so request a retry
            return false;
        }
        return true;
    }
    template <class T> bool SwitchToInputDesktop(const std::shared_ptr<T> data)
    {
        HDESK CurrentDesktop = nullptr;
        CurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
        if (!CurrentDesktop) {
            // We do not have access to the desktop so request a retry
            data->CommonData_.ExpectedErrorEvent = true;
            ProcessExit(DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED, data.get());
            return false;
        }

        // Attach desktop to this thread
        bool DesktopAttached = SetThreadDesktop(CurrentDesktop) != 0;
        CloseDesktop(CurrentDesktop);
        CurrentDesktop = nullptr;
        if (!DesktopAttached) {
            // We do not have access to the desktop so request a retry
            data->CommonData_.ExpectedErrorEvent = true;
            ProcessExit(DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED, data.get());
            return false;
        }
        return true;
    }
    void RunCaptureMouse(std::shared_ptr<Thread_Data> data)
    {
        if (!SwitchToInputDesktop(data))
            return;
        TryCaptureMouse<GDIMouseProcessor>(data);
    }
    void RunCaptureMonitor(std::shared_ptr<Thread_Data> data, Monitor monitor)
    {
        // need to switch to the input desktop for capturing...
        if (!SwitchToInputDesktop(data))
            return;

        enum class CaptureBackend { DXGI, GDI };
        using MonitorKey = std::pair<int, int>;

        const MonitorKey monitor_key(Adapter(monitor), Id(monitor));
        static std::mutex preferred_backend_mutex;
        static std::map<MonitorKey, CaptureBackend> preferred_backends;

        auto preferred_backend = CaptureBackend::DXGI;
        {
            std::lock_guard<std::mutex> lock(preferred_backend_mutex);
            const auto found = preferred_backends.find(monitor_key);
            if (found != preferred_backends.end()) {
                preferred_backend = found->second;
            }
        }
#if defined _DEBUG || !defined NDEBUG
        std::cout << "Starting to Capture on Monitor " << Name(monitor) << std::endl;
        if (preferred_backend == CaptureBackend::GDI) {
            std::cout << "Trying GDI Capturing first " << std::endl;
        }
        else {
            std::cout << "Trying DirectX Desktop Duplication first " << std::endl;
        }
#endif
        if (preferred_backend == CaptureBackend::GDI) {
            if (!TryCaptureMonitor<GDIFrameProcessor>(data, monitor)) {
#if defined _DEBUG || !defined NDEBUG
                std::cout << "GDI capture not supported, falling back to DirectX Desktop Duplication . . ." << std::endl;
#endif
                {
                    std::lock_guard<std::mutex> lock(preferred_backend_mutex);
                    preferred_backends[monitor_key] = CaptureBackend::DXGI;
                }
                if (!TryCaptureMonitor<DXFrameProcessor>(data, monitor)) {
                    std::lock_guard<std::mutex> lock(preferred_backend_mutex);
                    preferred_backends[monitor_key] = CaptureBackend::GDI;
                }
            }
        }
        else {
            const auto capture_started = TryCaptureMonitor<DXFrameProcessor>(data, monitor);
            if (!capture_started) { // if DX is not supported, fallback to GDI capture
#if defined _DEBUG || !defined NDEBUG
                std::cout << "DirectX Desktop Duplication not supported, falling back to GDI Capturing . . ." << std::endl;
#endif
                {
                    std::lock_guard<std::mutex> lock(preferred_backend_mutex);
                    preferred_backends[monitor_key] = CaptureBackend::GDI;
                }
                if (!TryCaptureMonitor<GDIFrameProcessor>(data, monitor)) {
                    std::lock_guard<std::mutex> lock(preferred_backend_mutex);
                    preferred_backends[monitor_key] = CaptureBackend::DXGI;
                }
            }
            else if (data->CommonData_.ExpectedErrorEvent) {
#if defined _DEBUG || !defined NDEBUG
                std::cout << "DirectX Desktop Duplication exited, trying GDI Capturing on restart . . ." << std::endl;
#endif
                std::lock_guard<std::mutex> lock(preferred_backend_mutex);
                preferred_backends[monitor_key] = CaptureBackend::GDI;
            }
        }
    }

    void RunCaptureWindow(std::shared_ptr<Thread_Data> data, Window wnd)
    {
        // need to switch to the input desktop for capturing...
        if (!SwitchToInputDesktop(data))
            return;
        TryCaptureWindow<GDIFrameProcessor>(data, wnd);
    }
} // namespace Screen_Capture
} // namespace SL
