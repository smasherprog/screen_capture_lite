#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

	}
}