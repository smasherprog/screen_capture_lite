using System;
using System.Diagnostics;

namespace screen_capture_lite_example_csharp
{
    public class Program
    {
        private static void WriteLine(SL.Screen_Capture.Monitor p)
        {
            Console.WriteLine($"Id = {p.Id} Index = {p.Index} Height = {p.Height} Width = {p.Width} OffsetX = {p.OffsetX} OffsetY= {p.OffsetY} Name= {p.Name}");
        }

        public static void Main(string[] args)
        {
            Console.WriteLine("Starting Capture Demo/Test"); 
            var goodmonitors = SL.Screen_Capture.NativeFunctions.GetMonitors(); 
            foreach (var m in goodmonitors)
            {
                WriteLine(m);
                Debug.Assert(SL.Screen_Capture.NativeFunctions.isMonitorInsideBounds(goodmonitors, m));
            }
        }
    }
}
