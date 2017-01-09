#include "ScreenCapture.h"
#include "SCCommon.h"
#include <X11/Xlib.h>

namespace SL{
    namespace Screen_Capture{
        
        std::vector<std::shared_ptr<Monitor>> GetMonitors() {
            std::vector<std::shared_ptr<Monitor>> ret;
            auto d = XOpenDisplay(0);
            XSync(d, False);
            auto num_screens = ScreenCount(d);
            for (auto i = 0; i < num_screens; i++) {
                auto name = std::string("Display ") + std::to_string(i);
                ret.push_back(CreateMonitor(i, i, DisplayHeight(d, i), DisplayWidth(d, i), 0, 0, name ));
            }
            XCloseDisplay(d);
            return ret;

        }
    }
}
