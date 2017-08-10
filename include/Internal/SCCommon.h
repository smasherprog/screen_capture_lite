#pragma once
#include "ScreenCapture.h"
#include <thread>
#include <atomic>

// this is INTERNAL DO NOT USE!
namespace SL {
    namespace Screen_Capture {

        template<typename F, typename M, typename W>struct CaptureData {
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
            // Used to indicate a transition event occurred e.g. PnpStop, PnpStart, mode change, TDR, desktop switch and the application needs to recreate the duplication interface
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
            std::unique_ptr<unsigned char[]> OldImageBuffer, NewImageBuffer;
            int ImageBufferSize = 0;
            bool FirstRun = true;
        };

        enum DUPL_RETURN
        {
            DUPL_RETURN_SUCCESS = 0,
            DUPL_RETURN_ERROR_EXPECTED = 1,
            DUPL_RETURN_ERROR_UNEXPECTED = 2
        };
        const int PixelStride = 4;
        Monitor CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string& n);

        Image Create(const ImageRect& imgrect, int pixelstride, int rowpadding, const unsigned char* data);
        //this function will copy data from the src into the dst. The only requirement is that src must not be larger than dst, but it can be smaller
       // void Copy(const Image& dst, const Image& src);

        std::vector<ImageRect> GetDifs(const Image & oldimg, const Image & newimg);

        template<class F, class T, class C>void ProcessCapture(const F& data, T& base, const C& mointor, ImageRect & imageract)
        {
            if (data.OnNewFrame) {
                auto wholeimg = Create(imageract, PixelStride, 0, base.NewImageBuffer.get());
                data.OnNewFrame(wholeimg, mointor);
            }
            if (data.OnFrameChanged) {
                if (base.FirstRun) {
                    //first time through, just send the whole image
                    auto wholeimgfirst = Create(imageract, PixelStride, 0, base.NewImageBuffer.get());
                    data.OnFrameChanged(wholeimgfirst, mointor);
                    base.FirstRun = false;
                }
                else {
                    //user wants difs, lets do it!
                    auto newimg = Create(imageract, PixelStride, 0, base.NewImageBuffer.get());
                    auto oldimg = Create(imageract, PixelStride, 0, base.OldImageBuffer.get());
                    auto imgdifs = GetDifs(oldimg, newimg);

                    for (auto& r : imgdifs) {
                        auto padding = (r.left *PixelStride) + ((Width(newimg) - r.right)*PixelStride);
                        auto startsrc = base.NewImageBuffer.get();
                        startsrc += (r.left *PixelStride) + (r.top *PixelStride *Width(newimg));

                        auto difimg = Create(r, PixelStride, padding, startsrc);
                        data.OnFrameChanged(difimg, mointor);
                    }
                }
                std::swap(base.NewImageBuffer, base.OldImageBuffer);
            }
        }


    }
}
