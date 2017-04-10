#include "ScreenCapture.h"
#include "SCCommon.h"
#include <ApplicationServices/ApplicationServices.h>


namespace SL{
    namespace Screen_Capture{
        
        std::vector<std::shared_ptr<Monitor>> GetMonitors() {
            std::vector<std::shared_ptr<Monitor>> ret;
            std::vector<CGDirectDisplayID> displays;
            CGDisplayCount count=0;
            //get count
            CGGetActiveDisplayList(0, 0, &count);
            displays.resize(count);
    
            CGGetActiveDisplayList(count, displays.data(), &count);
            for(auto  i = 0; i < count; i++) {
                //only include non-mirrored displays
                if(CGDisplayMirrorsDisplay(displays[i]) == kCGNullDirectDisplay){
                    
    
                    auto width = CGDisplayPixelsWide(displays[i]);
                    auto height = CGDisplayPixelsHigh(displays[i]);
      
                    auto r = CGDisplayBounds(displays[i]);
                 
                    auto name = std::string("Monitor ") + std::to_string(displays[i]);
                    ret.push_back(CreateMonitor(i, displays[i],height,width, int(r.origin.x), int(r.origin.y), name ));
                }
            }
            return ret;

        }
    }
}
