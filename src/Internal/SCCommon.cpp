#include "SCCommon.h"
#include <algorithm>
#include <iostream>
#include <chrono>

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
	}
}