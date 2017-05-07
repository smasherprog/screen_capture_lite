#include "ScreenCapture.h"
#include <iostream> 
#include <chrono> 
#include <atomic>
#include <thread>
#include <string>
#include <vector>


//THESE LIBRARIES ARE HERE FOR CONVINIENCE!! They are SLOW and ONLY USED FOR HOW THE LIBRARY WORKS!
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
    SL::Screen_Capture::ScreenCaptureManager framgrabber;

    framgrabber.onFrameChanged([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
        std::cout << "height  " << Height(img) << "  width  " << Width(img) << std::endl;
        auto r = realcounter.fetch_add(1);
        auto s = std::to_string(r) + std::string(" D") + std::string(".jpg");
        auto size = RowStride(img)*Height(img);

        auto imgbuffer(std::make_unique<char[]>(size));
        //you can extract the image as is in BGRA, or Convert
        //Extract(img, imgbuffer.get(), size);
        ExtractAndConvertToRGBA(img, imgbuffer.get(), size);

        //tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
    });
    std::atomic<int> onNewFramecounter;
    onNewFramecounter = 0;
    auto onNewFramestart = std::chrono::high_resolution_clock::now();
    framgrabber.onNewFrame([&](const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {

        auto r = realcounter.fetch_add(1);
        auto s = std::to_string(r) + std::string(" E") + std::string(".jpg");

        auto size = RowStride(img)*Height(img);
        auto imgbuffer(std::make_unique<char[]>(size));
        //Extract(img, imgbuffer.get(), size);

        ExtractAndConvertToRGBA(img, imgbuffer.get(), size);

        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - onNewFramestart).count() >= 1000) {
            std::cout << "onNewFrame fps" << onNewFramecounter << std::endl;
            onNewFramecounter = 0;
            onNewFramestart = std::chrono::high_resolution_clock::now();
        }
        onNewFramecounter += 1;
        //tje_encode_to_file(s.c_str(), Width(img), Height(img), 4, (const unsigned char*)imgbuffer.get());
    });
    framgrabber.onMouseChanged([&](const SL::Screen_Capture::Image* img, int x, int y) {

        auto r = realcounter.fetch_add(1);
        auto s = std::to_string(r) + std::string(" M") + std::string(".png");
        if (img) {
            //std::cout << "New Mouse Image  x= " << x << " y= " << y << std::endl;
            //lodepng::encode(s, (const unsigned char*)StartSrc(*img), Width(*img), Height(*img));
        }
        else {
            //new coords 
            //std::cout << "x= " << x << " y= " << y << std::endl;
        }

    });

    framgrabber.setMonitorsToCapture([] {
        return SL::Screen_Capture::GetMonitors();//specify which monitors you are interested in
    });

    framgrabber.setFrameChangeInterval(100);//100 ms
    framgrabber.setMouseChangeInterval(100);//100 ms

    framgrabber.Start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    framgrabber.Stop();
    return 0;
}

