#include "stdafx.h"
#include "Screen.h"
#include <assert.h>
#include <algorithm>


namespace SL {
	namespace Screen_Capture {

#if __APPLE__
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

		std::shared_ptr<Image> CaptureDesktopImage()
		{
			auto image_ref = CGDisplayCreateImage(CGMainDisplayID());
			auto provider = CGImageGetDataProvider(image_ref);
			auto dataref = CGDataProviderCopyData(provider);
			size_t w, h;
			w = CGImageGetWidth(image_ref);
			h = CGImageGetHeight(image_ref);
			size_t bpp = CGImageGetBitsPerPixel(image_ref) / 8;

			auto img = Image::CreateImage(h, w, (const char*)(CFDataGetBytePtr(dataref)), bpp);

			CFRelease(dataref);
			CFRelease(provider);
			CGImageRelease(image_ref);
			return img;
		}
#endif
	}
}


