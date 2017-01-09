#include "X11MouseProcessor.h"

#include <X11/X.h>
#include <X11/extensions/Xfixes.h>
#include <assert.h>
#include <cstring>

namespace SL {
	namespace Screen_Capture {
		
		struct X11MouseProcessorImpl {

			std::shared_ptr<Mouse_Thread_Data> Data;
			Display* SelectedDisplay;
			Window RootWindow;
		};

		X11MouseProcessor::X11MouseProcessor()
		{
			_X11MouseProcessorImpl = std::make_unique<X11MouseProcessorImpl>();
			_X11MouseProcessorImpl->SelectedDisplay = nullptr;
		}

		X11MouseProcessor::~X11MouseProcessor()
		{
			if (_X11MouseProcessorImpl->SelectedDisplay) {
				XCloseDisplay(_X11MouseProcessorImpl->SelectedDisplay);
			}
		}
		DUPL_RETURN X11MouseProcessor::Init(std::shared_ptr<Mouse_Thread_Data> data) {
			auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
			_X11MouseProcessorImpl->Data = data;
			_X11MouseProcessorImpl->SelectedDisplay = XOpenDisplay(NULL);
			if (!_X11MouseProcessorImpl->SelectedDisplay) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}
			_X11MouseProcessorImpl->RootWindow = DefaultRootWindow(_X11MouseProcessorImpl->SelectedDisplay);
			if (!_X11MouseProcessorImpl->RootWindow) {
				return DUPL_RETURN::DUPL_RETURN_ERROR_EXPECTED;
			}
			return ret;
		}
		//
		// Process a given frame and its metadata
		//
		DUPL_RETURN X11MouseProcessor::ProcessFrame()
		{
			auto Ret = DUPL_RETURN_SUCCESS;
	
            auto img = XFixesGetCursorImage(_X11MouseProcessorImpl->SelectedDisplay);
            auto pixels = (int *)img->pixels;
            if (sizeof(img->pixels[0]) == 8) {//if the pixelstride is 64 bits.. scale down to 32bits
                auto pixels = (int *)img->pixels;
                for (auto i = 0; i < img->width * img->height; ++i) {
                    pixels[i] = pixels[i * 2];
                }
            }
            ImageRect imgrect;
			imgrect.left = imgrect.top = 0;
			imgrect.right = img->width;
			imgrect.bottom = img->height;
            auto imgdata = std::make_unique<char[]>(PixelStride*imgrect.right*imgrect.bottom);
        	memcpy(imgdata.get(), img->pixels, PixelStride*imgrect.right*imgrect.bottom);

				// Get the mouse cursor position
            int x, y, root_x, root_y = 0;
            unsigned int mask = 0;
            Window child_win, root_win;
            XQueryPointer(_X11MouseProcessorImpl->SelectedDisplay, _X11MouseProcessorImpl->RootWindow, &child_win, &root_win, &root_x, &root_y, &x, &y, &mask);
            x -= img->xhot;
            y -= img->yhot;
                
            XFree(img);


			if (_X11MouseProcessorImpl->Data->CaptureCallback) {

				auto img = CreateImage(imgrect, PixelStride, 0, imgdata.get());
				_X11MouseProcessorImpl->Data->CaptureCallback(*img, x, y);
			}
			return Ret;
		}

	}
}