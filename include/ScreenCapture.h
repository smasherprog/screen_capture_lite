#pragma once
#include <memory>
#include <functional>
#include <vector>
#include <string>
#include <ostream>
#include <assert.h>
#include <cstring>
#include <chrono>
#include <thread>

namespace SL
{
    namespace Screen_Capture
    {

        struct Monitor
        {
            int Id = INT32_MAX;
            int Index = INT32_MAX;
            int Height = 0;
            int Width = 0;
            // Offsets are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
            // monitors around so this affects their offset.
            int OffsetX = 0;
            int OffsetY = 0;
            char Name[128] = { 0 };
        };

        struct ImageRect
        {
            int left = 0;
            int top = 0;
            int right = 0;
            int bottom = 0;
            bool Contains(const ImageRect& a) const
            {
                return left <= a.left && right >= a.right && top <= a.top && bottom >= a.bottom;
            }
        };
        struct Image
        {
            ImageRect Bounds;
            int Pixelstride = 4;
            int RowPadding = 0;
            // image data is BGRA, for example Data[0] = B, Data[1] =G, Data[2] = R, Data [3]=A
            // alpha is always unused and might contain garbage
            const unsigned char* Data = nullptr;
        };

        inline bool operator==(const ImageRect& a, const ImageRect& b)
        {
            return b.left == a.left && b.right == a.right && b.top == a.top && b.bottom == a.bottom;
        }
        inline std::ostream& operator<<(std::ostream& os, const ImageRect& p)
        {
            return os << "left=" << p.left << " top=" << p.top << " right=" << p.right << " bottom=" << p.bottom;
        }
        inline std::ostream& operator<<(std::ostream& os, const Monitor& p)
        {
            return os << "Id=" << p.Id << " Index=" << p.Index << " Height=" << p.Height << " Width=" << p.Width
                << " OffsetX=" << p.OffsetX << " OffsetY=" << p.OffsetY << " Name=" << p.Name;
        }
        // index to self in the GetMonitors() function
        int Index(const Monitor& mointor);
        // unique identifier
        int Id(const Monitor& mointor);
        int OffsetX(const Monitor& mointor);
        int OffsetY(const Monitor& mointor);
        const char* Name(const Monitor& mointor);
        int Height(const Monitor& mointor);
        int Width(const Monitor& mointor);

        int Height(const ImageRect& rect);
        int Width(const ImageRect& rect);

        int Height(const Image& img);
        int Width(const Image& img);
        const ImageRect& Rect(const Image& img);

        // number of bytes per row, NOT including the Rowpadding
        int RowStride(const Image& img);
        // number of bytes per row of padding
        int RowPadding(const Image& img);
        // the start of the image data, this is not guarenteed to be contiguos. You must use the Rowstride and rowpadding to
        // examine the image
        const unsigned char* StartSrc(const Image& img);

        inline void Extract(const Image& img, unsigned char* dst, size_t dst_size)
        {
            assert(dst_size >= static_cast<size_t>(RowStride(img) * Height(img)));
            auto startdst = dst;
            auto startsrc = StartSrc(img);
            if (RowPadding(img) == 0) { // no padding, the entire copy can be a single memcpy call
                memcpy(startdst, startsrc, RowStride(img) * Height(img));
            }
            else {
                for (auto i = 0; i < Height(img); i++) {
                    memcpy(startdst, startsrc, RowStride(img));
                    startdst += RowStride(img);                   // advance to the next row
                    startsrc += RowStride(img) + RowPadding(img); // advance to the next row
                }
            }
        }

        inline void ExtractAndConvertToRGBA(const Image& img, unsigned char* dst, size_t dst_size)
        {
      
            assert(dst_size >= static_cast<size_t>(RowStride(img) * Height(img)));
            auto imgsrc = StartSrc(img);
            auto imgdist = dst;
            for (auto h = 0; h < Height(img); h++) {
                for (auto w = 0; w < Width(img); w++) {
                    *imgdist++ = *(imgsrc + 2);
                    *imgdist++ = *(imgsrc + 1);
                    *imgdist++ = *(imgsrc);
                    *imgdist++ = 0; // alpha should be zero
                    imgsrc += img.Pixelstride;
                }
                imgsrc += RowPadding(img);
            }
        }
        inline void ExtractAndConvertToRGB(const Image& img, unsigned char* dst, size_t dst_size)
        {
            assert(dst_size >= static_cast<size_t>(Width(img) * 3 * Height(img)));
            auto imgsrc = StartSrc(img);
            auto imgdist = dst;
            for (auto h = 0; h < Height(img); h++) {
                for (auto w = 0; w < Width(img); w++) {
                    *imgdist++ = *(imgsrc + 2);
                    *imgdist++ = *(imgsrc + 1);
                    *imgdist++ = *(imgsrc);
                    imgsrc += img.Pixelstride;
                }
                imgsrc += RowPadding(img);
            }
        }

