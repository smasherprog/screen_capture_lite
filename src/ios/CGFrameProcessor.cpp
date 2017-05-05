#include "CGFrameProcessor.h"
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>
#include <iostream>

namespace SL {
    namespace Screen_Capture {
        struct CGFrameProcessorImpl {
            std::shared_ptr<Monitor_Thread_Data> Data;
 
        };


        CGFrameProcessor::CGFrameProcessor()
        {
            _CGFrameProcessorImpl = std::make_unique<CGFrameProcessorImpl>();
        }

        CGFrameProcessor::~CGFrameProcessor()
        {

        }
        DUPL_RETURN CGFrameProcessor::Init(std::shared_ptr<Monitor_Thread_Data> data) {
            auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
            _CGFrameProcessorImpl->Data = data;

            return ret;
        }
        //
        // Process a given frame and its metadata
        //
        DUPL_RETURN CGFrameProcessor::ProcessFrame()
        {
            auto Ret = DUPL_RETURN_SUCCESS;
            auto imageRef = CGDisplayCreateImage(Id(_CGFrameProcessorImpl->Data->SelectedMonitor));
            if(!imageRef) return DUPL_RETURN_ERROR_EXPECTED;//this happens when the monitors change.
            
            auto width = CGImageGetWidth(imageRef);
            auto height = CGImageGetHeight(imageRef);
            if(width!= Width(_CGFrameProcessorImpl->Data->SelectedMonitor) || height!= Height(_CGFrameProcessorImpl->Data->SelectedMonitor)){
                 CGImageRelease(imageRef);
                return DUPL_RETURN_ERROR_EXPECTED;//this happens when the monitors change.
            }
            
            auto prov = CGImageGetDataProvider(imageRef);
            if(!prov){
                 CGImageRelease(imageRef);
                return DUPL_RETURN_ERROR_EXPECTED;
            }
            auto bytesperrow = CGImageGetBytesPerRow(imageRef);
            auto bitsperpixel = CGImageGetBitsPerPixel(imageRef);
            // right now only support full 32 bit images.. Most desktops should run this as its the most efficent
            assert(bitsperpixel == PixelStride*8);
            
            auto rawdatas= CGDataProviderCopyData(prov);
            auto buf = CFDataGetBytePtr(rawdatas);
         
            auto datalen = width*height*PixelStride;
            if(bytesperrow == PixelStride*width){
                //most efficent, can be done in a single memcpy
                memcpy(_CGFrameProcessorImpl->Data->NewImageBuffer.get(),buf, datalen);
            } else {
                //for loop needed to copy each row
                auto dst =_CGFrameProcessorImpl->Data->NewImageBuffer.get();
                auto src =buf;
                for (auto h =0; h<height;h++) {
                    memcpy(dst,src, PixelStride*width);
                    dst +=PixelStride*width;
                    src +=bytesperrow;
                }
            }
            CFRelease(rawdatas);
 
            //this is not needed. It is freed when the image is released
            //CGDataProviderRelease(prov);
            
            CGImageRelease(imageRef);

            ImageRect imgrect;
            imgrect.left =  imgrect.top=0;
            imgrect.right =width;
            imgrect.bottom = height;
            ProcessMonitorCapture(*_CGFrameProcessorImpl->Data, imgrect);
            return Ret;
        }

    }
}
