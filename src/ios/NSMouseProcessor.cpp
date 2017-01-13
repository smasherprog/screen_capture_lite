#include "NSMouseProcessor.h"
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>
#include "NSMouseCapture.h"

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
            SLScreen_Capture_InitMouseCapture();
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN NSMouseProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;
        
            if(_NSMouseProcessorImpl->Data->CaptureCallback){
                auto mouseev = CGEventCreate(NULL);
                auto loc = CGEventGetLocation(mouseev);
                CFRelease(mouseev);
                
                auto imageRef = SLScreen_Capture_GetCurrentMouseImage();
                if(imageRef==NULL) return Ret;
                auto width = CGImageGetWidth(imageRef);
                auto height = CGImageGetHeight(imageRef);
                auto colorSpace = CGColorSpaceCreateDeviceRGB();
             
       
                auto bytesPerRow = PixelStride * width;
                auto bitsPerComponent = 8;
                auto tmp = std::make_unique<char[]>(width*height*PixelStride);
                auto rawData = tmp.get();
                
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
                auto wholeimgfirst = CreateImage(imgrect, PixelStride, 0, tmp.get());
                _NSMouseProcessorImpl->Data->CaptureCallback(*wholeimgfirst, loc.x, loc.y);
            }
			return Ret;
		}

	}
}