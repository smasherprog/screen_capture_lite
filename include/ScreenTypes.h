#pragma once
#include <memory>
#include <functional>

namespace SL {
	namespace Screen_Capture {
		struct CapturedImage {
			std::shared_ptr<char> Data;
			int Height = 0;
			int Width = 0;
			int RelativeTop = 0;
			int RelativeLeft = 0;
			int AbsoluteTop = 0;
			int AbsoluteLeft = 0;
			int ScreenIndex;
			const int PixelStride = 4;//in bytes
		};
		typedef std::function<void(const CapturedImage& img)> ImageCallback;

    }
}