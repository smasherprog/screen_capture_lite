#pragma once
#include <memory>
#include <vector>

namespace SL {
	namespace Screen_Capture {
		//forward declare 
		struct ScreenInfo;

		struct Image {
			std::shared_ptr<char> Data;
			int Height = 0;
			int Width = 0;
		};
		constexpr int get_pixelstride() { return 4; }
		inline int get_imagesize(const Image& img) { return get_pixelstride()*img.Height *img.Width; }

		//this will capture the monitors passed
		Image CaptureDesktopImage(const std::vector<ScreenInfo>& screens);
	}
};
