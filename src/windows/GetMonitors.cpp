#include "SCCommon.h"
#include "ScreenCapture.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

namespace SL {
namespace Screen_Capture {

    std::vector<Monitor> GetMonitors()
    {
        std::vector<Monitor> ret;
        DISPLAY_DEVICEA dd;
        ZeroMemory(&dd, sizeof(dd));
        dd.cb = sizeof(dd);
        for (auto i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
            // monitor must be attached to desktop and not a mirroring device
            if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) & !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
                DEVMODEA devMode;
                devMode.dmSize = sizeof(devMode);
                EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
                std::string name = dd.DeviceName;
                auto mon = CreateDCA(dd.DeviceName, NULL, NULL, NULL);
                auto xdpi = GetDeviceCaps(mon, LOGPIXELSX);
                DeleteDC(mon);
                auto scale = 1.0f;
                switch (xdpi) {
                case 96:
                    scale = 1.0f;
                    break;
                case 120:
                    scale = 1.25f;
                    break;
                case 144:
                    scale = 1.5f;
                    break;
                case 192:
                    scale = 2.0f;
                    break;
                default:
                    scale = 1.0f;
                    break;
                }
                ret.push_back(
                    CreateMonitor(i, i, devMode.dmPelsHeight, devMode.dmPelsWidth, devMode.dmPosition.x, devMode.dmPosition.y, name, scale));
            }
        }
        return ret;
    }
} // namespace Screen_Capture
} // namespace SL