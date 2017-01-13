#include "CGFrameProcessor.h"
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>

namespace SL {
	namespace Screen_Capture {
        struct CGFrameProcessorImpl {

			std::shared_ptr<Monitor_Thread_Data> Data;
			std::unique_ptr<char[]> OldImageBuffer, NewImageBuffer;
			size_t ImageBufferSize;
			bool FirstRun;
		};


		CGFrameProcessor::CGFrameProcessor()
		{
			_CGFrameProcessorImpl = std::make_unique<CGFrameProcessorImpl>();
			_CGFrameProcessorImpl->ImageBufferSize = 0;
			_CGFrameProcessorImpl->FirstRun = true;
		}

		CGFrameProcessor::~CGFrameProcessor()
		{

		}
		DUPL_RETURN CGFrameProcessor::Init(std::shared_ptr<Monitor_Thread_Data> data) {
            auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
            _CGFrameProcessorImpl->Data = data;
            _CGFrameProcessorImpl->ImageBufferSize  = Height(*data->SelectedMonitor)*Width(*data->SelectedMonitor)*PixelStride;
          	if (_CGFrameProcessorImpl->Data->CaptureDifMonitor) {//only need the old buffer if difs are needed. If no dif is needed, then the image is always new
				_CGFrameProcessorImpl->OldImageBuffer = std::make_unique<char[]>(_CGFrameProcessorImpl->ImageBufferSize);
			}
			_CGFrameProcessorImpl->NewImageBuffer = std::make_unique<char[]>(_CGFrameProcessorImpl->ImageBufferSize);
		
            
            
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN CGFrameProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;
            auto imageRef = CGDisplayCreateImage(Id(*_CGFrameProcessorImpl->Data->SelectedMonitor));
        
            auto width = CGImageGetWidth(imageRef);
            auto height = CGImageGetHeight(imageRef);
            auto colorSpace = CGColorSpaceCreateDeviceRGB();
            auto rawData = _CGFrameProcessorImpl->NewImageBuffer.get();
           
            auto bytesPerRow = PixelStride * width;
            auto bitsPerComponent = 8;
            auto context = CGBitmapContextCreate(rawData, width, height,
                                                         bitsPerComponent, bytesPerRow, colorSpace,
                                                         kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
            CGColorSpaceRelease(colorSpace);
            
            CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
            CGContextRelease(context);
            CGImageRelease(imageRef);
            ImageRect imgrect;
            imgrect.left =  imgrect.top=0;
            imgrect.right =width;
            imgrect.bottom = height;
            if(_CGFrameProcessorImpl->Data->CaptureEntireMonitor){
           
                auto img = CreateImage(imgrect, PixelStride, 0,_CGFrameProcessorImpl->NewImageBuffer.get());
                _CGFrameProcessorImpl->Data->CaptureEntireMonitor(*img, *_CGFrameProcessorImpl->Data->SelectedMonitor);
            }
			if (_CGFrameProcessorImpl->Data->CaptureDifMonitor) {
				if (_CGFrameProcessorImpl->FirstRun) {
						//first time through, just send the whole image
						auto wholeimgfirst = CreateImage(imgrect, PixelStride, 0, _CGFrameProcessorImpl->NewImageBuffer.get());
						_CGFrameProcessorImpl->Data->CaptureDifMonitor(*wholeimgfirst, *_CGFrameProcessorImpl->Data->SelectedMonitor);
						_CGFrameProcessorImpl->FirstRun = false;
				} else {		
				
				
					//user wants difs, lets do it!
					auto newimg = CreateImage(imgrect, PixelStride, 0, _CGFrameProcessorImpl->NewImageBuffer.get());
					auto oldimg = CreateImage(imgrect, PixelStride, 0, _CGFrameProcessorImpl->OldImageBuffer.get());
					auto imgdifs = GetDifs(*oldimg, *newimg);

					for (auto& r : imgdifs) {
						auto padding = (r.left *PixelStride) + ((Width(*newimg) - r.right)*PixelStride);
						auto startsrc = _CGFrameProcessorImpl->NewImageBuffer.get();
						startsrc += (r.left *PixelStride) + (r.top *PixelStride *Width(*newimg));

						auto difimg = CreateImage(r, PixelStride, padding, startsrc);
						_CGFrameProcessorImpl->Data->CaptureDifMonitor(*difimg, *_CGFrameProcessorImpl->Data->SelectedMonitor);

					}
				}
					std::swap(_CGFrameProcessorImpl->NewImageBuffer, _CGFrameProcessorImpl->OldImageBuffer);
				}
			return Ret;
		}

	}
}