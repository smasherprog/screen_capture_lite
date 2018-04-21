#pragma once
#include <assert.h>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#if defined(WINDOWS) || defined(WIN32)
#if defined(SC_LITE_DLL)
#define SC_LITE_EXTERN __declspec(dllexport)
#else
#define SC_LITE_EXTERN
#endif
#else
#define SC_LITE_EXTERN
#endif

namespace SL {
namespace Screen_Capture {
    struct Point {
        int x;
        int y;
    };
    struct Monitor {
        int Id = INT32_MAX;
        int Index = INT32_MAX;
        int Adapter = INT32_MAX;
        int Height = 0;
        int Width = 0;
        // Offsets are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
        // monitors around so this affects their offset.
        int OffsetX = 0;
        int OffsetY = 0;
        char Name[128] = {0};
        float Scaling = 1.0f;
    };

    struct Window {
        size_t Handle;
        Point Position;
        Point Size;
        // Name will always be lower case. It is converted to lower case internally by the library for comparisons
        char Name[128] = {0};
    };
    struct ImageRect {
        int left = 0;
        int top = 0;
        int right = 0;
        int bottom = 0;
        bool Contains(const ImageRect &a) const { return left <= a.left && right >= a.right && top <= a.top && bottom >= a.bottom; }
    };
    struct Image {
        ImageRect Bounds;
        int Pixelstride = 4;
        int RowPadding = 0;
        // image data is BGRA, for example Data[0] = B, Data[1] =G, Data[2] = R, Data [3]=A
        // alpha is always unused and might contain garbage
        const unsigned char *Data = nullptr;
    };

    inline bool operator==(const ImageRect &a, const ImageRect &b)
    {
        return b.left == a.left && b.right == a.right && b.top == a.top && b.bottom == a.bottom;
    }
    // index to self in the GetMonitors() function
    SC_LITE_EXTERN int Index(const Monitor &mointor);
    // unique identifier
    SC_LITE_EXTERN int Id(const Monitor &mointor);
    SC_LITE_EXTERN int Adapter(const Monitor &mointor);
    SC_LITE_EXTERN int OffsetX(const Monitor &mointor);
    SC_LITE_EXTERN int OffsetY(const Monitor &mointor);
    SC_LITE_EXTERN int OffsetX(const Window &mointor);
    SC_LITE_EXTERN int OffsetY(const Window &mointor);
    SC_LITE_EXTERN const char *Name(const Monitor &mointor);
    SC_LITE_EXTERN const char *Name(const Window &mointor);
    SC_LITE_EXTERN int Height(const Monitor &mointor);
    SC_LITE_EXTERN int Width(const Monitor &mointor);
    SC_LITE_EXTERN int Height(const Window &mointor);
    SC_LITE_EXTERN int Width(const Window &mointor);

    SC_LITE_EXTERN int Height(const ImageRect &rect);
    SC_LITE_EXTERN int Width(const ImageRect &rect);

    SC_LITE_EXTERN int Height(const Image &img);
    SC_LITE_EXTERN int Width(const Image &img);
    SC_LITE_EXTERN const ImageRect &Rect(const Image &img);

    // number of bytes per row, NOT including the Rowpadding
    SC_LITE_EXTERN int RowStride(const Image &img);
    // number of bytes per row of padding
    SC_LITE_EXTERN int RowPadding(const Image &img);
    // the start of the image data, this is not guarenteed to be contiguos. You must use the Rowstride and rowpadding to
    // examine the image
    SC_LITE_EXTERN const unsigned char *StartSrc(const Image &img);
    SC_LITE_EXTERN void Extract(const Image &img, unsigned char *dst, size_t dst_size);
    SC_LITE_EXTERN void ExtractAndConvertToRGBA(const Image &img, unsigned char *dst, size_t dst_size);
    SC_LITE_EXTERN void ExtractAndConvertToRGB(const Image &img, unsigned char *dst, size_t dst_size);
    SC_LITE_EXTERN void ExtractAndConvertToRGB565(const Image &img, unsigned char *dst, size_t dst_size);

    class ITimer {
      public:
        ITimer(){};
        virtual ~ITimer() {}
        virtual void start() = 0;
        virtual void wait() = 0;
    };

