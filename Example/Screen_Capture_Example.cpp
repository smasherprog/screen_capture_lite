#include "ScreenCapture.h"
#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <string>
#include <vector>

// THESE LIBRARIES ARE HERE FOR CONVINIENCE!! They are SLOW and ONLY USED FOR HOW THE LIBRARY WORKS!
#define TJE_IMPLEMENTATION
#include "tiny_jpeg.h"
#define LODEPNG_COMPILE_PNG
#define LODEPNG_COMPILE_DISK
#include "lodepng.h"
/////////////////////////////////////////////////////////////////////////

int main()
{
    std::cout << "Starting Capture Demo" << std::endl;
    std::atomic<int> realcounter;
    realcounter = 0;
    std::atomic<int> onNewFramecounter;
    onNewFramecounter = 0;
    auto onNewFramestart = std::chrono::high_resolution_clock::now();

    auto framgrabber =
        SL::Screen_Capture::CreateScreeCapture([]() { return SL::Screen_Capture::GetMonitors(); })
    
            .onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {

                auto r = realcounter.fetch_add(1);
                auto s = std::to_string(r) + std::string(" E") + std::string(".jpg");

                auto size = RowStride(img) * Height(img);
                //auto imgbuffer(std::make_unique<char[]>(size));
                // Extract(img, imgbuffer.get(), size);

                //ExtractAndConvertToRGBA(img, imgbuffer.get(), size);

                if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() -
                                                                         onNewFramestart).count() >= 1000) {
                    std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
                    onNewFramecounter = 0;
                    onNewFramestart = std::chrono::high_resolution_clock::now();
                }
                onNewFramecounter += 1;
            })
    
            .start_capturing();

    framgrabber.setFrameChangeInterval(std::chrono::milliseconds(0)); // 100 ms
    framgrabber.setMouseChangeInterval(std::chrono::milliseconds(100)); // 100 ms

    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}
