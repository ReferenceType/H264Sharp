using H264Sharp;
using System.Diagnostics;
using System.Drawing;

namespace CrossPlatformTest
{
    internal class Program
    {
        static void Main(string[] args)
        {
            //Defines.CiscoDllName64bit = "openh264-2.4.0-win64.dll";
            //Defines.CiscoDllName32bit = "openh264-2.4.0-win32.dll";

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            encoder.ConverterNumberOfThreads = 4;
            decoder.ConverterNumberOfThreads = 4;
            decoder.EnableSSEYUVConversion = true;

            decoder.Initialize();


            var w = 1920;
            var h = 1080;
            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraBasic);
            Console.WriteLine("Initialised Encoder");

            Stopwatch sw = Stopwatch.StartNew();
            var bytes = File.ReadAllBytes("RawBgr.bin");
            var data = new ImageData(ImageType.Bgra, 1920, 1080, 1920*4, bytes);

            //Converter converter = new Converter();
            //RgbImage to = new RgbImage(data.Width / 2, data.Height / 2);
            //converter.Downscale(data,to,2);
            //var bb = RgbToBitmap(to);
            //bb.Save("Dowmscaled.bmp");

            RgbImage rgbb = new RgbImage(w, h);
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

                    if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgbb))
                    {
                        //Console.WriteLine($"F:{encoded.FrameType} size: {encoded.Length}");
                        // Bitmap result = RgbToBitmap(rgbb);
                        // result.Save("Ok1.bmp");
                    }

                }


            }
            sw.Stop();
            Console.WriteLine(sw.ElapsedMilliseconds);

            encoder.Dispose();
            decoder.Dispose();
            Console.ReadLine();
        }

        

     
           


    }

}
