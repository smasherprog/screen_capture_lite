#include "stdafx.h"
#include "ScreenInfo.h"
#include <algorithm>

#if _WIN32
std::vector<SL::Screen_Capture::ScreenInfo> SL::Screen_Capture::GetMoitors()
{
	std::vector<SL::Screen_Capture::ScreenInfo> ret;
	DISPLAY_DEVICEA dd;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);
	for (auto i = 0; EnumDisplayDevicesA(NULL, i, &dd, 0); i++) {
		//monitor must be attached to desktop and not a mirroring device
		if ((dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) & !(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {
			SL::Screen_Capture::ScreenInfo temp;
			DEVMODEA devMode;
			devMode.dmSize = sizeof(devMode);
			EnumDisplaySettingsA(dd.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
			strncpy_s(temp.Device, dd.DeviceName, ARRAYSIZE(dd.DeviceName));
			temp.Offsetx = devMode.dmPosition.x;
			temp.Offsety = devMode.dmPosition.y;
			temp.Width = devMode.dmPelsWidth;
			temp.Height = devMode.dmPelsHeight;
			temp.Depth = devMode.dmBitsPerPel;
			ret.push_back(temp);
		}
	}
	//always reorder the screens left to right... as it should be, right?
	Reorder(ret);
	return ret;
}

void SL::Screen_Capture::Reorder(std::vector<SL::Screen_Capture::ScreenInfo>& screens)
{
	//organize the monitors so that the ordering is left to right for displaying purposes
	std::sort(begin(screens), end(screens), [](const SL::Screen_Capture::ScreenInfo& i, const SL::Screen_Capture::ScreenInfo& j) { return i.Offsetx < j.Offsetx; });
	auto index = 0;
	for (auto& x : screens) x.Index = index++;
}


#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#   error "Unknown Apple platform"
#endif


#include <ApplicationServices/ApplicationServices.h>

std::vector<SL::Screen_Capture::ScreenInfo> SL::Screen_Capture::GetMoitors()
{
	static XRectangle screens[16];
	static float dpi_h[16];
	static float dpi_v[16];
	CGDirectDisplayID displays[16];
	CGDisplayCount count, i;
	CGRect r;
	CGGetActiveDisplayList(16, displays, &count);
	for (i = 0; i < count; i++) {
		r = CGDisplayBounds(displays[i]);
		screens[i].x = int(r.origin.x);
		screens[i].y = int(r.origin.y);
		screens[i].width = int(r.size.width);
		screens[i].height = int(r.size.height);
		//fprintf(stderr,"screen %d %dx%dx%dx%d\n",i,screens[i].x,screens[i].y,screens[i].width,screens[i].height);
		if (CGDisplayScreenSize != NULL) {
			CGSize s = CGDisplayScreenSize(displays[i]); // from 10.3
			dpi_h[i] = screens[i].width / (s.width / 25.4);
			dpi_v[i] = screens[i].height / (s.height / 25.4);
		}
		else {
			dpi_h[i] = dpi_v[i] = 75.;
		}
	}
}

#elif __ANDROID__
std::vector<SL::Screen_Capture::ScreenInfo> SL::Screen_Capture::GetMoitors()
{
	return Image::CreateImage(0, 0);
}
#elif __linux__


std::vector<SL::Screen_Capture::ScreenInfo> SL::Screen_Capture::GetMoitors()
{
	num_screens = ScreenCount(fl_display);
	if (num_screens > MAX_SCREENS) num_screens = MAX_SCREENS;

	for (int i = 0; i < num_screens; i++) {
		screens[i].x_org = 0;
		screens[i].y_org = 0;
		screens[i].width = DisplayWidth(fl_display, i);
		screens[i].height = DisplayHeight(fl_display, i);

		int mm = DisplayWidthMM(fl_display, i);
		dpi[i][0] = mm ? DisplayWidth(fl_display, i)*25.4f / mm : 0.0f;
		mm = DisplayHeightMM(fl_display, i);
		dpi[i][1] = mm ? DisplayHeight(fl_display, i)*25.4f / mm : 0.0f;
	}

}
#endif


