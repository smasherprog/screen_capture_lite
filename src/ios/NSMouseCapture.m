//
//  NSMouseCapture.m
//  Screen_Capture
//
//  Created by scott lee on 1/11/17.
//
//

#import <Foundation/Foundation.h>
#import <appkit/appkit.h>
#import "NSMouseCapture.h"

void SLScreen_Capture_InitMouseCapture(){
    [NSApplication sharedApplication];
}
CGImageRef SLScreen_Capture_GetCurrentMouseImage(){
    CGImageRef img=NULL;
 
    @autoreleasepool {
        NSCursor *cur = [NSCursor currentSystemCursor];
           if(cur==nil) return img;
        NSImage *overlay    =  [cur image];
        CGImageSourceRef source = CGImageSourceCreateWithData((CFDataRef)[overlay TIFFRepresentation], NULL);
        img = CGImageSourceCreateImageAtIndex(source, 0, NULL);
        CFRelease(source);
    }
 
    return img;
} 