#include "ScreenCapture.h"
#include "SCCommon.h"
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

                RECT rect = { 0 };
                GetWindowRect(hwnd, &rect);
                ImageRect ret = { 0 };
                ret.bottom = rect.bottom - rect.top;
                ret.right = rect.right - rect.left;


                RECT frame = { 0 };
                RECT border = { 0 };
                if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &frame, sizeof(frame)))) {

                    border.left = frame.left - rect.left;
                    border.top = frame.top - rect.top;
                    border.right = rect.right - frame.right;
                    border.bottom = rect.bottom - frame.bottom;
                }
                ret.bottom -= border.bottom + border.top;
                ret.right -= border.right + border.left;
                w.Height = ret.bottom;
                w.Width = ret.right;
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