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
           
            Converter.EnableNEON = true;
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

            var ss1 = Stopwatch.StartNew();
            Converter.Rgbx2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
            ss1.Stop();

            Console.WriteLine("Conv 1: " + ss1.ElapsedMilliseconds);
            byte[] dat = new byte[w * h * 3];

            unsafe
            {
                fixed (byte* dataPtr = dat)
                    Buffer.MemoryCopy((byte*)rgb.ImageBytes.ToPointer(), dataPtr, dat.Length, dat.Length);
            }
            File.WriteAllBytes("Output.bin", dat);

            Converter.Rgb2Yuv(rgb, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
            unsafe
            {
                fixed (byte* dataPtr = dat)
                    Buffer.MemoryCopy((byte*)rgb.ImageBytes.ToPointer(), dataPtr, dat.Length, dat.Length);
            }
            File.WriteAllBytes("Output1.bin", dat);


            var ss2 = Stopwatch.StartNew();

            for (int i = 0; i < 50; i++)
            {
                bytes = File.ReadAllBytes("RawBgr.bin");
                data = new ImageData(ImageType.Bgra, 1920, 1080, 1920 * 4, bytes);

                ss2.Restart();
                Converter.Rgbx2Yuv(data, yuvImage);
                ss2.Stop();
                Console.WriteLine("Conv1: " + ss2.ElapsedMilliseconds);
                Thread.Sleep(100);

                ss2.Restart();
                Converter.Yuv2Rgb(yuvImage, rgb);
                ss2.Stop();
                Console.WriteLine("Conv2: " + ss2.ElapsedMilliseconds);
                Thread.Sleep(100);


            }


            var ss = Stopwatch.StartNew();
            for (int i = 0; i < 1000; i++)
            {
                Converter.Rgbx2Yuv(data, yuvImage);
                Converter.Yuv2Rgb(yuvImage, rgb);
            }
            ss.Stop();
            Console.WriteLine("Conv: " + ss.ElapsedMilliseconds);

            RgbImage rgbb = new RgbImage(w, h);
            for (int i = 0; i <= 200; i++) 
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
