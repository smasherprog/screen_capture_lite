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
        DUPL_RETURN CGFrameProcessor::ProcessFrame(const Monitor& curentmonitorinfo)
        {
            auto Ret = DUPL_RETURN_SUCCESS;
            
            CGImageRef imageRef;
            
            if(Height(curentmonitorinfo) != Height(SelectedMonitor) || Width(curentmonitorinfo) != Width(SelectedMonitor)){
                CGRect rec;
                rec.origin.y =OffsetY(SelectedMonitor);
                rec.origin.x =OffsetX(SelectedMonitor);
                rec.size.width =Width(SelectedMonitor);
                rec.size.height =Height(SelectedMonitor);
                
                imageRef = CGDisplayCreateImageForRect(Id(SelectedMonitor), rec);
            } else {
                imageRef =  CGDisplayCreateImage(Id(SelectedMonitor));
            }
         
            
            if(!imageRef) return DUPL_RETURN_ERROR_EXPECTED;//this happens when the monitors change.
            
            auto width = CGImageGetWidth(imageRef);
            auto height = CGImageGetHeight(imageRef);

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
            ImageRect ret;
            ret.left =  ret.top=0;
            ret.right =width;
            ret.bottom = height;
            if(Data->CaptureEntireMonitor && !Data->CaptureDifMonitor) {
                
                auto wholeimg = SL::Screen_Capture::Create(ret, PixelStride, bytesperrow - PixelStride*width, buf);
                Data->CaptureEntireMonitor(wholeimg, SelectedMonitor);
                
            } else {
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
                
                ProcessMonitorCapture(*Data, *this, SelectedMonitor ,ret);
            }
            
            CFRelease(rawdatas);
            CGImageRelease(imageRef);
            
            
            return Ret;
        }
        
    }
}
