#include "CGFrameProcessor.h"
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>
#include <iostream>

namespace SL {
    namespace Screen_Capture {
 
   
        DUPL_RETURN CGFrameProcessor::Init(std::shared_ptr<Thread_Data> data, Monitor& monitor) {
            auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
            Data = data;
            SelectedMonitor = monitor;
            return ret;
        }
        //
        // Process a given frame and its metadata
        //
        DUPL_RETURN CGFrameProcessor::ProcessFrame()
        {
            auto Ret = DUPL_RETURN_SUCCESS;
            auto imageRef = CGDisplayCreateImage(Id(SelectedMonitor));
            if(!imageRef) return DUPL_RETURN_ERROR_EXPECTED;//this happens when the monitors change.
            
            auto width = CGImageGetWidth(imageRef);
            auto height = CGImageGetHeight(imageRef);
            if(width!= Width(SelectedMonitor) || height!= Height(SelectedMonitor)){
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
                memcpy(NewImageBuffer.get(),buf, datalen);
            } else {
                //for loop needed to copy each row
                auto dst =NewImageBuffer.get();
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
            ProcessMonitorCapture(*Data, *this, SelectedMonitor ,imgrect);
            return Ret;
        }

    }
}
