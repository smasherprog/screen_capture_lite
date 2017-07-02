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
    auto monitorcapturing = data->CaptureDifMonitor || data->CaptureEntireMonitor;
    std::vector<Monitor> monitors;
    if(monitorcapturing) {
        assert(data->MonitorsChanged);
        monitors = data->MonitorsChanged();
    }
    // veryify the captured area exists within the monitors
    // inefficent below, but its only called when the library is restarting so the cost is ZERO!!!
    auto mons = GetMonitors();
    for(auto& m : monitors) {
        assert(isMonitorInsideBounds(mons, m));
    }

    m_ThreadHandles.resize(monitors.size() +
                           (data->CaptureMouse ? 1 : 0)); // add another thread for mouse capturing if needed

    for(size_t i = 0; i < monitors.size(); ++i) {
        m_ThreadHandles[i] = std::thread(&SL::Screen_Capture::RunCaptureMonitor, data, monitors[i]);
    }

    if(data->CaptureMouse) {
        m_ThreadHandles.back() = std::thread(&SL::Screen_Capture::RunCaptureMouse, data);
    }
}

void SL::Screen_Capture::ThreadManager::Join()
{
    for(auto& t : m_ThreadHandles) {
        if(t.joinable()) {
            t.join();
        }
    }
    m_ThreadHandles.clear();
}
