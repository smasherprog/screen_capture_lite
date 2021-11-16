
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;

namespace screen_capture_lite_example_csharp
{
    public class Program
    {
        
        private static void WriteLine(SCL.Monitor p)
        {
            Console.WriteLine($"Id = {p.Id} Index = {p.Index} Height = {p.Height} Width = {p.Width} OffsetX = {p.OffsetX} OffsetY= {p.OffsetY} Name= {p.Name}");
        }

        private static void WriteLine(SCL.Window p)
        {
            Console.WriteLine($"Name = {p.Name} Postion.x = {p.Position.x} Postion.y = {p.Position.y} Size.x = {p.Size.x} Size.y = {p.Size.y}");
        }

        private static void WriteLine(SCL.Image p)
        {
            Console.WriteLine($"BytesToNextRow = {p.BytesToNextRow} isContiguous = {p.isContiguous} Bounds.bottom = {p.Bounds.bottom} Bounds.left = {p.Bounds.left} Bounds.right = {p.Bounds.right} Bounds.top = {p.Bounds.top}");
        }

        static int onNewFramecounter = 0;
        static DateTime onNewFramestart = DateTime.Now;
        
        public static SCL.ScreenCaptureManager SCL_CreateMonitorCaptureManager()
        {

            onNewFramecounter = 0;
            onNewFramestart = DateTime.Now;

            static SCL.Monitor[] SelectMonitors()
            {
                
                var mons = SCL.MonitorCaptureConfiguration.GetMonitors();
                Console.WriteLine("Library is requesting the list of monitors to capture!");
                
                for (int i = 0; i < mons.Length; ++i)
                {
                    WriteLine( mons[i]);
                }
                
                return mons;
                
            }

            var config = new SCL.MonitorCaptureConfiguration(SelectMonitors);

            config.OnNewFrame((image, monitor) =>
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

            });

            config.OnFrameChanged((image, monitor) =>
            {
                //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
                //newBitmap.Save($"{onNewFramecounter++}onFrameChanged.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
                //WriteLine( img);
                //WriteLine( monitor);
            });

            config.OnMouseChanged((image, point) =>
            {
                if (image != null)
                {
                    // var newBitmap = new Bitmap(image.Bounds.right - image.Bounds.left,
                    //     image.Bounds.bottom - image.Bounds.top, image.BytesToNextRow,
                    //     System.Drawing.Imaging.PixelFormat.Format32bppArgb, image.Data);
                    // newBitmap.Save($"{onNewFramecounter++}NewMouseImage.jpg", System.Drawing.Imaging.ImageFormat.Png);
                    // WriteLine(image);
                    Console.WriteLine("New Mouse image");
                }
                else
                {
                    Console.WriteLine("Mouse Moved");
                }
            });

            var screenCaptureManager = new SCL.ScreenCaptureManager(config);
            screenCaptureManager.SetFrameChangeInterval(100);
            screenCaptureManager.SetMouseChangeInterval(100);
            return screenCaptureManager;
            

        }

        public static SCL.ScreenCaptureManager SCL_CreateWindowCaptureManager()
        {
            
            onNewFramecounter = 0;
            onNewFramestart = DateTime.Now;

            static SCL.Window[] SelectWindows()
            {
                
                var wnds = SCL.WindowCaptureConfiguration.GetWindows();
                Console.WriteLine("Library is requesting the list of monitors to capture!");
                
                for (int i = 0; i < wnds.Length; ++i)
                {
                    WriteLine(wnds[i]);
                }
                
                return wnds;

            }

            var config = new SCL.WindowCaptureConfiguration(SelectWindows);

            config.OnNewFrame((image, window) =>
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

            });

            config.OnFrameChanged((image, window) =>
            {
                //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
                //newBitmap.Save($"{onNewFramecounter++}onFrameChanged.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
                //WriteLine( img);
                //WriteLine( monitor);
            });

