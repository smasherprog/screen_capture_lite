#pragma once
#include <memory>
#include <functional>

namespace SL {
	namespace Screen_Capture {
		struct Image {
			std::shared_ptr<char> Data;
			int Height = 0;
			int Width = 0;
			int Top = 0;
			int Left = 0;
			const int PixelStride = 4;//in bytes
		};
		enum Captured_Image{ IMAGE_NEW, IMAGE_DIF, IMAGE_PERMISSION_NEEDED, IMAGE_ERROR  };
		typedef std::function<void(const Image& img, Captured_Image type)> ImageCallback;

    }
}