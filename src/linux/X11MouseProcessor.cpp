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
            std::vector<char> NewImageBuffer, LastImageBuffer;
            int Last_x, Last_y;
        };

        X11MouseProcessor::X11MouseProcessor()
        {
            _X11MouseProcessorImpl = std::make_unique<X11MouseProcessorImpl>();
            _X11MouseProcessorImpl->SelectedDisplay = nullptr;
            _X11MouseProcessorImpl->Last_x = _X11MouseProcessorImpl->Last_y = 0;
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
            
            _X11MouseProcessorImpl->NewImageBuffer.resize(PixelStride*imgrect.right*imgrect.bottom);
            _X11MouseProcessorImpl->LastImageBuffer.resize(PixelStride*imgrect.right*imgrect.bottom);
            memcpy(_X11MouseProcessorImpl->NewImageBuffer.data(), img->pixels, _X11MouseProcessorImpl->NewImageBuffer.size());

                // Get the mouse cursor position
            int x, y, root_x, root_y = 0;
            unsigned int mask = 0;
            Window child_win, root_win;
            XQueryPointer(_X11MouseProcessorImpl->SelectedDisplay, _X11MouseProcessorImpl->RootWindow, &child_win, &root_win, &root_x, &root_y, &x, &y, &mask);
            x -= img->xhot;
            y -= img->yhot;
                
            XFree(img);


            if (_X11MouseProcessorImpl->Data->CaptureCallback) {

                auto wholeimg = Create(imgrect, PixelStride, 0, _X11MouseProcessorImpl->LastImageBuffer.data());
                    
                //if the mouse image is different, send the new image and swap the data 
                if (memcmp(_X11MouseProcessorImpl->NewImageBuffer.data(), _X11MouseProcessorImpl->LastImageBuffer.data(), _X11MouseProcessorImpl->NewImageBuffer.size()) != 0) {
                    _X11MouseProcessorImpl->Data->CaptureCallback(&wholeimg, x, y);
                    _X11MouseProcessorImpl->NewImageBuffer.swap(_X11MouseProcessorImpl->LastImageBuffer);
                }
                else if(_X11MouseProcessorImpl->Last_x != x || _X11MouseProcessorImpl->Last_y != y){
                    _X11MouseProcessorImpl->Data->CaptureCallback(nullptr, x, y);
                }
                _X11MouseProcessorImpl->Last_x = x;
                _X11MouseProcessorImpl->Last_y = y;
            }
            return Ret;
        }

    }
}