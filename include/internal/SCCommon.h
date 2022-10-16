#pragma once
#include "ScreenCapture.h"
#include <assert.h>
#include <atomic>
#include <thread>
// this is INTERNAL DO NOT USE!
namespace SL {
namespace Screen_Capture {
    template <typename F, typename M, typename W> struct CaptureData {
#if defined(_WIN32) && defined(__cplusplus) && __cplusplus >= 202002L && !defined(__MINGW32__)
        std::atomic<std::shared_ptr<Timer> > FrameTimer;
#else
        std::shared_ptr<Timer> FrameTimer;
#endif
        F OnNewFrame;
        F OnFrameChanged;
#if defined(_WIN32) && defined(__cplusplus) && __cplusplus >= 202002L && !defined(__MINGW32__)
        std::atomic<std::shared_ptr<Timer> > MouseTimer;
#else
        std::shared_ptr<Timer> MouseTimer;
#endif
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

    class BaseMouseProcessor : public BaseFrameProcessor {
      public:
        std::unique_ptr<unsigned char[]> NewImageBuffer;
        int Last_x = 0;
        int Last_y = 0;
    };

    enum DUPL_RETURN { DUPL_RETURN_SUCCESS = 0, DUPL_RETURN_ERROR_EXPECTED = 1, DUPL_RETURN_ERROR_UNEXPECTED = 2 };
    Monitor CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string &n, float scale);
    Monitor CreateMonitor(int index, int id, int adapter, int h, int w, int ox, int oy, const std::string &n, float scale);
    SC_LITE_EXTERN Image CreateImage(const ImageRect &imgrect, int rowStrideInBytes, const ImageBGRA *data);

    SC_LITE_EXTERN std::vector<ImageRect> GetDifs(const Image &oldimg, const Image &newimg);
    template <class F, class C>
    void ProcessCapture(const F &data, BaseFrameProcessor &base, const C &mointor, const unsigned char *startsrc, int srcrowstride)
    {
        ImageRect imageract;
        imageract.left = 0;
        imageract.top = 0;
        imageract.bottom = Height(mointor);
        imageract.right = Width(mointor);
        const auto sizeofimgbgra = static_cast<int>(sizeof(ImageBGRA));
        const auto startimgsrc = reinterpret_cast<const ImageBGRA *>(startsrc);
        auto dstrowstride = sizeofimgbgra * Width(mointor);
        if (data.OnNewFrame) { // each frame we still let the caller know if asked for
            auto wholeimg = CreateImage(imageract, srcrowstride, startimgsrc);
            wholeimg.isContiguous = dstrowstride == srcrowstride;
            data.OnNewFrame(wholeimg, mointor);
        }
        if (data.OnFrameChanged) { // difs are needed, which means that we must hold the last known image in memory for comparisons...
            if (base.FirstRun) {
                // first time through, just send the whole image
                auto wholeimg = CreateImage(imageract, srcrowstride, startimgsrc);
                wholeimg.isContiguous = dstrowstride == srcrowstride;
                data.OnFrameChanged(wholeimg, mointor);
                base.FirstRun = false;
            }
            else {
                // user wants difs, lets do it!
                auto newimg = CreateImage(imageract, srcrowstride, startimgsrc);
                auto oldimg = CreateImage(imageract, 0, reinterpret_cast<const ImageBGRA *>(base.ImageBuffer.get()));
                auto imgdifs = GetDifs(oldimg, newimg);

                for (auto &r : imgdifs) {
                    auto leftoffset = r.left * sizeofimgbgra;
                    auto thisstartsrc = startsrc + leftoffset + (r.top * srcrowstride);

                    auto difimg = CreateImage(r, srcrowstride, reinterpret_cast<const ImageBGRA *>(thisstartsrc));
                    difimg.isContiguous = false;
                    data.OnFrameChanged(difimg, mointor);
                }
            }
            auto startdst = base.ImageBuffer.get(); 
            assert(base.ImageBufferSize >= dstrowstride * Height(mointor));
            if (dstrowstride == srcrowstride) { // no need for multiple calls, there is no padding here 
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
