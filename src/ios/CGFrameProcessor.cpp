#include "CGFrameProcessor.h"
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>

namespace SL {
	namespace Screen_Capture {
        struct CGFrameProcessorImpl {

			std::shared_ptr<THREAD_DATA> Data;
			std::unique_ptr<char[]> ImageBuffer;
			size_t ImageBufferSize;
		};


		CGFrameProcessor::CGFrameProcessor()
		{
			_CGFrameProcessorImpl = std::make_unique<CGFrameProcessorImpl>();
			_CGFrameProcessorImpl->ImageBufferSize = 0;
		}

		CGFrameProcessor::~CGFrameProcessor()
		{

		}
		DUPL_RETURN CGFrameProcessor::Init(std::shared_ptr<THREAD_DATA> data) {
            auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
              //CGDisplayRegisterReconfigurationCallback(<#CGDisplayReconfigurationCallBack  _Nullable callback#>, <#void * _Nullable userInfo#>)
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN CGFrameProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;

			return Ret;
		}

	}
}