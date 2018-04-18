#pragma once
#include "ScreenCapture.h"
#include <atomic>
#include <thread>

// this is INTERNAL DO NOT USE!
namespace SL {
    namespace Screen_Capture {

        template <typename F, typename M, typename W> struct CaptureData {
            std::shared_ptr<ITimer> FrameTimer;
            F OnNewFrame;
            F OnFrameChanged;
            std::shared_ptr<ITimer> MouseTimer;
            M OnMouseChanged;
            W getThingsToWatch;
        };
        struct CommonData {
            // Used to indicate abnormal error condition
            std::atomic<bool> UnexpectedErrorEvent;
            // Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate
            // the duplication interface
            std::atomic<bool> ExpectedErrorEvent;
            // Used to signal to threads to exit
            std::atomic<bool> TerminateThreadsEvent;
            std::atomic<bool> Paused;
        };
        struct Thread_Data {

            CaptureData<ScreenCaptureCallback, MouseCallback, MonitorCallback> ScreenCaptureData;
            CaptureData<WindowCaptureCallback, MouseCallback, WindowCallback> WindowCaptureData;
            CommonData CommonData_;
        };

        class BaseFrameProcessor {
        public:
            std::shared_ptr<Thread_Data> Data;
            std::unique_ptr<unsigned char[]> ImageBuffer;
            int ImageBufferSize = 0;
            bool FirstRun = true;
        };

        enum DUPL_RETURN { DUPL_RETURN_SUCCESS = 0, DUPL_RETURN_ERROR_EXPECTED = 1, DUPL_RETURN_ERROR_UNEXPECTED = 2 };
        const int PixelStride = 4;
        Monitor CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string &n, float scale);
        Monitor CreateMonitor(int index, int id, int adapter, int h, int w, int ox, int oy, const std::string &n, float scale);

        Image Create(const ImageRect &imgrect, int pixelstride, int rowpadding, const unsigned char *data);
        // this function will copy data from the src into the dst. The only requirement is that src must not be larger than dst, but it can be smaller
        // void Copy(const Image& dst, const Image& src);

        std::vector<ImageRect> GetDifs(const Image &oldimg, const Image &newimg);

        template <class F, class T, class C> void ProcessCapture(const F &data, T &base, const C &mointor,
            unsigned char * startsrc,
            int srcrowstride
        ) {
            ImageRect imageract = { 0 };
            imageract.left = 0;
            imageract.top = 0;
            imageract.bottom = Height(mointor);
            imageract.right = Width(mointor); 
            auto dstrowstride = PixelStride * Width(mointor);
            if (data.OnNewFrame) {//each frame we still let the caller know if asked for
                auto wholeimg = Create(imageract, PixelStride, srcrowstride - dstrowstride, startsrc);
                data.OnNewFrame(wholeimg, mointor);
            }
            if (data.OnFrameChanged) {//difs are needed!
                if (base.FirstRun) {
                    // first time through, just send the whole image
                    auto wholeimg = Create(imageract, PixelStride, srcrowstride - dstrowstride, startsrc);
                    data.OnFrameChanged(wholeimg, mointor);
                    base.FirstRun = false;
                }
                else {
                    // user wants difs, lets do it!
                    auto newimg = Create(imageract, PixelStride, srcrowstride - dstrowstride, startsrc);
                    auto oldimg = Create(imageract, PixelStride, 0, base.ImageBuffer.get());
                    auto imgdifs = GetDifs(oldimg, newimg);

                    for (auto &r : imgdifs) {
                        auto leftoffset = r.left * PixelStride;
                        auto rightpadding = (RowStride(newimg) + RowPadding(newimg)) - (r.right * PixelStride);
                        auto padding = leftoffset + rightpadding;
                        auto thisstartsrc = startsrc;
                        thisstartsrc += leftoffset + (r.top * srcrowstride);

                        auto difimg = Create(r, PixelStride, padding, thisstartsrc);
                        data.OnFrameChanged(difimg, mointor);
                    }
                }
                auto startdst = base.ImageBuffer.get();
                if (dstrowstride == static_cast<int>(srcrowstride)) { // no need for multiple calls, there is no padding here
                    memcpy(startdst, startsrc, dstrowstride * Height(mointor));
                }
                else {
                    for (auto i = 0; i < Height(mointor); i++) {
                        memcpy(startdst + (i * dstrowstride), startsrc + (i * srcrowstride), dstrowstride);
                    }
                }
            }
        }
    } // namespace Screen_Capture
} // namespace SL
