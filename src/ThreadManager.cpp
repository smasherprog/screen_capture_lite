#include "ThreadManager.h"
#ifdef WIN32
#include "ScreenCaptureWindows.h"
#endif

SL::Screen_Capture::ThreadManager::ThreadManager()
{

}
SL::Screen_Capture::ThreadManager::~ThreadManager() {
	for (auto& t : m_ThreadData) {
		*t->TerminateThreadsEvent = true;
	}
	Join();
}

SL::Screen_Capture::DUPL_RETURN SL::Screen_Capture::ThreadManager::Init(std::shared_ptr<std::atomic_bool>& unexpected, std::shared_ptr<std::atomic_bool>& expected, std::shared_ptr<std::atomic_bool>& terminate, ImageCallback & CallBack)
{
	auto monitors = GetMonitors();
	m_ThreadHandles.resize(monitors.size());
	m_ThreadData.resize(monitors.size());

	DUPL_RETURN Ret = DUPL_RETURN_SUCCESS;
	for (int i = 0; i < monitors.size(); ++i)
	{
		m_ThreadData[i] = std::make_shared<THREAD_DATA>();
		m_ThreadData[i]->UnexpectedErrorEvent = unexpected;
		m_ThreadData[i]->ExpectedErrorEvent = expected;
		m_ThreadData[i]->TerminateThreadsEvent = terminate;
		m_ThreadData[i]->SelectedMonitor = monitors[i];
		m_ThreadData[i]->CallBack = CallBack;

#ifdef WIN32
		auto framegrabber = std::make_shared<SL::Screen_Capture::ScreenCaptureWindows>(m_ThreadData[i]);
#endif

		Ret = InitializeDx(&m_ThreadData[i]->DxRes);

		if (Ret != DUPL_RETURN_SUCCESS)
		{
			return Ret;
		}

		m_ThreadHandles[i] = std::thread(&SL::Screen_Capture::RunThread, this, &m_ThreadData[i]);

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
