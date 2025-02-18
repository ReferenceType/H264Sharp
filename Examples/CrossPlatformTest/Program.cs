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
            TestCorrectness();
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

        class TestData
        {
            public YuvImage y1, y2, y3, y4;
            public RgbImage r1, r2, r3, r4;

            public TestData(YuvImage y1, YuvImage y2, YuvImage y3, YuvImage y4, RgbImage r1, RgbImage r2, RgbImage r3, RgbImage r4)
            {
                this.y1 = y1;
                this.y2 = y2;
                this.y3 = y3;
                this.y4 = y4;
                this.r1 = r1;
                this.r2 = r2;
                this.r3 = r3;
                this.r4 = r4;
            }

            public bool isEqual(TestData other)
            {
                bool res = true;

                res &= CheckSeq(other.y1.GetBytes(),y1.GetBytes());
                res &= CheckSeq(other.y2.GetBytes(),(y2.GetBytes()));
                res &= CheckSeq(other.y3.GetBytes(),(y3.GetBytes()));
                res &= CheckSeq(other.y4.GetBytes(),(y4.GetBytes()));
                Console.WriteLine("---");
                res &= CheckSeq(other.r1.GetBytes(),(r1.GetBytes()));
                res &= CheckSeq(other.r2.GetBytes(),(r2.GetBytes()));
                res &= CheckSeq(other.r3.GetBytes(),(r3.GetBytes()));
                res &= CheckSeq(other.r4.GetBytes(),(r4.GetBytes()));
                Console.WriteLine("----------");

                return res;
                
            }

            bool CheckSeq(byte[] b1, byte[] b2)
            {
                bool ret = true;
                int max = 0;
                for (int i = 0; i < b1.Length; i++)
                {
                    if (b1[i] != b2[i] && Math.Abs(b1[i] - b2[i])>100)
                        ret =  false;

                    max =Math.Max(
                        max,
                        Math.Min( 
                            Math.Abs(b1[i] - b2[i]),
                            Math.Abs(b2[i] - b1[i])));
                }
                Console.WriteLine(max);
                return ret;
            }
        }

        static void TestCorrectness()
        {
            var config = ConverterConfig.Default;
            config.EnableSSE = 0;
            config.EnableNeon = 0;
            config.EnableAvx2 = 0;
            config.NumThreads = 0;
            config.EnableCustomthreadPool = 1;
            config.EnableDebugPrints = 0;

            var td1 = Test(config);

            config.EnableSSE = 1;
            var td2 = Test(config);

            config.EnableAvx2 = 1;
            var td3 = Test(config);

            config.EnableNeon = 1;
            var td4 = Test(config);

            bool res = true;

            Console.WriteLine("Base vs SSE");
            res &= td1.isEqual(td2);

            Console.WriteLine("Base vs AVX");
            res &= td1.isEqual(td3);

            Console.WriteLine("SSE vs AVX");
            res &= td2.isEqual(td3);

            Console.WriteLine("Base vs Neon");

            res &= td1.isEqual(td4);

            if (res)
                Console.WriteLine("Test Passed!");
            else
                Console.WriteLine("Test Failed!");
        }

        static TestData Test(ConverterConfig config)
        {
            int w = 1920;
            int h = 1080;

            var fi1 = Converter.AllocAllignedNative((w * h)*3);
            var fi2 = Converter.AllocAllignedNative((w * h)*4);


            unsafe
            {
                byte* rgb_ = (byte*)fi1.ToPointer();
                byte* rgba = (byte*)fi2.ToPointer();

                for (int i = 0; i < w * h * 3; i++)
                {
                    rgb_[i] = (byte)(i % 256);
                }

                int jj = 0;
                for (int i = 0; i < w * h * 4; i += 4)
                {
                    rgba[i] =     rgb_[jj++];
                    rgba[i + 1] = rgb_[jj++];
                    rgba[i + 2] = rgb_[jj++];
                    rgba[i + 3] = 0xff;
                }

            }

            byte[] fakeImage3 = new byte[w * h * 3];
            for (int i = 0; i < w * h * 3; i++)
            {
                fakeImage3[i] = (byte)(i % 256);
            }

            byte[] fakeImage4 = new byte[w * h * 4];
            int j = 0;
            for (int i = 0; i < w * h * 4; i += 4)
            {
                fakeImage4[i] = fakeImage3[j++];
                fakeImage4[i + 1] = fakeImage3[j++];
                fakeImage4[i + 2] = fakeImage3[j++];
                fakeImage4[i + 3] = 0xff;
            }

            var bytes = File.ReadAllBytes("RawBgr.bin");
            var dataX = new ImageData(ImageType.Bgra, 1920, 1080, 1920 * 4, bytes);

            Converter.SetConfig(config);

            YuvImage yuvImage = new YuvImage(w, h);
            YuvImage yuvImage1 = new YuvImage(w, h);
            YuvImage yuvImage2 = new YuvImage(w, h);
            YuvImage yuvImage3 = new YuvImage(w, h);

            RgbImage rgb = new RgbImage(w, h);
            RgbImage rgb1 = new RgbImage(w, h);
            RgbImage rgb2 = new RgbImage(w, h);
            RgbImage rgb3 = new RgbImage(w, h);


            //var data = new ImageData(ImageType.Rgb, w, h, w, fakeImage3);
            //var data1 = new ImageData(ImageType.Bgr, w, h, w, fakeImage3);
            //var data2 = new ImageData(ImageType.Rgba, w, h, w, fakeImage4);
            //var data3 = new ImageData(ImageType.Bgra, w, h, w, fakeImage4);

            var data = new ImageData(ImageType.Rgb, w, h, w, fi1);
            var data1 = new ImageData(ImageType.Bgr, w, h, w, fi1);
            var data2 = new ImageData(ImageType.Rgba, w, h, w, fi2);
            var data3 = new ImageData(ImageType.Bgra, w, h, w, fi2);

            Convert(yuvImage, rgb, data);
            Convert(yuvImage1, rgb1, data1);
            Convert(yuvImage2, rgb2, data2);
            Convert(yuvImage3, rgb3, dataX);

            return new TestData(yuvImage, yuvImage1, yuvImage2, yuvImage3,
                                        rgb, rgb1, rgb2, rgb3);

           
        }

        private static void Convert(YuvImage yuvImage, RgbImage rgb, ImageData data)
        {
            Converter.Rgb2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
        }
    }

}
