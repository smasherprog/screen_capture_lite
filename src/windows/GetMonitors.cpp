#include "ScreenCapture.h"
#include "SCCommon.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>


namespace SL {
	namespace Screen_Capture {

		std::vector<std::shared_ptr<Monitor>> GetMonitors() {
			std::vector<std::shared_ptr<Monitor>> ret;
			DISPLAY_DEVICEA dd;
			ZeroMemory(&dd, sizeof(dd));
			dd.cb = sizeof(dd);
			for (auto i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
				//monitor must be attached to desktop and not a mirroring device
				if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) & !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
					DEVMODEA devMode;
					devMode.dmSize = sizeof(devMode);
					EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
					std::string name = dd.DeviceName;
					ret.push_back(CreateMonitor(i, i, devMode.dmPelsHeight, devMode.dmPelsWidth, devMode.dmPosition.x, devMode.dmPosition.y, name));
				}
			}
			return ret;

		}
	}
}