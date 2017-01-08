#include "ScreenCapture.h"
#include "SCCommon.h"
#include <ApplicationServices/ApplicationServices.h>


namespace SL{
    namespace Screen_Capture{
        
        std::vector<std::shared_ptr<Monitor>> GetMonitors() {
            std::vector<std::shared_ptr<Monitor>> ret;
           
typedef struct
{
	short x_org;
	short y_org;
	short width;
	short height;
} FLScreenInfo;
static FLScreenInfo screens[16];
static float dpi[16][2];


std::vector<SL::Screen_Capture::ScreenInfo> SL::Screen_Capture::GetMoitors()
{
	if (!fl_display)
		fl_open_display();
	num_screens = ScreenCount(fl_display);
	if (num_screens > MAX_SCREENS)
		num_screens = MAX_SCREENS;

	for (int i = 0; i < num_screens; i++) {
		screens[i].x_org = 0;
		screens[i].y_org = 0;
		screens[i].width = DisplayWidth(fl_display, i);
		screens[i].height = DisplayHeight(fl_display, i);

		int mm = DisplayWidthMM(fl_display, i);
		dpi[i][0] = mm ? DisplayWidth(fl_display, i) * 25.4f / mm : 0.0f;
		mm = DisplayHeightMM(fl_display, i);
		dpi[i][1] = mm ? DisplayHeight(fl_display, i) * 25.4f / mm : 0.0f;
	}
}


            return ret;

        }
    }
}
