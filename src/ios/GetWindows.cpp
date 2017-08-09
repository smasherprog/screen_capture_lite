#include "ScreenCapture.h"
#include "SCCommon.h"
#include <algorithm>
#include <string>

namespace SL
{
namespace Screen_Capture
{

    void AddMatch(std::vector<Window>& wnd,
                  const std::string& searchfor,
                  WindowStringMatch searchby)
    {
        std::string name;
    
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
        std::vector<Window> ret;
        return ret;
    }
}
}
