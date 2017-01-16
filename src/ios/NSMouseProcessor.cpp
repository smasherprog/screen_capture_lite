#include "NSMouseProcessor.h"
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>
#include "NSMouseCapture.h"
#include <iostream>

namespace SL {
	namespace Screen_Capture {
        struct NSMouseProcessorImpl {
            const int MaxCursurorSize = 48;
			std::shared_ptr<Mouse_Thread_Data> Data;
            std::unique_ptr<char[]> NewImageBuffer, LastImageBuffer;
            size_t ImageBufferSize;
            bool FirstRun;
            int Last_x, Last_y;
		};


		NSMouseProcessor::NSMouseProcessor()
		{
			_NSMouseProcessorImpl = std::make_unique<NSMouseProcessorImpl>();
            _NSMouseProcessorImpl->ImageBufferSize = 0;
            _NSMouseProcessorImpl->FirstRun = true;
            _NSMouseProcessorImpl->Last_x = _NSMouseProcessorImpl->Last_y = 0;
		}

		NSMouseProcessor::~NSMouseProcessor()
		{

		}
		DUPL_RETURN NSMouseProcessor::Init(std::shared_ptr<Mouse_Thread_Data> data) {
            auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
            _NSMouseProcessorImpl->Data = data;
            SLScreen_Capture_InitMouseCapture();
            _NSMouseProcessorImpl->ImageBufferSize = _NSMouseProcessorImpl->MaxCursurorSize* _NSMouseProcessorImpl->MaxCursurorSize* PixelStride;
            _NSMouseProcessorImpl->NewImageBuffer = std::make_unique<char[]>(_NSMouseProcessorImpl->ImageBufferSize);
            _NSMouseProcessorImpl->LastImageBuffer = std::make_unique<char[]>(_NSMouseProcessorImpl->ImageBufferSize);
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
                
                auto prov = CGImageGetDataProvider(imageRef);

                auto rawdatas= CGDataProviderCopyData(prov);
                auto buf = CFDataGetBytePtr(rawdatas);
                auto datalen = CFDataGetLength(rawdatas);
                memcpy(_NSMouseProcessorImpl->NewImageBuffer.get(),buf, datalen);
                CFRelease(rawdatas);
               
               //this is not needed. It is freed when the image is released
                //CGDataProviderRelease(prov);
               
                CGImageRelease(imageRef);
                
                ImageRect imgrect;
                imgrect.left =  imgrect.top=0;
                imgrect.right =width;
                imgrect.bottom = height;
                auto wholeimgfirst = CreateImage(imgrect, PixelStride, 0, _NSMouseProcessorImpl->NewImageBuffer.get());
                
              
                auto lastx =loc.x;
                auto lasty = loc.y;
                    //if the mouse image is different, send the new image and swap the data
                                if (memcmp(_NSMouseProcessorImpl->NewImageBuffer.get(), _NSMouseProcessorImpl->LastImageBuffer.get(), datalen) != 0) {
                    _NSMouseProcessorImpl->Data->CaptureCallback(wholeimgfirst.get(), lastx, lasty);
                    std::swap(_NSMouseProcessorImpl->NewImageBuffer,_NSMouseProcessorImpl->LastImageBuffer );
        
                }
                else if(_NSMouseProcessorImpl->Last_x != lastx || _NSMouseProcessorImpl->Last_y != lasty){
                    _NSMouseProcessorImpl->Data->CaptureCallback(nullptr, lastx, lasty);
                }
                _NSMouseProcessorImpl->Last_x = lastx;
                _NSMouseProcessorImpl->Last_y = lasty;
          
            }
			return Ret;
		}

	}
}