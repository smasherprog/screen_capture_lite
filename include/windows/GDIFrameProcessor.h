#pragma once
#include "ScreenCapture.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>

namespace SL {
	namespace Screen_Capture {
		class HDCWrapper {
		public:
			HDCWrapper() : DC(nullptr) {}
			~HDCWrapper() { if (DC != nullptr) { DeleteDC(DC); } }
			HDC DC;
		};
		class HBITMAPWrapper {
		public:
			HBITMAPWrapper() : Bitmap(nullptr) {}
			~HBITMAPWrapper() { if (Bitmap != nullptr) { DeleteObject(Bitmap); } }
			HBITMAP Bitmap;
		};
		class GDIFrameProcessor {
		public:
			GDIFrameProcessor();
			~GDIFrameProcessor();
			DUPL_RETURN Init(ImageCallback& cb, Monitor monitor);
			DUPL_RETURN ProcessFrame();

		private:

			HDCWrapper MonitorDC;
			HDCWrapper CaptureDC;
			HBITMAPWrapper CaptureBMP;
            

			ImageCallback CallBack;
			Monitor CurrentMonitor;
		};
	}
}