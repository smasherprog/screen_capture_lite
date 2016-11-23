#include "stdafx.h"
#include "Screen.h"
#include <assert.h>
#include <algorithm>


namespace SL {
	namespace Screen_Capture {

#if __linux__

		void Save(const Image & img, std::string path)
		{

		}

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

		std::shared_ptr<Image> CaptureDesktopImage()
		{
			auto display = XOpenDisplay(NULL);
			auto root = DefaultRootWindow(display);
			auto screen = XDefaultScreen(display);
			auto visual = DefaultVisual(display, screen);
			auto depth = DefaultDepth(display, screen);

			XWindowAttributes gwa;
			XGetWindowAttributes(display, root, &gwa);
			auto width = gwa.width;
			auto height = gwa.height;

			XShmSegmentInfo shminfo;
			auto image = XShmCreateImage(display, visual, depth, ZPixmap, NULL, &shminfo, width, height);
			shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);

			shminfo.readOnly = False;
			shminfo.shmaddr = image->data = (char*)shmat(shminfo.shmid, 0, 0);

			XShmAttach(display, &shminfo);

			XShmGetImage(display, root, image, 0, 0, AllPlanes);

			XShmDetach(display, &shminfo);

			auto px = Image::CreateImage(height, width, (char*)shminfo.shmaddr, image->bits_per_pixel / 8);
			assert(image->bits_per_pixel == 32);//this should always be true... Ill write a case where it isnt, but for now it should be

			XDestroyImage(image);
			shmdt(shminfo.shmaddr);
			shmctl(shminfo.shmid, IPC_RMID, 0);
			XCloseDisplay(display);

			return px;

	}
	

#endif
	}
}


