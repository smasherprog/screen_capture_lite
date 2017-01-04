#include "ScreenCapture.h"
#include <ApplicationServices/ApplicationServices.h>

std::vector<SL::Screen_Capture::Monitor> SL::Screen_Capture::GetMonitors() {
	std::vector<Monitor> ret;
    CGDirectDisplayID displays[16];
    CGDisplayCount count, i;
    CGRect r;
    CGGetActiveDisplayList(16, displays, &count);
    for( i = 0; i < count; i++) {
        r = CGDisplayBounds(displays[i]);
        Monitor tmp;
        tmp.Index=i;
        tmp.OffsetY=int(r.origin.y);
        tmp.OffsetX=int(r.origin.x);
        tmp.Width=int(r.size.width);
        tmp.Height=int(r.size.height);
    
        ret.push_back(tmp);
    }
	return ret;

}