using H264Sharp;
using System.Diagnostics;
using System.Drawing;
using System.Reflection;
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
            Console.WriteLine("OS "+ RuntimeInformation.OSDescription);
            Console.WriteLine("Architecture "+RuntimeInformation.ProcessArchitecture);
            Console.WriteLine();

            var conf = Converter.GetCurrentConfig();
            conf.EnableDebugPrints = 1;
            Converter.SetConfig(conf);

            TestCorrectness();
            TestEncoderCorrectness();

            BenchmarkConverter();
            BenchmarkH264();

            Console.ReadLine();

        }
        public static void ConvertI420ToNV12(IntPtr ImageBytes, int width, int height, IntPtr NV12Buffer)
        {
            int ySize = width * height;
            int uvSize = ySize / 4;

            // Y plane is the same in both formats
            unsafe
            {
                byte* src = (byte*)ImageBytes.ToPointer();
                byte* dst = (byte*)NV12Buffer.ToPointer();

                // Copy Y plane
                for (int i = 0; i < ySize; i++)
                {
                    dst[i] = src[i];
                }

                // Interleave U and V planes into UV plane
                byte* uSrc = src + ySize;
                byte* vSrc = uSrc + uvSize;
                byte* uvDst = dst + ySize;

                for (int i = 0; i < uvSize; i++)
                {
                    uvDst[2 * i] = uSrc[i];     // U component
                    uvDst[2 * i + 1] = vSrc[i]; // V component
                }
            }
        }
        private static void BenchmarkH264()
        {
            Console.WriteLine();
            Console.WriteLine("##### Benchmarking H264");
            int numFrame = 1000;


            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            Converter.SetConfig(config);

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            var bytes = File.ReadAllBytes("RawBgr.bin");
            var data = new ImageData(ImageType.Bgra, 1920, 1080, 1920 * 4, bytes);
            int w = data.Width;
            int h = data.Height;

            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraCaptureAdvancedHP);
            decoder.Initialize();

            //var mem = Marshal.AllocHGlobal(1920 * 1080 + (1920 * 1080) / 2);
            //YUVNV12ImagePointer nv12 = new YUVNV12ImagePointer(mem, 1920, 1080);
            var yuv =  new YuvImage(w,h);
            var yuv2 =  new YuvImage(w,h);
            Converter.Rgb2Yuv(data, yuv);
            ConvertI420ToNV12(yuv.ImageBytes, w, h, yuv2.ImageBytes);
            YUVNV12ImagePointer nv12 = new YUVNV12ImagePointer(yuv2.ImageBytes, 1920, 1080);

            RgbImage rgbb = new RgbImage(w, h);
            Stopwatch sw = Stopwatch.StartNew();
            for (int i = 0; i < numFrame; i++)
            {
                if(!encoder.Encode(nv12, out EncodedData[] ec)) continue;
                foreach (var encoded in ec)
                {
                    decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgbb);
                }
            }
            sw.Stop();

            Console.WriteLine();
            Console.WriteLine($"[Benchmark Result] Encoded/Decoded 1000 frames in {sw.ElapsedMilliseconds} ms:");
            Console.WriteLine($"[Benchmark Result] Throughput: {((numFrame / sw.Elapsed.TotalMilliseconds)* numFrame).ToString("N2") } fps");
            Console.WriteLine();

        }

        private static void BenchmarkConverter()
        {
            Console.WriteLine();
            Console.WriteLine("##### Benchmarking Converter");

            int numFrame = 1000;

            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.EnableAvx512 = 1;
            config.NumThreads = 1;
            Converter.SetConfig(config);

            var bytes = File.ReadAllBytes("RawBgr.bin");
            var data = new ImageData(ImageType.Bgra, 1920, 1080, 1920 * 4, bytes);
            int w = data.Width;
            int h = data.Height;

            YuvImage yuvImage = new YuvImage(w, h);
            RgbImage rgb = new RgbImage(w, h);

            Converter.Rgb2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);

            Stopwatch sw = Stopwatch.StartNew();
            for (int i = 0; i < numFrame; i++)
            {
                Converter.Yuv2Rgb(yuvImage, rgb);
                Converter.Rgb2Yuv(rgb, yuvImage);
            }
            sw.Stop();

            Console.WriteLine($"[Benchmark Result] Converted {numFrame} frames in {sw.ElapsedMilliseconds} ms.");
        }

        private static void TestEncoderCorrectness()
        {
            Console.WriteLine();
            Console.WriteLine("##### Testing Encode/Decode Correctness");

            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            Converter.SetConfig(config);

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            var bytes = File.ReadAllBytes("random.bin");
            var data = new ImageData(ImageType.Bgra, 1920, 1080, 1920 * 4, bytes);
            int w = data.Width;
            int h = data.Height;

            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraBasic);
            decoder.Initialize();

            RgbImage rgbb = new RgbImage(w, h);

            encoder.Encode(data, out EncodedData[] ec);
            foreach (var encoded in ec)
            {

                if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgbb))
                {
                    //var b = rgbb.GetBytes();
                    //File.WriteAllBytes("randomEncoded.bin", b);

                    var encodedIm = rgbb.GetBytes();
                    var knownEncoded = File.ReadAllBytes("randomEncoded.bin");

                    Console.WriteLine();
                    Console.WriteLine("Testing Complete");
                    if (CheckSeq(knownEncoded, encodedIm))
                    {
                        Console.WriteLine("[Test Result] Encode/Decode Test PASS");
                    }
                    else
                    {
                        Console.WriteLine("[Test Result] Encode/Decode Test FAIL!");
                    }

                }

            }
        }

        static void TestCorrectness()
        {
            Console.WriteLine();
            Console.WriteLine("##### Testing Converter Correctness");

            var config = ConverterConfig.Default;
            config.EnableSSE = 0;
            config.EnableNeon = 0;
            config.EnableAvx2 = 0;
            config.NumThreads = 0;
            config.EnableCustomthreadPool = 1;
            config.ForceNaiveConversion = 1;

            var baseline = GenerateData(config);

            config.EnableSSE = 1;
            var sse = GenerateData(config);

            config.EnableAvx2 = 1;
            var avx = GenerateData(config);

            config.EnableNeon = 1;
            var neon = GenerateData(config);

            bool res = true;
            if(RuntimeInformation.ProcessArchitecture == Architecture.X86 ||
                RuntimeInformation.ProcessArchitecture == Architecture.X64)
            {
                Console.WriteLine("# Naive vs SSE");
                res &= baseline.isEqual(sse);

                Console.WriteLine("# Naive vs AVX");
                res &= baseline.isEqual(avx);
            }
            else
            {
                Console.WriteLine("# Naive vs Neon");
                res &= baseline.isEqual(neon);
            }
          
            Console.WriteLine();
            Console.WriteLine("Testing Complete");

            if (res)
                Console.WriteLine("[Test Result] Converter Test Passed!");
            else
                Console.WriteLine("[Test Result] Converter Test Failed!");

            Console.WriteLine();
        }


        static TestData GenerateData(ConverterConfig config)
        {
            Random r = new Random(42);

            int w = 1920;
            int h = 1080;

            //var fi1 = Converter.AllocAllignedNative((w * h) * 3);
            //var fi2 = Converter.AllocAllignedNative((w * h) * 4);
            //unsafe
            //{
            //    byte* rgb_ = (byte*)fi1.ToPointer();
            //    byte* rgba = (byte*)fi2.ToPointer();
            //    for (int i = 0; i < w * h * 3; i++)
            //    {
            //        rgb_[i] = (byte)r.Next(0, 256);
            //        rgb_[i + 1] = (byte)r.Next(0, 256);
            //        rgb_[i + 2] = (byte)r.Next(0, 256);
            //    }
            //    int jj = 0;
            //    for (int i = 0; i < w * h * 4; i += 4)
            //    {
            //        rgba[i] = rgb_[jj++];
            //        rgba[i + 1] = rgb_[jj++];
            //        rgba[i + 2] = rgb_[jj++];
            //        rgba[i + 3] = 0xff;
            //    }
            //}


            //var data = new ImageData(ImageType.Rgb, w, h, w, fi1);
            //var data1 = new ImageData(ImageType.Bgr, w, h, w, fi1);
            //var data2 = new ImageData(ImageType.Rgba, w, h, w, fi2);
            //var data3 = new ImageData(ImageType.Bgra, w, h, w, fi2);


            byte[] randImage3 = new byte[w * h * 3];
            for (int i = 0; i < w * h * 3; i+=3)
            {
                randImage3[i] =   (byte)r.Next(0, 256);
                randImage3[i+1] = (byte)r.Next(0, 256);
                randImage3[i+2] = (byte)r.Next(0, 256);
            }

            byte[] randImage4 = new byte[w * h * 4];
            int j = 0;
            for (int i = 0; i < w * h * 4; i += 4)
            {
                randImage4[i] =     randImage3[j++];
                randImage4[i + 1] = randImage3[j++];
                randImage4[i + 2] = randImage3[j++];
                randImage4[i + 3] = 0xff;
            }

            var bytes = File.ReadAllBytes("RawBgr.bin");
            var srcExtRandom = new ImageData(ImageType.Bgra, 1920, 1080, 1920 * 4, bytes);

            Converter.SetConfig(config);

            YuvImage yuvImage = new YuvImage(w, h);
            YuvImage yuvImage1 = new YuvImage(w, h);
            YuvImage yuvImage2 = new YuvImage(w, h);
            YuvImage yuvImage3 = new YuvImage(w, h);

            var rgb = new ImageData(ImageType.Bgra,w, h);
            var rgb1 = new ImageData(ImageType.Rgba, w, h);
            var rgb2 = new ImageData(ImageType.Bgr, w, h);
            var rgb3 = new ImageData(ImageType.Rgb, w, h);


            var src = new ImageData(ImageType.Rgb, w, h, w, randImage3);
            var src1 = new ImageData(ImageType.Bgr, w, h, w, randImage3);
            var src2 = new ImageData(ImageType.Rgba, w, h, w, randImage4);
            var src3 = new ImageData(ImageType.Bgra, w, h, w, randImage4);


            Convert(src, yuvImage, rgb);
            Convert(src, yuvImage1, rgb1);
            Convert(src2, yuvImage2, rgb2);
            Convert(srcExtRandom, yuvImage3, rgb3);

            return new TestData(yuvImage, yuvImage1, yuvImage2, yuvImage3,
                                        rgb, rgb1, rgb2, rgb3);

           
        }

        private static void Convert(ImageData source,YuvImage yuvImage, ImageData rgb)
        {
            Converter.Rgb2Yuv(source, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
        }

        class TestData
        {
            public YuvImage y1, y2, y3, y4;
            public ImageData r1, r2, r3, r4;

            public TestData(YuvImage y1, YuvImage y2, YuvImage y3, YuvImage y4, ImageData r1, ImageData r2, ImageData r3, ImageData r4)
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
                res &= CheckSeq(y1.GetBytes(), other.y1.GetBytes());
                res &= CheckSeq(y2.GetBytes(), other.y2.GetBytes());
                res &= CheckSeq(y3.GetBytes(), other.y3.GetBytes());
                res &= CheckSeq(y4.GetBytes(), other.y4.GetBytes());
                if (!res) Console.WriteLine("----------");

                res &= CheckSeq(r1.GetBytes(), other.r1.GetBytes());
                res &= CheckSeq(r2.GetBytes(), other.r2.GetBytes());
                res &= CheckSeq(r3.GetBytes(), other.r3.GetBytes());
                res &= CheckSeq(r4.GetBytes(), other.r4.GetBytes());

                if (!res) Console.WriteLine("-------------------");
                if (!res) Console.WriteLine(); else Console.WriteLine(" - PASS");

                return res;

            }
        }
        static bool CheckSeq(byte[] b1, byte[] b2)
        {
            bool ret = true;
            int max = 0;
            for (int i = 0; i < b1.Length; i++)
            {
                if (b1[i] != b2[i] && Math.Abs(b1[i] - b2[i]) > 100)
                {
                    ret = false;
                    max = Math.Max(max, Math.Abs(b1[i] - b2[i]));
                }

            }
            if(!ret)
                Console.WriteLine("max ▲ : " + max);

            return ret;
        }

    }

}
