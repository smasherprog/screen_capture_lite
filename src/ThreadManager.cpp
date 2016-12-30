#include "ThreadManager.h"

SL::Screen_Capture::ThreadManager::ThreadManager()
{

}
SL::Screen_Capture::ThreadManager::~ThreadManager() {
	for (auto& t : m_ThreadData) {
		*t->TerminateThreadsEvent = true;
	}
	Join();
}

void SL::Screen_Capture::ThreadManager::Init(std::shared_ptr<std::atomic_bool>& unexpected, std::shared_ptr<std::atomic_bool>& expected, std::shared_ptr<std::atomic_bool>& terminate, ImageCallback & CallBack, int mininterval)
{
	auto monitors = GetMonitors();
	m_ThreadHandles.resize(monitors.size());
	m_ThreadData.resize(monitors.size());

	for (int i = 0; i < monitors.size(); ++i)
	{
		m_ThreadData[i] = std::make_shared<THREAD_DATA>();
		m_ThreadData[i]->UnexpectedErrorEvent = unexpected;
		m_ThreadData[i]->ExpectedErrorEvent = expected;
		m_ThreadData[i]->TerminateThreadsEvent = terminate;
		m_ThreadData[i]->SelectedMonitor = monitors[i];
		m_ThreadData[i]->CallBack = CallBack;
		m_ThreadData[i]->CaptureInterval = mininterval;
		m_ThreadHandles[i] = std::thread(&SL::Screen_Capture::RunCapture, m_ThreadData[i]);

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
	m_ThreadData.clear();
}
