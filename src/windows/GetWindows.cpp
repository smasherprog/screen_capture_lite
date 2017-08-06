#include "ScreenCapture.h"
#include "SCCommon.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <algorithm>
#include <locale>

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
            bool found = false;
            if (s->searchby == WindowStringMatch::STARTSWITH) {
                auto f = name.substr(s->Name.size());
                std::transform(f.begin(), f.end(), f.begin(), [](char c) {  return std::tolower(c, std::locale());  });//convert to lower
                found = f == s->Name;
            }
            else if (s->searchby == WindowStringMatch::EXACT) {
                auto f = name; 
                std::transform(f.begin(), f.end(), f.begin(), [](char c) {  return std::tolower(c, std::locale());  });//convert to lower
                found = f == s->Name;
            }
            else if (s->searchby == WindowStringMatch::CONTAINS) {
                auto f = name;
                std::transform(f.begin(), f.end(), f.begin(), [](char c) {  return std::tolower(c, std::locale());  });//convert to lower
                found = f.find(s->Name) != std::string::npos;
            }
            if (found) {
                Window w;
                w.Handle = reinterpret_cast<size_t>(hwnd);
                RECT r;
                GetWindowRect(hwnd, &r);
                w.Height = r.bottom - r.top;
                w.Width = r.right - r.left;
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