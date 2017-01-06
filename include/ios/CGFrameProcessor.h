#pragma once
#include "SCCommon.h"
#include <memory>

namespace SL {
	namespace Screen_Capture {
		struct CGFrameProcessorImpl;
		class CGFrameProcessor {
			std::unique_ptr<CGFrameProcessorImpl> _CGFrameProcessorImpl;
		public:
			CGFrameProcessor();
			~CGFrameProcessor();
			DUPL_RETURN Init(std::shared_ptr<THREAD_DATA> data);
			DUPL_RETURN ProcessFrame();

		};

	}
}