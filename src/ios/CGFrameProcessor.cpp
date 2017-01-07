#include "CGFrameProcessor.h"
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>

namespace SL {
	namespace Screen_Capture {
        struct CGFrameProcessorImpl {

			std::shared_ptr<THREAD_DATA> Data;
					std::unique_ptr<char[]> OldImageBuffer, NewImageBuffer;
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
            auto rawData = _CGFrameProcessorImpl->ImageBufferSize.get();
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
                ImageRect imgrect;
                imgrect.left =  imgrect.top=0;
                imgrect.right =width;
                imgrect.bottom = height;
                auto img = CreateImage(imgrect, PixelStride, 0,_CGFrameProcessorImpl->ImageBufferSize.get());
                _CGFrameProcessorImpl->Data->CaptureEntireMonitor(*img, *_CGFrameProcessorImpl->Data->SelectedMonitor);
            }
			if (_CGFrameProcessorImpl->Data->CaptureDifMonitor) {
					//user wants difs, lets do it!
					auto newimg = CreateImage(ret, PixelStride, 0, _CGFrameProcessorImpl->NewImageBuffer.get());
					auto oldimg = CreateImage(ret, PixelStride, 0, _CGFrameProcessorImpl->OldImageBuffer.get());
					auto imgdifs = GetDifs(*oldimg, *newimg);

					for (auto& r : imgdifs) {
						auto padding = (r.left *PixelStride) + ((Width(*newimg) - r.right)*PixelStride);
						auto startsrc = _CGFrameProcessorImpl->NewImageBuffer.get();
						startsrc += (r.left *PixelStride) + (r.top *PixelStride *Width(*newimg));

						auto difimg = CreateImage(r, PixelStride, padding, startsrc);
						_CGFrameProcessorImpl->Data->CaptureDifMonitor(*difimg, *_CGFrameProcessorImpl->Data->SelectedMonitor);

					}
					std::swap(_CGFrameProcessorImpl->NewImageBuffer, _CGFrameProcessorImpl->OldImageBuffer);
				}
			return Ret;
		}

	}
}