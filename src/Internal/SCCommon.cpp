#include "SCCommon.h"
#include <algorithm>
#include <iostream>
#include <chrono>
#include <assert.h>
#include <string.h>

namespace SL {
    namespace Screen_Capture {

        void SanitizeRects(std::vector<ImageRect>& rects, const Image & img) {
            for (auto& r : rects) {
                if (r.right > Width(img)) {
                    r.right = Width(img);
                }
                if (r.bottom > Height(img)) {
                    r.bottom = Height(img);
                }
            }
        }
#define maxdist 256

        std::vector<ImageRect> GetDifs(const Image & oldimg, const Image & newimg)
        {
            auto start = std::chrono::steady_clock::now();
            std::vector<ImageRect> rects;
            if (Width(oldimg) != Width(newimg) || Height(oldimg) != Height(newimg)) {
                rects.push_back(Rect(newimg));
                return rects;
            }
            rects.reserve((Height(newimg) / maxdist) + 1 * (Width(newimg) / maxdist) + 1);

            auto oldimg_ptr = (const int*)StartSrc(oldimg);
            auto newimg_ptr = (const int*)StartSrc(newimg);

            auto imgwidth = Width(oldimg);
            auto imgheight = Height(oldimg);

            for (decltype(imgheight) row = 0; row < imgheight; row += maxdist) {
                for (decltype(imgwidth) col = 0; col < imgwidth; col += maxdist) {

                    for (decltype(row) y = row; y < maxdist + row && y < imgheight; y++) {
                        for (decltype(col) x = col; x < maxdist + col&& x < imgwidth; x++) {
                            auto old = oldimg_ptr[y*imgwidth + x];
                            auto ne = newimg_ptr[y*imgwidth + x];
                            if (ne != old) {
                                ImageRect re;
                                re.left = col;
                                re.top = row;
                                re.bottom = row + maxdist;
                                re.right = col + maxdist;
                                rects.push_back(re);
                                y += maxdist;
                                x += maxdist;
                            }
                        }
                    }
                }
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

            /*		for (auto& r : rects) {
                        std::cout << r << std::endl;
                    }*/

            if (rects.size() <= 2) {
                SanitizeRects(rects, newimg);
                return rects;//make sure there is at least 2
            }

            std::vector<ImageRect> outrects;
            outrects.reserve(rects.size());
            outrects.push_back(rects[0]);
            //horizontal scan
            int expandcount = 0;
            int containedcount = 0;
            for (size_t i = 1; i < rects.size(); i++) {
                if (outrects.back().right == rects[i].left && outrects.back().bottom == rects[i].bottom) {
                    outrects.back().right = rects[i].right;
                    expandcount++;
                }
                else {
                    outrects.push_back(rects[i]);
                    containedcount++;
                }
            }

            if (expandcount != 0 || containedcount != 0) {
                //std::cout << "On Horizontal Scan expandcount " << expandcount << "  containedcount " << containedcount << std::endl;
            }
            expandcount = 0;
            containedcount = 0;

            if (outrects.size() <= 2) {
                SanitizeRects(outrects, newimg);
                return outrects;//make sure there is at least 2
            }
            rects.resize(0);
            //vertical scan
            for (auto& otrect : outrects) {

                auto found = std::find_if(rects.rbegin(), rects.rend(), [=](const ImageRect& rec) {
                    return rec.bottom == otrect.top && rec.left == otrect.left && rec.right == otrect.right;
                });
                if (found == rects.rend()) {
                    rects.push_back(otrect);
                    containedcount++;
                }
                else {
                    found->bottom = otrect.bottom;
                    expandcount++;
                }
            }
            if (expandcount != 0 || containedcount != 0) {
                //std::cout << "On Horizontal Scan expandcount " << expandcount << "  containedcount " << containedcount << std::endl;
            }
            /*	for (auto& r : rects) {
                    std::cout << r << std::endl;
                }
                */
                //std::cout << "Found " << rects.size() << " rects in img dif. It took " << elapsed.count() << " milliseconds to compare run GetDifs ";

            SanitizeRects(rects, newimg);
            return rects;
        }

        void ProcessMonitorCapture(Monitor_Thread_Data & data, ImageRect & imageract)
        {
            if (data.CaptureEntireMonitor) {
                auto wholeimg = Create(imageract, PixelStride, 0, data.NewImageBuffer.get());
                data.CaptureEntireMonitor(wholeimg, data.SelectedMonitor);
            }
            if (data.CaptureDifMonitor) {
                if (data.FirstRun) {
                    //first time through, just send the whole image
                    auto wholeimgfirst = Create(imageract, PixelStride, 0, data.NewImageBuffer.get());
                    data.CaptureDifMonitor(wholeimgfirst, data.SelectedMonitor);
                    data.FirstRun = false;
                }
                else {
                    //user wants difs, lets do it!
                    auto newimg = Create(imageract, PixelStride, 0, data.NewImageBuffer.get());
                    auto oldimg = Create(imageract, PixelStride, 0, data.OldImageBuffer.get());
                    auto imgdifs = GetDifs(oldimg, newimg);

                    for (auto& r : imgdifs) {
                        auto padding = (r.left *PixelStride) + ((Width(newimg) - r.right)*PixelStride);
                        auto startsrc = data.NewImageBuffer.get();
                        startsrc += (r.left *PixelStride) + (r.top *PixelStride *Width(newimg));

                        auto difimg = Create(r, PixelStride, padding, startsrc);
                        data.CaptureDifMonitor(difimg, data.SelectedMonitor);
                    }
                }
                std::swap(data.NewImageBuffer, data.OldImageBuffer);
            }
        }



        std::shared_ptr<Monitor> CreateMonitor(int index, int id, int h, int w, int ox, int oy, const std::string & n)
        {
            auto ret(std::make_shared<Monitor>());
            ret->Index = index;
            ret->Height = h;
            ret->Id = id;
            assert(n.size() + 1 < sizeof(ret->Name));
            memcpy(ret->Name, n.c_str(), n.size() + 1);
            ret->OffsetX = ox;
            ret->OffsetY = oy;
            ret->Width = w;
            return ret;
        }

        Image Create(const ImageRect& imgrect, int pixelstride, int rowpadding, char* data) {
            Image ret;
            ret.Bounds = imgrect;
            ret.Data = data;
            ret.Pixelstride = pixelstride;
            ret.RowPadding = rowpadding;
            return ret;
        }
        int Index(const Monitor& mointor) { return mointor.Index; }
        int Id(const Monitor& mointor) { return mointor.Id; }
        int OffsetX(const Monitor& mointor) { return mointor.OffsetX; }
        int OffsetY(const Monitor& mointor) { return mointor.OffsetY; }
        const char* Name(const Monitor& mointor) { return mointor.Name; }
        int Height(const Monitor& mointor) { return mointor.Height; }
        int Width(const Monitor& mointor) { return mointor.Width; }
        int Height(const ImageRect& rect) { return rect.bottom - rect.top; }
        int Width(const ImageRect& rect) { return rect.right - rect.left; }
        int Height(const Image& img) { return Height(img.Bounds); }
        int Width(const Image& img) { return Width(img.Bounds); }
        const ImageRect& Rect(const Image& img) { return img.Bounds; }

        //number of bytes per row, NOT including the Rowpadding
        int RowStride(const Image& img) { return img.Pixelstride* Width(img); }
        //number of bytes per row of padding
        int RowPadding(const Image& img) { return img.RowPadding; }
        const char* StartSrc(const Image& img) { return img.Data; }
    }
}