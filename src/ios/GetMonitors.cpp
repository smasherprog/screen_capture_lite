#include "ScreenCapture.h"
#include <ApplicationServices/ApplicationServices.h>


namespace SL{
    namespace Screen_Capture{
        
        std::vector<Monitor> GetMonitors() {
            std::vector<Monitor> ret;
            std::vector<CGDirectDisplayID> displays;
            CGDisplayCount count=0;
            //get count
            CGGetActiveDisplayList(0, 0, &count);
            displays.resize(count);
    
            CGGetActiveDisplayList(count, displays.data(), &count);
            for(auto  i = 0; i < count; i++) {
                auto r = CGDisplayBounds(displays[i]);
                Monitor tmp;
                tmp.Id=displays[i];
                tmp.OffsetY=int(r.origin.y);
                tmp.OffsetX=int(r.origin.x);
                tmp.Width=int(r.size.width);
                tmp.Height=int(r.size.height);
                tmp.Name = std::string("Monitor ") + std::to_string(tmp.Id);
                ret.push_back(tmp);
            }
            return ret;

        }
    }
}
