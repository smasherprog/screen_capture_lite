#include "ScreenCapture.h"
#include "SCCommon.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xinerama.h>

namespace SL
{
namespace Screen_Capture
{

    std::vector<Monitor> GetMonitors()
    {
        std::vector<Monitor> ret;

        Display* display = XOpenDisplay(NULL);
        int nmonitors = 0;
        XineramaScreenInfo* screen = XineramaQueryScreens(display, &nmonitors);
        ret.reserve(nmonitors);
        for(auto i = 0; i < nmonitors; i++) {

            auto name = std::string("Display ") + std::to_string(i);
            ret.push_back(CreateMonitor(
                i, screen[i].screen_number, screen[i].height, screen[i].width, screen[i].x_org, screen[i].y_org, name));
        }
        XFree(screen);

        XCloseDisplay(display);
        return ret;
    }
}
}
