#pragma once
#include "ScreenCapture.h"
#include "SCCommon.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {

		struct GDIMouseProcessorImpl;
		class GDIMouseProcessor {
			std::unique_ptr<GDIMouseProcessorImpl> _GDIMouseProcessorImpl;

		public:

			GDIMouseProcessor();
			~GDIMouseProcessor();
			DUPL_RETURN Init(std::shared_ptr<Mouse_Thread_Data> data);
			DUPL_RETURN ProcessFrame();

		};
	}
}