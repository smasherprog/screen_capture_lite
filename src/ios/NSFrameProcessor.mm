#include "NSFrameProcessorm.h"
#include <thread>
#include <chrono>

@implementation FrameProcessor

-(SL::Screen_Capture::DUPL_RETURN) Init:(SL::Screen_Capture::NSFrameProcessor*) parent
{
    self = [super init];
    self.Working = false;
    if (self) {
        self.nsframeprocessor = parent;
        self.avcapturesession = [[AVCaptureSession alloc] init];
        
        auto input = [[[AVCaptureScreenInput alloc] initWithDisplayID:SL::Screen_Capture::Id(parent->SelectedMonitor)] autorelease];
        [self.avcapturesession addInput:input];
        
        auto output = [[[AVCaptureVideoDataOutput alloc] init] autorelease];
        NSDictionary* videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey, nil];
        
        [output setVideoSettings:videoSettings];
        [output setAlwaysDiscardsLateVideoFrames:true];
        [input setMinFrameDuration:CMTimeMake(1, 30)]; 
        input.capturesCursor = false;
        input.capturesMouseClicks = false;
        
        [self.avcapturesession addOutput:output];
        [output setSampleBufferDelegate:self queue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0)];
        [self.avcapturesession startRunning];
           return SL::Screen_Capture::DUPL_RETURN::DUPL_RETURN_SUCCESS;
    }
    return SL::Screen_Capture::DUPL_RETURN::DUPL_RETURN_ERROR_UNEXPECTED;
}

- (void)dealloc
{
    [self.avcapturesession stopRunning];
    while(self.avcapturesession.isRunning || self.Working){
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    [self.avcapturesession release];
    [super dealloc];
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {

    self.Working = true;
    auto data=self.nsframeprocessor->Data;
    if(!self.avcapturesession.isRunning || !self.nsframeprocessor || !data){
        self.Working = false;
        return;
    }
    auto& selectedmonitor =self.nsframeprocessor->SelectedMonitor;
    SL::Screen_Capture::ImageRect ret = {0};
    ret.left = 0;
    ret.top = 0;
    ret.bottom = Height(selectedmonitor);
    ret.right = Width(selectedmonitor);
    
    auto imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    auto bytesperrow = CVPixelBufferGetBytesPerRow(imageBuffer);
    
    auto buf = static_cast<unsigned char*>(CVPixelBufferGetBaseAddress(imageBuffer));
    
    auto rowstride = SL::Screen_Capture::PixelStride * SL::Screen_Capture::Width(selectedmonitor);
    auto startbuf = buf + (SL::Screen_Capture::OffsetX(selectedmonitor)*SL::Screen_Capture::PixelStride);//advance to the start of this image
    startbuf += (SL::Screen_Capture::OffsetY(selectedmonitor) *  bytesperrow);
    SL::Screen_Capture::ProcessCapture(data->ScreenCaptureData, *(self.nsframeprocessor), selectedmonitor, startbuf, bytesperrow);
 
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    self.Working = false;
}
@end

namespace SL{
    namespace Screen_Capture{
        struct NSFrameProcessorImpl{
            FrameProcessor* ptr=nullptr;
            NSFrameProcessorImpl(){
                ptr = [[FrameProcessor alloc] init];
            }
            ~NSFrameProcessorImpl(){
                if(ptr) {
                    [ptr release];
                }
            }
            DUPL_RETURN Init(NSFrameProcessor* parent){
                return [ptr Init:parent];
            }
        };
        NSFrameProcessorImpl* CreateNSFrameProcessorImpl(){
            return new NSFrameProcessorImpl();
        }
        void DestroyNSFrameProcessorImpl(NSFrameProcessorImpl* p){
            if(p){
                delete p;
            }
        }
        DUPL_RETURN Init(NSFrameProcessorImpl* createdimpl, NSFrameProcessor* parent){
            if(createdimpl){
                return createdimpl->Init(parent);
            }
            return DUPL_RETURN::DUPL_RETURN_ERROR_UNEXPECTED;
        }
    }
}
