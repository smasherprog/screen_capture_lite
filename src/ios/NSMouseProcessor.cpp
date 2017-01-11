#include "NSMouseProcessor.h"
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>

namespace SL {
	namespace Screen_Capture {
        struct NSMouseProcessorImpl {

			std::shared_ptr<Mouse_Thread_Data> Data;
    
		};


		NSMouseProcessor::NSMouseProcessor()
		{
			_NSMouseProcessorImpl = std::make_unique<NSMouseProcessorImpl>();
		}

		NSMouseProcessor::~NSMouseProcessor()
		{

		}
		DUPL_RETURN NSMouseProcessor::Init(std::shared_ptr<Mouse_Thread_Data> data) {
            auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
            _NSMouseProcessorImpl->Data = data;
        
            
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN NSMouseProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;
            auto mouseev = CGEventCreate(NULL);
            auto loc = CGEventGetLocation(mouseev);
            CFRelease(mouseev);
       
			return Ret;
		}

	}
}