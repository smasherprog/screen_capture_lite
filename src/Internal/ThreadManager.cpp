#include "ThreadManager.h"
#include <assert.h>

SL::Screen_Capture::ThreadManager::ThreadManager()
{

}
SL::Screen_Capture::ThreadManager::~ThreadManager() {
    *TerminateThreadsEvent = true;
    Join();
}

void SL::Screen_Capture::ThreadManager::Init(const std::shared_ptr<Thread_Data>& data)
{
    Reset();
    TerminateThreadsEvent = data->TerminateThreadsEvent;

    auto monitorcapturing = data->CaptureDifMonitor || data->CaptureEntireMonitor;
    std::vector<std::shared_ptr<Monitor>> monitors;
    if (monitorcapturing) {
        assert(data->MonitorsChanged);
        monitors = data->MonitorsChanged();
    }

    m_ThreadHandles.resize(monitors.size() + (data->CaptureMouse ? 1 : 0));// add another thread for mouse capturing if needed

    for (size_t i = 0; i < monitors.size(); ++i)
    {   
        m_ThreadHandles[i] = std::thread(&SL::Screen_Capture::RunCaptureMonitor, data, *monitors[i]);
    }


    if (data->CaptureMouse) {
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
}

void SL::Screen_Capture::ThreadManager::Reset()
{
    m_ThreadHandles.clear();
}