    class Timer : public ITimer {
        using Clock = std::conditional<
            std::chrono::high_resolution_clock::is_steady,
            std::chrono::high_resolution_clock,
            std::chrono::steady_clock
        >::type;

        std::chrono::microseconds Duration;
        Clock::time_point Deadline;

      public:

        template <typename Rep, typename Period>
        Timer(const std::chrono::duration<Rep, Period> &duration)
            : Duration(std::chrono::duration_cast<std::chrono::microseconds>(duration))
            , Deadline(Clock::now() + Duration)
        {};

        virtual ~Timer() override {}
        virtual void start() override { Deadline = Clock::now() + Duration; }
        virtual void wait() override
        {
            const auto now = Clock::now();
            if (now < Deadline) {
                std::this_thread::sleep_for(Deadline - now);
            }
        }
    };
    // will return all attached monitors
    SC_LITE_EXTERN std::vector<Monitor> GetMonitors();
    // will return all windows
    SC_LITE_EXTERN std::vector<Window> GetWindows();

    SC_LITE_EXTERN bool isMonitorInsideBounds(const std::vector<Monitor> &monitors, const Monitor &monitor);

    typedef std::function<void(const SL::Screen_Capture::Image &img, const Window &window)> WindowCaptureCallback;
    typedef std::function<void(const SL::Screen_Capture::Image &img, const Monitor &monitor)> ScreenCaptureCallback;

    typedef std::function<void(const SL::Screen_Capture::Image *img, const Point &point)> MouseCallback;

    typedef std::function<std::vector<Monitor>()> MonitorCallback;
    typedef std::function<std::vector<Window>()> WindowCallback;

    class SC_LITE_EXTERN IScreenCaptureManager {
      public:
        virtual ~IScreenCaptureManager() {}

        // Used by the library to determine the callback frequency
        template <class Rep, class Period> void setFrameChangeInterval(const std::chrono::duration<Rep, Period> &rel_time)
        {
            setFrameChangeInterval(std::make_shared<Timer>(rel_time));
        }
        // Used by the library to determine the callback frequency
        template <class Rep, class Period> void setMouseChangeInterval(const std::chrono::duration<Rep, Period> &rel_time)
        {
            setMouseChangeInterval(std::make_shared<Timer>(rel_time));
        }

        virtual void setFrameChangeInterval(const std::shared_ptr<ITimer> &timer) = 0;
        virtual void setMouseChangeInterval(const std::shared_ptr<ITimer> &timer) = 0;

        // Will pause all capturing
        virtual void pause() = 0;
        // Will return whether the library is paused
        virtual bool isPaused() const = 0;
        // Will resume all capturing if paused, otherwise has no effect
        virtual void resume() = 0;
    };

    template <typename CAPTURECALLBACK> class ICaptureConfiguration {
      public:
        virtual ~ICaptureConfiguration() {}
        // When a new frame is available the callback is invoked
        virtual std::shared_ptr<ICaptureConfiguration<CAPTURECALLBACK>> onNewFrame(const CAPTURECALLBACK &cb) = 0;
        // When a change in a frame is detected, the callback is invoked
        virtual std::shared_ptr<ICaptureConfiguration<CAPTURECALLBACK>> onFrameChanged(const CAPTURECALLBACK &cb) = 0;
        // When a mouse image changes or the mouse changes position, the callback is invoked.
        virtual std::shared_ptr<ICaptureConfiguration<CAPTURECALLBACK>> onMouseChanged(const MouseCallback &cb) = 0;
        // start capturing
        virtual std::shared_ptr<IScreenCaptureManager> start_capturing() = 0;
    };

    // the callback of windowstocapture represents the list of monitors which should be captured. Users should return the list of monitors they want
    // to be captured
    SC_LITE_EXTERN std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> CreateCaptureConfiguration(const MonitorCallback &monitorstocapture);
    // the callback of windowstocapture represents the list of windows which should be captured. Users should return the list of windows they want to
    // be captured
    SC_LITE_EXTERN std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> CreateCaptureConfiguration(const WindowCallback &windowstocapture);
} // namespace Screen_Capture
} // namespace SL
