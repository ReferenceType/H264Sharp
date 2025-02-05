using H264Sharp;
using H264SharpBitmapExtentions;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace H264PInvoke
{
#pragma warning disable CA1416 // Validate platform compatibility

    internal class Program
    {
        static unsafe void Main(string[] args)
        {
            //Defines.CiscoDllName64bit = "openh264-2.5.0-win64.dll";
            //Defines.CiscoDllName32bit = "openh264-2.4.0-win32.dll";

            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.NumThreads = 4;
            config.EnableCustomthreadPool = 0;
            Converter.SetConfig(config);

            H264Encoder.EnableDebugPrints = true;
            H264Decoder.EnableDebugPrints = true;

            var img = System.Drawing.Image.FromFile("ocean 1920x1080.jpg");
            //var img = System.Drawing.Image.FromFile("ocean 3840x2160.jpg");

            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);
            Console.WriteLine($"{w}x{h}");

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            decoder.Initialize();
            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraBasic);

            Console.WriteLine("Initialised Encoder");

            var data = bmp.ToImageData();

            RgbImage rgbb = new RgbImage(w, h);
            Stopwatch sw = Stopwatch.StartNew();

            for (int j = 0; j < 1000; j++)
            {

                if (!encoder.Encode(data, out EncodedData[] ec))
                {
                    Console.WriteLine("skipped");
                    continue;
                }

                //encoder.ForceIntraFrame();
                //encoder.SetMaxBitrate(2000000);
                //encoder.SetTargetFps(16.9f);

                foreach (var encoded in ec)
                {
                    bool keyframe = encoded.FrameType == FrameType.I || encoded.FrameType == FrameType.IDR;
                    //encoded.GetBytes();
                    //encoded.CopyTo(buffer,offset);


                    if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, out  rgbb))
                    {
                        //Console.WriteLine($"F:{encoded.FrameType} size: {encoded.Length}");
                        //var result = rgbb.ToBitmap();
                        //result.Save("OUT2.bmp");

                    }

                }
            }
            sw.Stop();
            Console.WriteLine(sw.ElapsedMilliseconds);

            encoder.Dispose();
            decoder.Dispose();
            Console.ReadLine();
        }


        private static void BencmarkConverter()
        {
            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.NumThreads = 4;
            config.EnableCustomthreadPool = 0;
            Converter.SetConfig(config);

            //var img = System.Drawing.Image.FromFile("ocean 3840x2160.jpg");
            var img = System.Drawing.Image.FromFile("ocean 1920x1080.jpg");

            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);


            YuvImage yuvImage = new YuvImage(w, h);
            RgbImage rgb = new RgbImage(w, h);

            var data = bmp.ToImageData();

            Converter.Rgb2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
            rgb.ToBitmap().Save("converted.bmp");

            Stopwatch sw = Stopwatch.StartNew();
            for (int i = 0; i < 10000; i++)
            {
                Converter.Yuv2Rgb(yuvImage, rgb);

                Converter.Rgb2Yuv(rgb, yuvImage);
            }
            Console.WriteLine(sw.ElapsedMilliseconds);

        }

       
    }
}
#pragma warning restore CA1416 // Validate platform compatibility

