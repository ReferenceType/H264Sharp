using H264Sharp;
using System.Diagnostics;
using System.Drawing;

namespace CrossPlatformTest
{
    internal class Program
    {
        //TODO test on linux with the opt none removal

        /*
         * Loads a raw rgba and encodes -> decodes. 
         * I publish this for linux and add ncessary .so files on out dir.
         */
        static void Main(string[] args)
        {
            H264Encoder.EnableDebugPrints = true;
            H264Decoder.EnableDebugPrints = true;
            Converter.EnableNEON = false;
            Converter.NumThreads = 1;

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

           

            decoder.Initialize();

            var w = 1920;
            var h = 1080;
            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraCaptureAdvanced);
            Console.WriteLine("Initialised Encoder");

            Stopwatch sw = Stopwatch.StartNew();
            var bytes = File.ReadAllBytes("RawBgr.bin");
            var data = new ImageData(ImageType.Bgra, 1920, 1080, 1920*4, bytes);

            //Converter.EnableNEON = false;

            YuvImage yuvImage = new YuvImage(w, h);
            RgbImage rgb = new RgbImage(w, h);
            Converter.Rgbx2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
            byte[] dat = new byte[w * h * 3];

            unsafe
            {
                fixed (byte* dataPtr = dat)
                    Buffer.MemoryCopy((byte*)rgb.ImageBytes.ToPointer(), dataPtr, dat.Length, dat.Length);
            }
            File.WriteAllBytes("Output.bin", dat);
           


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
