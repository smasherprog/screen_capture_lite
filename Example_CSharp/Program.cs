using SL;
using System;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Threading;

namespace screen_capture_lite_example_csharp
{
    public class Program
    {
        private static void WriteLine( SL.Screen_Capture.Monitor p)
        {
            Console.WriteLine($"Id = {p.Id} Index = {p.Index} Height = {p.Height} Width = {p.Width} OffsetX = {p.OffsetX} OffsetY= {p.OffsetY} Name= {p.Name}");
        }
        private static void WriteLine( SL.Screen_Capture.Window p)
        {
            Console.WriteLine($"Name = {p.Name} Postion.x = {p.Position.x} Postion.y = {p.Position.y} Size.x = {p.Size.x} Size.y = {p.Size.y}");
        }
        private static void WriteLine( SL.Screen_Capture.Image p)
        {
            Console.WriteLine($"BytesToNextRow = {p.BytesToNextRow} isContiguous = {p.isContiguous} Bounds.bottom = {p.Bounds.bottom} Bounds.left = {p.Bounds.left} Bounds.right = {p.Bounds.right} Bounds.top = {p.Bounds.top}");
        }
        static int onNewFramecounter = 0;
        static DateTime onNewFramestart = DateTime.Now;
        public static SL.Screen_Capture.ScreenCaptureManager createframegrabber()
        {
            onNewFramecounter = 0;
            onNewFramestart = DateTime.Now;
            var framgrabber = SL.Screen_Capture.CaptureConfiguration.CreateCaptureConfiguration(() =>
           {
               var mons = SL.Screen_Capture.GetMonitors();
               Console.WriteLine("Library is requesting the list of monitors to capture!");
               for (int i = 0; i < mons.Length; ++i)
               {
                   WriteLine( mons[i]);
               }
               return mons;
           }).onNewFrame(( SL.Screen_Capture.Image img,  SL.Screen_Capture.Monitor monitor) =>
           {
               //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
               //newBitmap.Save($"{onNewFramecounter++}onNewFrame.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
               //WriteLine( img);
               //WriteLine( monitor); 
               if ((DateTime.Now - onNewFramestart).TotalMilliseconds > 1000)
               {
                   Console.WriteLine("onNewFrame fps" + onNewFramecounter);
                   onNewFramestart = DateTime.Now;
                   onNewFramecounter = 0;
               }
               Interlocked.Increment(ref onNewFramecounter);

           }).onFrameChanged(( SL.Screen_Capture.Image img,  SL.Screen_Capture.Monitor monitor) =>
           {
               //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
               //newBitmap.Save($"{onNewFramecounter++}onFrameChanged.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
               //WriteLine( img);
               //WriteLine( monitor); 
           }).onMouseChanged((SL.Screen_Capture.Image img, SL.Screen_Capture.MousePoint mousePoint) =>
           { 
               if (img != null)
               {
                   var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppArgb, img.Data);
                   newBitmap.Save($"{onNewFramecounter++}NewMouseImage.jpg", System.Drawing.Imaging.ImageFormat.Png);
                   WriteLine(img);
                   Console.WriteLine("New Mouse image");
               }
               else
               { 
                   Console.WriteLine("Mouse Moved"); 
               }

           }).start_capturing();
            framgrabber.setFrameChangeInterval(100);
            framgrabber.setMouseChangeInterval(100);
            return framgrabber;
        }

        public static SL.Screen_Capture.ScreenCaptureManager createwindowgrabber()
        {
            onNewFramecounter = 0;
            onNewFramestart = DateTime.Now;
            var framgrabber = SL.Screen_Capture.CaptureConfiguration.CreateCaptureConfiguration(() =>
            {
                var windows = SL.Screen_Capture.GetWindows();
                Console.WriteLine("Library is requesting the list of windows to capture!");
                for (int i = 0; i < windows.Length; ++i)
                {
                    WriteLine(windows[i]);
                }
                return windows.Where(a => a.Name.ToLower().Contains("google")).ToArray();
            }).onNewFrame(( SL.Screen_Capture.Image img,  SL.Screen_Capture.Window monitor) =>
            {
                //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
                //newBitmap.Save($"{onNewFramecounter++}onNewFrame.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
                //WriteLine( img);
                //WriteLine( monitor); 
                if ((DateTime.Now - onNewFramestart).TotalMilliseconds > 1000)
                {
                    Console.WriteLine("onNewFrame fps" + onNewFramecounter);
                    onNewFramestart = DateTime.Now;
                    onNewFramecounter = 0;
                }
                Interlocked.Increment(ref onNewFramecounter);

            }).onFrameChanged(( SL.Screen_Capture.Image img,  SL.Screen_Capture.Window monitor) =>
            {
                //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
                //newBitmap.Save($"{onNewFramecounter++}onFrameChanged.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
                //WriteLine( img);
                //WriteLine( monitor); 
            }).onMouseChanged(( SL.Screen_Capture.Image img,  SL.Screen_Capture.MousePoint mousePoint) =>
            {
                 
                if (img != null)
                {
                    Console.WriteLine("onNewFrame fps" + onNewFramecounter);
                }
                else
                {
                    Console.WriteLine("onNewFrame fps" + onNewFramecounter);
                }

            }).start_capturing();

            framgrabber.setFrameChangeInterval(100);
            framgrabbeextern.setMouseChangeInterval(100);
            return framgrabber;
        }

        public static void Main(string[] args)
        {
            Console.WriteLine("Starting Capture Demo/Test");
            var monitors = SL.Screen_Capture.SCL_GetMonitors();
            foreach(var item in monitors)
            {
                WriteLine(item); 
            }

            var windows = SL.Screen_Capture.SCL_GetWindows();
            foreach (var item in windows)
            {
                WriteLine(item);
            }

            Console.WriteLine("Running display capturing for 10 seconds");
            using (var framgrabber = createframegrabber())
            {
                System.Threading.Thread.Sleep(10 * 1000);
            }

            Console.WriteLine("Running window capturing for 10 seconds");
            using (var framgrabber = createwindowgrabber())
            {
                System.Threading.Thread.Sleep(10 * 1000);
            }

            using (var framgrabber = createframegrabber())
            {
                Console.WriteLine("Running display capturing for 1 seconds");
                System.Threading.Thread.Sleep(1 * 1000);
                Console.WriteLine("Pausing for 10 seconds.");
                framgrabber.pause();
                var counti = 0;
                while (counti++ < 10)
                {
                    Debug.Assert(framgrabber.SCL_IsPaused());
                    Console.Write(" . ");
                    System.Threading.Thread.Sleep(1 * 1000);
                }

                framgrabber.SCL_Resume();
                Console.WriteLine("Testing changing the interval during runtime for race conditions ");

                var start = DateTime.Now;
                while ((DateTime.Now - start).TotalSeconds < 10)
                {
                    for (var t = 0; t < 100; t++)
                    {
                        framgrabber.setFrameChangeInterval(100);
                        framgrabber.SCL_SetMouseChangeInterval(100);
                    }
                }

                Console.WriteLine("Changing the cpature rate to 1 second");
                framgrabber.SCL_SetFrameChangeInterval(1000);
                System.Threading.Thread.Sleep(1 * 1000);
            }
        }
    }
}
