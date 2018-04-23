#include "NSFrameProcessorm.h"
#include <thread>
#include <chrono>
 
@implementation FrameProcessor

-(SL::Screen_Capture::DUPL_RETURN) Init:(SL::Screen_Capture::NSFrameProcessor*) parent
{
    self = [super init];
    if (self) {
        self.Working = false;
        self.nsframeprocessor = parent;
        self.avcapturesession = [[AVCaptureSession alloc] init];
        
        self.avinput = [[[AVCaptureScreenInput alloc] initWithDisplayID:SL::Screen_Capture::Id(parent->SelectedMonitor)] autorelease];
        [self.avcapturesession addInput:self.avinput];
        
        auto output = [[[AVCaptureVideoDataOutput alloc] init] autorelease];
        NSDictionary* videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey, nil];
        
        [output setVideoSettings:videoSettings];
        [output setAlwaysDiscardsLateVideoFrames:true];
        //partial capture needed
        if(parent->SelectedMonitor.OffsetX != parent->SelectedMonitor.OriginalOffsetX ||
           parent->SelectedMonitor.OffsetY != parent->SelectedMonitor.OriginalOffsetY ||
           parent->SelectedMonitor.Height != parent->SelectedMonitor.OriginalHeight ||
           parent->SelectedMonitor.Width != parent->SelectedMonitor.OriginalWidth){
                CGRect r;
                r.origin.x = parent->SelectedMonitor.OffsetX;
                //apple uses the opengl texture coords where the bottom left is 0,0
                r.origin.y = parent->SelectedMonitor.OriginalHeight - (parent->SelectedMonitor.OffsetY + parent->SelectedMonitor.Height);
                r.size.height =parent->SelectedMonitor.Height;
                r.size.width =parent->SelectedMonitor.Width;
                [self.avinput setCropRect:r];
        }
        
        [self.avinput setMinFrameDuration:CMTimeMake(1, 10)];
    
        self.avinput.capturesCursor = false;
        self.avinput.capturesMouseClicks = false;
        
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
    if(!self.avcapturesession.isRunning){
        self.Working = false;
        return;
    }
    auto data=self.nsframeprocessor->Data;
    auto& selectedmonitor =self.nsframeprocessor->SelectedMonitor;

    auto imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    auto bytesperrow = CVPixelBufferGetBytesPerRow(imageBuffer);
    auto buf = static_cast<unsigned char*>(CVPixelBufferGetBaseAddress(imageBuffer));
    SL::Screen_Capture::ProcessCapture(data->ScreenCaptureData, *(self.nsframeprocessor), selectedmonitor, buf, bytesperrow);
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    self.Working = false;
}
@end

namespace SL{
    namespace Screen_Capture{
        void SetFrameInterval(FrameProcessor* f, int ms){
            
        }
        struct NSFrameProcessorImpl{
            FrameProcessor* ptr=nullptr;
            NSFrameProcessorImpl(){
                ptr = [[FrameProcessor alloc] init];
            }
            ~NSFrameProcessorImpl(){
                if(ptr) {
                    [ptr release];
                    ptr = nullptr;
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
