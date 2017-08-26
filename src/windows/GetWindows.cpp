#include "GDIHelpers.h"
#include "SCCommon.h"
#include "ScreenCapture.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

namespace SL {
namespace Screen_Capture {

    struct srch {
        std::vector<Window> Found;
    };
    BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
    {
        char buffer[255];
        GetWindowTextA(hwnd, buffer, sizeof(buffer));
        srch *s = (srch *)lParam;
        std::string name = buffer;
        Window w;
        w.Handle = reinterpret_cast<size_t>(hwnd);
        auto windowrect = SL::Screen_Capture::GetWindowRect(hwnd);
        w.Position.x = windowrect.ClientRect.left;
        w.Position.y = windowrect.ClientRect.top;
        w.Size.x = windowrect.ClientRect.right - windowrect.ClientRect.left;
        w.Size.y = windowrect.ClientRect.bottom - windowrect.ClientRect.top;

        memcpy(w.Name, name.c_str(), name.size() + 1);
        s->Found.push_back(w);
        return TRUE;
    }

    std::vector<Window> GetWindows()
    {
        srch s;
        EnumWindows(EnumWindowsProc, (LPARAM)&s);
        return s.Found;
    }

} // namespace Screen_Capture
} // namespace SL