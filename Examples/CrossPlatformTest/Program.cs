using H264Sharp;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices;

namespace CrossPlatformTest
{
    internal class Program
    {
        /*
         * Loads a raw rgba and encodes -> decodes. 
         * I publish this for linux and add ncessary .so files on out dir.
         */
        static void Main(string[] args)
        {
            Console.WriteLine(RuntimeInformation.OSDescription);
            Console.WriteLine(RuntimeInformation.ProcessArchitecture);

            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.NumThreads = 4;
            Converter.SetConfig(config);

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            decoder.Initialize();

            var bytes = File.ReadAllBytes("RawBgr.bin");
            var data = new ImageData(ImageType.Bgra, 1920, 1080, 1920 * 4, bytes);
            int w = data.Width;
            int h = data.Height;

            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraBasic);

            RgbImage rgbb = new RgbImage(w, h);
            Stopwatch sw = Stopwatch.StartNew();

            for (int i = 0; i <= 1000; i++) 
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
