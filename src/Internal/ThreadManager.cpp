#include "ThreadManager.h"
#include <assert.h>

SL::Screen_Capture::ThreadManager::ThreadManager()
{

}
SL::Screen_Capture::ThreadManager::~ThreadManager() {
	*TerminateThreadsEvent = true;
	Join();
}

void SL::Screen_Capture::ThreadManager::Init(const Base_Thread_Data& data, const ScreenCapture_Settings& settings)
{
	Reset();
	TerminateThreadsEvent = data.TerminateThreadsEvent;

	auto monitorcapturing = settings.CaptureDifMonitor || settings.CaptureEntireMonitor;
	std::vector<std::shared_ptr<Monitor>> monitors;
	if (monitorcapturing) {
		assert(settings.MonitorsChanged);
		monitors = settings.MonitorsChanged();
	}

	m_ThreadHandles.resize(monitors.size() + (settings.CaptureMouse ? 1 : 0));// add another thread for mouse capturing if needed

	for (size_t i = 0; i < monitors.size(); ++i)
	{
		auto tdata = std::make_shared<Monitor_Thread_Data>();
		tdata->UnexpectedErrorEvent = data.UnexpectedErrorEvent;
		tdata->ExpectedErrorEvent = data.ExpectedErrorEvent;
		tdata->TerminateThreadsEvent = data.TerminateThreadsEvent;
		tdata->SelectedMonitor = *monitors[i];
		tdata->CaptureDifMonitor = settings.CaptureDifMonitor;
		tdata->CaptureEntireMonitor = settings.CaptureEntireMonitor;
		tdata->CaptureInterval = settings.Monitor_Capture_Interval;
		m_ThreadHandles[i] = std::thread(&SL::Screen_Capture::RunCapture, tdata);
	}


	if (settings.CaptureMouse) {
		auto mousedata = std::make_shared<Mouse_Thread_Data>();

		mousedata->UnexpectedErrorEvent = data.UnexpectedErrorEvent;
		mousedata->ExpectedErrorEvent = data.ExpectedErrorEvent;
		mousedata->TerminateThreadsEvent = data.TerminateThreadsEvent;
		mousedata->CaptureCallback = settings.CaptureMouse;
		mousedata->CaptureInterval = settings.Mouse_Capture_Interval;

		m_ThreadHandles.back() = std::thread(&SL::Screen_Capture::RunCaptureMouse, mousedata);
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
