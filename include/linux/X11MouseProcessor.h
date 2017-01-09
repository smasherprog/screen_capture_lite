#pragma once
#include "SCCommon.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {
		struct X11MouseProcessorImpl;
		class X11MouseProcessor {
			std::unique_ptr<X11MouseProcessorImpl> _X11MouseProcessorImpl;
		public:
			X11MouseProcessor();
			~X11MouseProcessor();
			DUPL_RETURN Init(std::shared_ptr<Mouse_Thread_Data> data);
			DUPL_RETURN ProcessFrame();

		};

	}
}