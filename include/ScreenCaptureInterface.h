#pragma once
#include <assert.h>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <stdio.h>

#if defined(WINDOWS) || defined(WIN32)
#if defined(SC_LITE_DLL)
#define SC_LITE_EXTERN __declspec(dllexport)
#else
#define SC_LITE_EXTERN
#endif
#else
#define SC_LITE_EXTERN
#endif


void C_ExtractAndConvertToRGBA(const SL::Screen_Capture::Image &img, unsigned char *dst, size_t dst_size) {
    assert(dst_size >= static_cast<size_t>(SL::Screen_Capture::Width(img) * SL::Screen_Capture::Height(img) * sizeof(SL::Screen_Capture::ImageBGRA)));
    auto imgStart = StartSrc(img); // 0
    // auto row = img.BytesToNextRow;
    auto imgWidth = Width(img);
    auto imgHeight = Height(img);

    auto imgsrc = imgStart;
    auto imgdist = dst;
    for (auto h = imgHeight-1; h >= 0 ; h--) {
        imgsrc = (SL::Screen_Capture::ImageBGRA*)(imgStart + h*img.BytesToNextRow/4);
        for (auto w = 0; w < imgWidth; w++) {
            *imgdist++ = imgsrc->R;
            *imgdist++ = imgsrc->G;
            *imgdist++ = imgsrc->B;
            *imgdist++ = 255; // alpha 255 = visible
            imgsrc++;
        }
    }
}