        inline void ExtractAndConvertToRGB565(const Image& img, unsigned char* dst, size_t dst_size)
        {
            assert(dst_size >= static_cast<size_t>(Width(img) * 2 * Height(img)));
            auto imgsrc = StartSrc(img);
            auto imgdist = dst;
            for (auto h = 0; h < Height(img); h++) {
                for (auto w = 0; w < Width(img); w++) {
                    int short rgb = (*(imgsrc + 2) << 11) | (*(imgsrc + 1) << 5) | *(imgsrc);
                    *imgdist++ = static_cast<unsigned char>(rgb);
                    *imgdist++ = static_cast<unsigned char>(rgb << 8);
                    imgsrc += img.Pixelstride;
                }
                imgsrc += RowPadding(img);
            }
        }
        class ITimer
        {
        public:
            ITimer() {};
            virtual ~ITimer()
            {
            }

            virtual void start() = 0;
            virtual void wait() = 0;
        };
        template <class Rep, class Period> class Timer : public ITimer
        {
            std::chrono::duration<Rep, Period> Rel_Time;
            std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
            std::chrono::time_point<std::chrono::high_resolution_clock> StopTime;

        public:
            Timer(const std::chrono::duration<Rep, Period>& rel_time)
                : Rel_Time(rel_time) {};
            virtual ~Timer()
            {
            }
            virtual void start()
            {
                StartTime = std::chrono::high_resolution_clock::now();
            }
            virtual void wait()
            {
                auto duration = std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(
                    std::chrono::high_resolution_clock::now() - StartTime);
                auto timetowait = Rel_Time - duration;
                if (timetowait.count() > 0) {
                    std::this_thread::sleep_for(timetowait);
                }
            }
        };

        std::vector<Monitor> GetMonitors();
        bool isMonitorInsideBounds(const std::vector<Monitor>& monitors, const Monitor& monitor);

        typedef std::function<void(const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor)>
            CaptureCallback;
        typedef std::function<void(const SL::Screen_Capture::Image* img, int x, int y)> MouseCallback;
        typedef std::function<std::vector<Monitor>()> MonitorCallback;

        class ScreenCaptureManagerImpl;
        class ScreenCaptureManager
        {
            std::shared_ptr<ScreenCaptureManagerImpl> Impl_;

            void setFrameChangeInterval_(const std::shared_ptr<ITimer>& timer);
            void setMouseChangeInterval_(const std::shared_ptr<ITimer>& timer);

        public:
            ScreenCaptureManager(const std::shared_ptr<ScreenCaptureManagerImpl>& impl)
                : Impl_(impl)
            {
            }
            ScreenCaptureManager()
            {
            }

            // Used by the library to determine the callback frequency
            template <class Rep, class Period>
            void setFrameChangeInterval(const std::chrono::duration<Rep, Period>& rel_time)
            {
                setFrameChangeInterval_(std::make_shared<Timer<Rep, Period> >(rel_time));
            }
            // Used by the library to determine the callback frequency
            template <class Rep, class Period>
            void setMouseChangeInterval(const std::chrono::duration<Rep, Period>& rel_time)
            {
                setMouseChangeInterval_(std::make_shared<Timer<Rep, Period> >(rel_time));
            }

            // Will pause all capturing
            void pause();
            // Will return whether the library is paused
            bool isPaused() const;
            // Will resume all capturing if paused, otherwise has no effect
            void resume();

            operator bool() const
            {
                return Impl_.operator bool();
            }
            // the library will stop processing frames and release all memory
            void destroy()
            {
                Impl_.reset();
            }
        };
        class ScreenCaptureConfiguration
        {
            std::shared_ptr<ScreenCaptureManagerImpl> Impl_;

        public:
            ScreenCaptureConfiguration(const std::shared_ptr<ScreenCaptureManagerImpl>& impl)
                : Impl_(impl)
            {
            }
            // When a new frame is available the callback is invoked
            ScreenCaptureConfiguration onNewFrame(const CaptureCallback& cb);
            // When a change in a frame is detected, the callback is invoked
            ScreenCaptureConfiguration onFrameChanged(const CaptureCallback& cb);
            // When a mouse image changes or the mouse changes position, the callback is invoked.
            ScreenCaptureConfiguration onMouseChanged(const MouseCallback& cb);
            // start capturing
            ScreenCaptureManager start_capturing();
        };
        ScreenCaptureConfiguration CreateScreeCapture(const MonitorCallback& monitorstocapture);
    }
}
