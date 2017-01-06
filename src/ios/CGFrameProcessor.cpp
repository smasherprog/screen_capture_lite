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
            _CGFrameProcessorImpl->Data = data;
            _CGFrameProcessorImpl->ImageBufferSize  = data->SelectedMonitor.Height*data->SelectedMonitor.Width*PixelStride;
            _CGFrameProcessorImpl->ImageBuffer = std::make_unique<char[]>(_CGFrameProcessorImpl->ImageBufferSize);
            
            
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN CGFrameProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;
            auto imageRef = CGDisplayCreateImage(_CGFrameProcessorImpl->Data->SelectedMonitor.Id);
        
            auto width = CGImageGetWidth(imageRef);
            auto height = CGImageGetHeight(imageRef);
            auto colorSpace = CGColorSpaceCreateDeviceRGB();
            auto rawData = _CGFrameProcessorImpl->ImageBuffer.get();
            auto bytesPerPixel = 4;
            auto bytesPerRow = bytesPerPixel * width;
            auto bitsPerComponent = 8;
            auto context = CGBitmapContextCreate(rawData, width, height,
                                                         bitsPerComponent, bytesPerRow, colorSpace,
                                                         kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
            CGColorSpaceRelease(colorSpace);
            
            CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
            CGContextRelease(context);
            CGImageRelease(imageRef);
            if(_CGFrameProcessorImpl->Data->CaptureEntireMonitor){
                _CGFrameProcessorImpl->Data->CaptureEntireMonitor(_CGFrameProcessorImpl->ImageBuffer.get(), PixelStride,_CGFrameProcessorImpl->Data->SelectedMonitor);
            }
			return Ret;
		}

	}
}