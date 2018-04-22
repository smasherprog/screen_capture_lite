#include "ScreenCapture.h"
#include "internal/SCCommon.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>
#include <dlfcn.h>

namespace SL
{
namespace Screen_Capture
{

    std::vector<Monitor> GetMonitors()
    {
        std::vector<Monitor> ret; 
        Display* display = XOpenDisplay(NULL); 
        if(display == NULL){
            return ret;
        }
        auto nmonitors = ScreenCount(display);
        ret.reserve(nmonitors);
        
        for(auto i = 0; i < nmonitors; i++) {
            auto screen = ScreenOfDisplay(display, i);
        
            auto name = std::string("Display ") + std::to_string(i);
            ret.push_back(CreateMonitor(
                i, i, screen->height, screen->width, 0, 0, name, 1.0f));
        }     
        XCloseDisplay(display); 
        return ret;
    }
}
}
