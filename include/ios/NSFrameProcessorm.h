#pragma once
#import "SCCommon.h"
#import "NSFrameProcessor.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

@interface FrameProcessor: NSObject<AVCaptureVideoDataOutputSampleBufferDelegate>
    @property(nonatomic, assign) SL::Screen_Capture::NSFrameProcessor* nsframeprocessor;
    @property(nonatomic, retain) AVCaptureSession *avcapturesession;
    @property(nonatomic, retain) AVCaptureScreenInput* avinput;
    @property(atomic) bool Working;
    -(SL::Screen_Capture::DUPL_RETURN) Init:(SL::Screen_Capture::NSFrameProcessor*) parent;
@end


