#pragma once
#include "SCCommon.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {
		struct CGMouseProcessorImpl;
		class CGMouseProcessor {
			std::unique_ptr<CGMouseProcessorImpl> _CGMouseProcessorImpl;
		public:
			CGMouseProcessor();
			~CGMouseProcessor();
			DUPL_RETURN Init(std::shared_ptr<Mouse_Thread_Data> data);
			DUPL_RETURN ProcessFrame();

		};

	}
}