            config.OnMouseChanged((image, point) =>
            {
                if (image != null)
                {
                    // var newBitmap = new Bitmap(image.Bounds.right - image.Bounds.left,
                    //     image.Bounds.bottom - image.Bounds.top, image.BytesToNextRow,
                    //     System.Drawing.Imaging.PixelFormat.Format32bppArgb, image.Data);
                    // newBitmap.Save($"{onNewFramecounter++}NewMouseImage.jpg", System.Drawing.Imaging.ImageFormat.Png);
                    // WriteLine(image);
                    Console.WriteLine("New Mouse image");
                }
                else
                {
                    Console.WriteLine("Mouse Moved");
                }
            });

            var screenCaptureManager = new SCL.ScreenCaptureManager(config);
            screenCaptureManager.SetFrameChangeInterval(100);
            screenCaptureManager.SetMouseChangeInterval(100);
            return screenCaptureManager;
            

        } 

        public static void Main(string[] args)
        { 
            var startingmemory = GC.GetTotalMemory(false);
            Console.WriteLine($"Starting Capture Demo/Test With {startingmemory.ToString("N0")} bytes of memory used");
            var monitors = SCL.MonitorCaptureConfiguration.GetMonitors();
            foreach(var item in monitors)
            {
                WriteLine(item);
            }

            var windows = SCL.WindowCaptureConfiguration.GetWindows();
            foreach (var item in windows)
            {
                WriteLine(item);
            }

            Console.WriteLine("Running display capturing for 10 seconds");
            using (var scm = SCL_CreateMonitorCaptureManager())
            {
                System.Threading.Thread.Sleep(10 * 1000);
            }

            Console.WriteLine("Running window capturing for 10 seconds");
            using (var scm = SCL_CreateWindowCaptureManager())
            {
                System.Threading.Thread.Sleep(10 * 1000);
                
                Console.WriteLine("Running display capturing for 1 seconds");
                System.Threading.Thread.Sleep(1 * 1000);
                
                Console.WriteLine("Pausing for 10 seconds.");
                scm.IsPaused = true;

                var counti = 0;
                while (counti++ < 10)
                {
                    Debug.Assert(scm.IsPaused);
                    Console.Write(" . ");
                    System.Threading.Thread.Sleep(1 * 1000);
                }

                scm.IsPaused = false;
                Console.WriteLine("Testing changing the interval during runtime for race conditions ");

                var start = DateTime.Now;
                while ((DateTime.Now - start).TotalSeconds < 10)
                {
                    for (var t = 0; t < 100; t++)
                    {
                        scm.SetFrameChangeInterval(100);
                        scm.SetMouseChangeInterval(100);
                    }
                }

                Console.WriteLine("Changing the cpature rate to 1 second");
                scm.SetFrameChangeInterval(1000);
                System.Threading.Thread.Sleep(1 * 1000);

            }

            using (var scm = SCL_CreateMonitorCaptureManager())
            {
                
                Console.WriteLine("Running display capturing for 1 seconds");
                System.Threading.Thread.Sleep(1 * 1000);
                
                Console.WriteLine("Pausing for 10 seconds.");
                scm.IsPaused = true;

                var counti = 0;
                while (counti++ < 10)
                {
                    Debug.Assert(scm.IsPaused);
                    Console.Write(" . ");
                    System.Threading.Thread.Sleep(1 * 1000);
                }

                scm.IsPaused = false;
                Console.WriteLine("Testing changing the interval during runtime for race conditions ");

                var start = DateTime.Now;
                while ((DateTime.Now - start).TotalSeconds < 10)
                {
                    for (var t = 0; t < 100; t++)
                    {
                        scm.SetFrameChangeInterval(100);
                        scm.SetMouseChangeInterval(100);
                    }
                }

                Console.WriteLine("Changing the cpature rate to 1 second");
                scm.SetFrameChangeInterval(1000);
                System.Threading.Thread.Sleep(1 * 1000);

            }

            var counter = 0;
            while (counter++ < 10)
            { 
                Console.WriteLine("Sleeping   .  and Freeing Memory");
                System.Threading.Thread.Sleep(1 * 1000);
                GC.Collect();
            }

            var endingmemory = GC.GetTotalMemory(false);
            var difmemory = endingmemory - startingmemory;
            Console.WriteLine($"Ending Capture Demo/Test With {endingmemory.ToString("N0")} bytes of memory used which is a different of {difmemory.ToString("N0")} bytes");
        }
    }
}
