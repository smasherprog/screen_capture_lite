#include "ScreenCapture.h"
#include "SCCommon.h"
#include "GDIHelpers.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <algorithm>
#include <locale>
#include <Dwmapi.h>

namespace SL {
    namespace Screen_Capture {

        struct srch {
            std::string Name;
            WindowStringMatch searchby;
            std::vector<Window> Found;
        };
        BOOL CALLBACK EnumWindowsProc(_In_ HWND   hwnd, _In_ LPARAM lParam) {
            char buffer[255];
            GetWindowTextA(hwnd, buffer, sizeof(buffer));
            srch* s = (srch*)lParam;
            std::string name = buffer;
            std::transform(name.begin(), name.end(), name.begin(), [](char c) {  return std::tolower(c, std::locale());  });//convert to lower
            bool found = false;
            if (s->searchby == WindowStringMatch::STARTSWITH) {
                auto f = name.substr(0, s->Name.size());
                found = f == s->Name;
            }
            else if (s->searchby == WindowStringMatch::EXACT) {
                found = name == s->Name;
            }
            else if (s->searchby == WindowStringMatch::CONTAINS) {
                found = name.find(s->Name) != std::string::npos;
            }
            if (found) {
                Window w;
                w.Handle = reinterpret_cast<size_t>(hwnd);
                auto windowrect = SL::Screen_Capture::GetWindowRect(hwnd);
                w.Position.x = windowrect.ClientRect.left;
                w.Position.y = windowrect.ClientRect.top;
                w.Size.x = windowrect.ClientRect.right - windowrect.ClientRect.left;
                w.Size.y = windowrect.ClientRect.bottom - windowrect.ClientRect.top;

                memcpy(w.Name, name.c_str(), name.size() + 1);
                s->Found.push_back(w);
            }
            return TRUE;
        }


        std::vector<Window> GetWindows(const std::string& name, WindowStringMatch searchby) {
            srch s{ name, searchby };
            std::transform(s.Name.begin(), s.Name.end(), s.Name.begin(), [](char c) {  return std::tolower(c, std::locale());  });//convert to lower
            EnumWindows(EnumWindowsProc, (LPARAM)&s);
            return s.Found;
        }

    }
}