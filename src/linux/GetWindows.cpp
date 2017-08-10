#include "ScreenCapture.h"
#include "SCCommon.h"
#include <X11/Xlib.h>
#include <algorithm>
#include <string>

namespace SL
{
namespace Screen_Capture
{

    void AddMatch(Display* display,
                  XID& window,
                  std::vector<Window>& wnd,
                  const std::string& searchfor,
                  WindowStringMatch searchby)
    {
        std::string name;
        char* n = NULL;
        if(XFetchName(display, window, &n) > 0) {
            name = n;
            XFree(n);
        }

        std::transform(name.begin(), name.end(), name.begin(), [](char c) {
            return std::tolower(c, std::locale());
        }); // convert to lower
        bool found = false;
        if(searchby == WindowStringMatch::STARTSWITH) {
            auto f = name.substr(0, searchfor.size());
            found = f == searchfor;
        } else if(searchby == WindowStringMatch::EXACT) {
            found = name == searchfor;
        } else if(searchby == WindowStringMatch::CONTAINS) {
            found = name.find(searchfor) != std::string::npos;
        }
        if(found) {
            Window w;
            w.Handle = reinterpret_cast<size_t>(window);
            XWindowAttributes wndattr;
            XGetWindowAttributes(display, window, &wndattr);
            w.Position = Point{ wndattr.x, wndattr.y };
            w.Size = Point{ wndattr.width, wndattr.height };

            memcpy(w.Name, name.c_str(), name.size() + 1);
            wnd.push_back(w);
        }
    }

    std::vector<Window> GetWindows(const std::string& name, WindowStringMatch searchby)
    {
        auto nametofind = name;
        std::transform(nametofind.begin(), nametofind.end(), nametofind.begin(), [](char c) {
            return std::tolower(c, std::locale());
        }); // convert to lower
        auto* display = XOpenDisplay(NULL);
        Atom a = XInternAtom(display, "_NET_CLIENT_LIST", true);
        Atom actualType;
        int format;
        unsigned long numItems, bytesAfter;
        unsigned char* data = 0;
        int status = XGetWindowProperty(display,
                                        XDefaultRootWindow(display),
                                        a,
                                        0L,
                                        (~0L),
                                        false,
                                        AnyPropertyType,
                                        &actualType,
                                        &format,
                                        &numItems,
                                        &bytesAfter,
                                        &data);
        std::vector<Window> ret;
        if(status >= Success && numItems) {
            auto array = (XID*)data;
            for(decltype(numItems) k = 0; k < numItems; k++) {
                auto w = array[k];
                AddMatch(display, w, ret, name, searchby);
            }
            XFree(data);
        }
        XCloseDisplay(display);
        return ret;
    }
}
}