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
    auto imgsrc = StartSrc(img);
    auto imgdist = dst;
    for (auto h = 0; h < Height(img); h++) {
        auto startimgsrc = imgsrc;
        for (auto w = 0; w < Width(img); w++) {
            *imgdist++ = imgsrc->R;
            *imgdist++ = imgsrc->G;
            *imgdist++ = imgsrc->B;
            *imgdist++ = 255; // alpha 255 = visible
            imgsrc++;
        }
        imgsrc = SL::Screen_Capture::GotoNextRow(img, startimgsrc);
    }
}

extern "C" {
namespace SL {
namespace Screen_Capture {

    SC_LITE_EXTERN int C_GetWindows_Count () {
        std::vector<Window> cpp_windows = SL::Screen_Capture::GetWindows();
        return cpp_windows.size();
    }

    SC_LITE_EXTERN void C_GetWindows_Index_Out (int index, Window* window_out) {
        std::vector<Window> cpp_windows = SL::Screen_Capture::GetWindows();
        if (index < cpp_windows.size()) {
            Window w = cpp_windows[index];
            memcpy(window_out, &w, sizeof *window_out);
        }
    }

    SC_LITE_EXTERN void C_GetWindows (Window** window_out) {
        std::vector<Window> cpp_windows = SL::Screen_Capture::GetWindows();
        return &cpp_windows.data();
        // if (index < cpp_windows.size()) {
        //     Window w = cpp_windows[index];
        //     memcpy(window_out, &w, sizeof *window_out);
        // }
    }

    SC_LITE_EXTERN int C_GetMonitors_Count () {
        std::vector<Monitor> cpp_monitors = SL::Screen_Capture::GetMonitors();
        return cpp_monitors.size();
    }

    SC_LITE_EXTERN void C_GetMonitors_Index_Out (int index, Monitor* monitor_out) {
        std::vector<Monitor> cpp_monitors = SL::Screen_Capture::GetMonitors();
        if (index < cpp_monitors.size()) {
            // monitor_out = cpp_monitors[index];
            Monitor m = cpp_monitors[index];
            memcpy(monitor_out, &m, sizeof *monitor_out);
        }
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
        // frameGrabberInstance->onNewFrame([&](const SL::Screen_Capture::Image& image,const SL::Screen_Capture::Window& window ) {
        //     if (newFrameCallbackInstance != NULL) {
        //         // Testing Callback Types
        //         int w = image.Bounds.right - image.Bounds.left;
        //         int h = image.Bounds.bottom - image.Bounds.top;
        //         pixels = w * h;
        //         size = pixels * sizeof(SL::Screen_Capture::ImageBGRA);
        //         ::fprintf(stdout, "C_ICaptureConfiguration.onNewFrame %i %i %zu\n", w, h, size); fflush(stdout);
        //         auto imgbuffer(std::make_unique<unsigned char[]>(size));
        //         C_ExtractAndConvertToRGBA(image, imgbuffer.get(), size);

        //         // const unsigned  char *retbuffer= (const unsigned  char *)(imgbuffer);
        //         // newFrameCallback(imagek, window);

        //         // unsigned char** dataPtr = NULL;
        //         // *dataPtr = (unsigned char*)imgbuffer.get();
        //         // newFrameCallbackInstance(w*h, dataPtr);

        //         newFrameCallbackInstance(w, h, size, imgbuffer.get());
        //     }
        // });
        // frameGrabberInstance->onMouseChanged([&](const SL::Screen_Capture::Image* image, const SL::Screen_Capture::MousePoint &mousepoint) {
        //     if (mouseChangedCallbackInstance != NULL) {
        //         // Testing Callback Types
        //         int w = image->Bounds.right - image->Bounds.left;
        //         int h = image->Bounds.bottom - image->Bounds.top;
        //         pixels = w * h;
        //         size = pixels * sizeof(SL::Screen_Capture::ImageBGRA);
        //         ::fprintf(stdout, "C_ICaptureConfiguration.onMouseChanged %i %i %zu\n", w, h, size); fflush(stdout);
        //         auto imgbuffer(std::make_unique<unsigned char[]>(size));
        //         C_ExtractAndConvertToRGBA(*image, imgbuffer.get(), size);

        //         // const unsigned  char *retbuffer= (const unsigned  char *)(imgbuffer);
        //         // mouseChangedCallback(image, mousepoint);

        //         // unsigned char** dataPtr = NULL;
        //         // *dataPtr = (unsigned char*)imgbuffer.get();
        //         // mouseChangedCallbackInstance(w*h, dataPtr);

        //         mouseChangedCallbackInstance(w, h, size, imgbuffer.get());
        //     }
        // });
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