#include "ScreenCapture.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

std::vector<SL::Screen_Capture::Monitor> SL::Screen_Capture::GetMonitors() {
	std::vector<Monitor> ret;
	DISPLAY_DEVICEA dd;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	for (auto i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
		//monitor must be attached to desktop and not a mirroring device
		if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) & !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
			Monitor tmp;
			DEVMODEA devMode;
			devMode.dmSize = sizeof(devMode);
			EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
			tmp.Name = dd.DeviceName;
			tmp.Index = i;
			tmp.Width = devMode.dmPelsWidth;
			tmp.Height = devMode.dmPelsHeight;
			tmp.OffsetX = devMode.dmPosition.x;
			tmp.OffsetY = devMode.dmPosition.y;
			ret.push_back(tmp);
		}
	}
	return ret;

}