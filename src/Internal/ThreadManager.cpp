#include "ThreadManager.h"
#include <assert.h>
#include <algorithm>

SL::Screen_Capture::ThreadManager::ThreadManager()
{
}
SL::Screen_Capture::ThreadManager::~ThreadManager()
{
    Join();
}

void SL::Screen_Capture::ThreadManager::Init(const std::shared_ptr<Thread_Data>& data)
{
    assert(m_ThreadHandles.empty());

    if (data->getMonitorsToWatch) {

        auto framecapturting = data->OnFrameChanged || data->OnNewFrame;
        std::vector<Monitor> monitors;
        if (framecapturting) {
            assert(data->getMonitorsToWatch);
            monitors = data->getMonitorsToWatch();
        }
        // veryify the captured area exists within the monitors
        // inefficent below, but its only called when the library is restarting so the cost is ZERO!!!
        auto mons = GetMonitors();
        for (auto& m : monitors) {
            assert(isMonitorInsideBounds(mons, m));
        }

        m_ThreadHandles.resize(monitors.size() + (data->OnMouseChanged ? 1 : 0)); // add another thread for mouse capturing if needed

        for (size_t i = 0; i < monitors.size(); ++i) {
            m_ThreadHandles[i] = std::thread(&SL::Screen_Capture::RunCaptureMonitor, data, monitors[i]);
        }

    }
    else if (data->getWindowToWatch) {

        auto framecapturting = data->OnWindowChanged || data->OnWindowNewFrame;
        std::vector<Window> windows;
        if (framecapturting) {
            assert(data->getWindowToWatch);
            windows = data->getWindowToWatch();
        }

        m_ThreadHandles.resize(windows.size() + (data->OnMouseChanged ? 1 : 0)); // add another thread for mouse capturing if needed

        for (size_t i = 0; i < windows.size(); ++i) {
            m_ThreadHandles[i] = std::thread(&SL::Screen_Capture::RunCaptureWindow, data, windows[i]);
        }
    }
    if (data->OnMouseChanged) {
        m_ThreadHandles.back() = std::thread(&SL::Screen_Capture::RunCaptureMouse, data);
    }
}

void SL::Screen_Capture::ThreadManager::Join()
{
    for (auto& t : m_ThreadHandles) {
        if (t.joinable()) {
            t.join();
        }
    }
    m_ThreadHandles.clear();
}
