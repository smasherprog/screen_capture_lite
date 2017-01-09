#pragma once
#include "ScreenCapture.h"
#include "SCCommon.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {

		struct GDIFrameProcessorImpl;
		class GDIFrameProcessor {
			std::unique_ptr<GDIFrameProcessorImpl> _GDIFrameProcessorImpl;
		public:
			GDIFrameProcessor();
			~GDIFrameProcessor();
			DUPL_RETURN Init(std::shared_ptr<Monitor_Thread_Data> data);
			DUPL_RETURN ProcessFrame();

		};
	}
}