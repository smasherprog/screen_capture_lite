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

void SL::Screen_Capture::ThreadManager::Init(std::shared_ptr<std::atomic_bool>& unexpected,
	std::shared_ptr<std::atomic_bool>& expected,
	std::shared_ptr<std::atomic_bool>& terminate,
	CaptureCallback& captureentiremonitor,
	CaptureCallback& capturedifmonitor,
	int mininterval,
	const std::vector<std::shared_ptr<Monitor>>& monitorstocapture)
{
	Reset();
	m_ThreadHandles.resize(monitorstocapture.size());
	m_ThreadData.resize(monitorstocapture.size());

	for (size_t i = 0; i < monitorstocapture.size(); ++i)
	{
		m_ThreadData[i] = std::make_shared<THREAD_DATA>();
		m_ThreadData[i]->UnexpectedErrorEvent = unexpected;
		m_ThreadData[i]->ExpectedErrorEvent = expected;
		m_ThreadData[i]->TerminateThreadsEvent = terminate;
		m_ThreadData[i]->SelectedMonitor = monitorstocapture[i];
		m_ThreadData[i]->CaptureDifMonitor = capturedifmonitor;
		m_ThreadData[i]->CaptureEntireMonitor = captureentiremonitor;
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