extern "C" {
namespace SL {
namespace Screen_Capture {

    // https://bravenewmethod.com/2017/10/30/unity-c-native-plugin-examples/

    std::vector<Window> cpp_windows_out;  // required, or the memory is released right after running this function
    SC_LITE_EXTERN Window* C_GetWindows (int &size) {
        cpp_windows_out = SL::Screen_Capture::GetWindows();
        size = cpp_windows_out.size();
        return cpp_windows_out.data();
    }

    std::vector<Monitor> cpp_monitor_out;  // required, or the memory is released right after running this function
    SC_LITE_EXTERN Monitor* C_GetMonitors (int &size) {
        cpp_monitor_out = SL::Screen_Capture::GetMonitors();
        size = cpp_monitor_out.size();
        return cpp_monitor_out.data();
    }

    // https://forum.unity.com/threads/returning-a-byte-array-to-c-from-objc.93503/
    SC_LITE_EXTERN typedef void ( *ImageRefWindowRefCallbackType )(const int width, const int height, const int length, unsigned char* imageData);
    SC_LITE_EXTERN typedef void ( *ImagePtrMousePointRefCallbackType )(const int width, const int height, const int length, unsigned char* imageData);

    // int event_cb_register(event_cb_t cb, void *userdata);
    SC_LITE_EXTERN void ImageRefWindowRefCallback( ImageRefWindowRefCallbackType fp );
    SC_LITE_EXTERN void ImagePtrMousePointRefCallback( ImagePtrMousePointRefCallbackType fp );

    static std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> frameGrabberInstance;
    static ImageRefWindowRefCallbackType frameChangedCallbackInstance;
    static ImageRefWindowRefCallbackType newFrameCallbackInstance;
    static ImagePtrMousePointRefCallbackType mouseChangedCallbackInstance;

    static size_t size;
    static size_t pixels;

    // static unsigned char* imgbuffer; //(std::make_unique<unsigned char[]>(size));

    SC_LITE_EXTERN void C_ICaptureConfiguration (
            Window windowToCapture, 
            ImageRefWindowRefCallbackType frameChangedCallback, 
            ImageRefWindowRefCallbackType newFrameCallback, 
            ImagePtrMousePointRefCallbackType mouseChangedCallback) {
        
        frameChangedCallbackInstance = frameChangedCallback;
        newFrameCallbackInstance = newFrameCallback;
        mouseChangedCallbackInstance = mouseChangedCallback;
        
        std::vector<Window> windowsToCapture;
        windowsToCapture.push_back(windowToCapture);
        frameGrabberInstance =  SL::Screen_Capture::CreateCaptureConfiguration([windowsToCapture]() {
            //add your own custom filtering here if you want to capture only some monitors
            return windowsToCapture;
        });

        if (frameChangedCallbackInstance) {
            frameGrabberInstance->onFrameChanged([&](const SL::Screen_Capture::Image& image,const SL::Screen_Capture::Window& window) {
                if (frameChangedCallbackInstance != NULL) {
                    // Testing Callback Types
                    int w = image.Bounds.right - image.Bounds.left;
                    int h = image.Bounds.bottom - image.Bounds.top;
                    pixels = w * h;
                    size = pixels * sizeof(SL::Screen_Capture::ImageBGRA);
                    auto imgbuffer(std::make_unique<unsigned char[]>(size));
                    // ::fprintf(stdout, "C_ICaptureConfiguration.onFrameChanged w:%i h:%i s:%zu  Data: %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n\n", w, h, size, imgbuffer[0], imgbuffer[1], imgbuffer[2], imgbuffer[3], imgbuffer[4], imgbuffer[5], imgbuffer[6], imgbuffer[7]);
                    // ::fflush(stdout);
                    C_ExtractAndConvertToRGBA(image, imgbuffer.get(), size);
                    frameChangedCallbackInstance(w, h, size, imgbuffer.get());
                }
            });
        }
        if (newFrameCallbackInstance) {
            frameGrabberInstance->onNewFrame([&](const SL::Screen_Capture::Image& image,const SL::Screen_Capture::Window& window) {
                if (newFrameCallbackInstance != NULL) {
                    // Testing Callback Types
                    int w = image.Bounds.right - image.Bounds.left;
                    int h = image.Bounds.bottom - image.Bounds.top;
                    pixels = w * h;
                    size = pixels * sizeof(SL::Screen_Capture::ImageBGRA);
                    auto imgbuffer(std::make_unique<unsigned char[]>(size));
                    // ::fprintf(stdout, "C_ICaptureConfiguration.onFrameChanged w:%i h:%i s:%zu  Data: %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n\n", w, h, size, imgbuffer[0], imgbuffer[1], imgbuffer[2], imgbuffer[3], imgbuffer[4], imgbuffer[5], imgbuffer[6], imgbuffer[7]);
                    // ::fflush(stdout);
                    C_ExtractAndConvertToRGBA(image, imgbuffer.get(), size);
                    newFrameCallbackInstance(w, h, size, imgbuffer.get());
                }
            });
        }
        if (mouseChangedCallbackInstance) {
            frameGrabberInstance->onMouseChanged([&](const SL::Screen_Capture::Image* image,const SL::Screen_Capture::MousePoint& mousepoint) {
                if (mouseChangedCallbackInstance != NULL) {
                    // currently only callback when mousepos and image is received
                    if (image) {
                        // Testing Callback Types
                        int w = image->Bounds.right - image->Bounds.left;
                        int h = image->Bounds.bottom - image->Bounds.top;
                        pixels = w * h;
                        size = pixels * sizeof(SL::Screen_Capture::ImageBGRA);
                        auto imgbuffer(std::make_unique<unsigned char[]>(size));
                        // ::fprintf(stdout, "C_ICaptureConfiguration.onFrameChanged w:%i h:%i s:%zu  Data: %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n\n", w, h, size, imgbuffer[0], imgbuffer[1], imgbuffer[2], imgbuffer[3], imgbuffer[4], imgbuffer[5], imgbuffer[6], imgbuffer[7]);
                        // ::fflush(stdout);
                        C_ExtractAndConvertToRGBA(*image, imgbuffer.get(), size);
                        mouseChangedCallbackInstance(w, h, size, imgbuffer.get());
                    }
                }
            });
        }
        frameGrabberInstance->start_capturing();
        // return framgrabber;
    }

    SC_LITE_EXTERN void C_Capture_Start () {
        frameGrabberInstance->start_capturing();
    }

    SC_LITE_EXTERN void C_Capture_Stop () {
        frameGrabberInstance.reset();
    }

} // namespace Screen_Capture
} // namespace SL
}