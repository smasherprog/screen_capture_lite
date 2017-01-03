#pragma once
#include "ScreenCapture.h"
#include "ThreadRunner.h"

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
			DUPL_RETURN Init(std::shared_ptr<THREAD_DATA> data);
			DUPL_RETURN ProcessFrame();

		private:

			HDCWrapper MonitorDC;
			HDCWrapper CaptureDC;
			HBITMAPWrapper CaptureBMP;
			std::shared_ptr<THREAD_DATA> Data;
			std::unique_ptr<char[]> ImageBuffer;
			size_t ImageBufferSize;
		};
	}
}