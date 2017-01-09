#include "GDIMouseProcessor.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace SL {
	namespace Screen_Capture {
	
		struct GDIMouseProcessorImpl {
			std::shared_ptr<Mouse_Thread_Data> Data;
			std::unique_ptr<char[]> OldImageBuffer, NewImageBuffer;
			size_t ImageBufferSize;
			bool FirstRun;
		};


		GDIMouseProcessor::GDIMouseProcessor()
		{
			_GDIMouseProcessorImpl = std::make_unique<GDIMouseProcessorImpl>();
			_GDIMouseProcessorImpl->ImageBufferSize = 0;
			_GDIMouseProcessorImpl->FirstRun = true;
		}

		GDIMouseProcessor::~GDIMouseProcessor()
		{

		}
		DUPL_RETURN GDIMouseProcessor::Init(std::shared_ptr<Mouse_Thread_Data> data) {
			auto Ret = DUPL_RETURN_SUCCESS;
			
			return Ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN GDIMouseProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;


			return Ret;
		}

	}
}