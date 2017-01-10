#include "CGMouseProcessor.h"
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>

namespace SL {
	namespace Screen_Capture {
        struct CGMouseProcessorImpl {

			std::shared_ptr<Mouse_Thread_Data> Data;
    
		};


		CGMouseProcessor::CGMouseProcessor()
		{
			_CGMouseProcessorImpl = std::make_unique<CGMouseProcessorImpl>();
		}

		CGMouseProcessor::~CGMouseProcessor()
		{

		}
		DUPL_RETURN CGMouseProcessor::Init(std::shared_ptr<Mouse_Thread_Data> data) {
            auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
            _CGMouseProcessorImpl->Data = data;
        
            
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN CGMouseProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;
            auto mouseev = CGEventCreate(NULL);
            auto loc = CGEventGetLocation(mouseev);
            CFRelease(mouseev);
       
			return Ret;
		}

	}
}