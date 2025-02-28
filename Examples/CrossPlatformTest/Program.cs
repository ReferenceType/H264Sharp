using H264Sharp;
using System;
using System.Diagnostics;
using System.Drawing;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;

namespace CrossPlatformTest
{
    internal class Program
    {
        class Config
        {
            public int NumIterations { get; set; } = 1000;
            public int EnableCustomThreadPool { get; set; } = 1;
            public int Numthreads { get; set; } = 1;
            public int EnableSSE { get; set; } = 1;
            public int EnableAvx2 { get; set; } = 1;

            public void Print()
            {
                Console.WriteLine($"NumIterations: {NumIterations}");
                Console.WriteLine($"EnableCustomThreadPool: {EnableCustomThreadPool}");
                Console.WriteLine($"Numthreads: {Numthreads}");
                Console.WriteLine($"EnableSSE: {EnableSSE}");
                Console.WriteLine($"EnableAvx2: {EnableAvx2}");
                Console.WriteLine();
            }
        }
        /*
         * Loads a raw rgba and encodes -> decodes. 
         * I publish this for linux and add ncessary .so files on out dir.
         */
        static int th = 16;
        static void Main(string[] args)
        {
            Config config = JsonSerializer.Deserialize<Config>(System.IO.File.ReadAllText("config.json"))!;
            if (config == null)
                config = new Config();
           // th = config.Numthreads;
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
            BenchmarkH264v2();

            Console.ReadLine();

        }
        static void BenchmarkH264v2()
        {
            int numFrame = 1000;

            int w = 0;
            int h = 0;
            int frameCount = 0;
            List<RgbImage> rawframes = new List<RgbImage>();
            using (var fs = new FileStream("frames.bin", FileMode.Open, FileAccess.Read))
            {
                byte[] header = new byte[12];
                fs.Read(header, 0, header.Length);
                w = BitConverter.ToInt32(header, 0);
                h = BitConverter.ToInt32(header, 4);
                frameCount = BitConverter.ToInt32(header, 8);

                for (int i = 0; i < frameCount; i++)
                {
                    var nativeMem = Converter.AllocAllignedNative(w * h * 3);
                    byte[] buffer = new byte[w * h * 3];
                    fs.Read(buffer, 0, buffer.Length);
                    Marshal.Copy(buffer, 0, nativeMem, buffer.Length);

                    var rgb = new RgbImage(ImageFormat.Bgr, w, h, w * 3, nativeMem);
                    rawframes.Add(rgb);
                }

            }

            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.NumThreads = th;
            config.EnableCustomthreadPool = 1;
            Converter.SetConfig(config);

            Console.WriteLine($"{w}x{h}");

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            decoder.Initialize();
            encoder.Initialize(w, h, 2_000_000, 30, ConfigType.CameraCaptureAdvancedHP);

            List<byte[]> frames = new List<byte[]>();

            int ctr = 0;
            int dir = 1;
            int next()
            {
                ctr += dir;
                if (ctr == frameCount - 1) dir = -1;
                if (ctr == 0) dir = 1;
                return ctr;
            }

            Stopwatch sw = Stopwatch.StartNew();
            for (int i = 0; i < numFrame; i++)
            {
                if (!encoder.Encode(rawframes[next()], out EncodedData[] ec)) continue;

                foreach (var encoded in ec)
                {
                    frames.Add(encoded.GetBytes());
                }
            }
            sw.Stop();

            Console.WriteLine();
            Console.WriteLine($"[Benchmark Result] Encoded 1000 frames in {sw.ElapsedMilliseconds} ms:");
            Console.WriteLine($"[Benchmark Result] Throughput: {((numFrame / sw.Elapsed.TotalMilliseconds) * numFrame).ToString("N2")} fps");
            Console.WriteLine();

            var rgbb = new RgbImage(ImageFormat.Rgb, w, h);
            Stopwatch sw2 = Stopwatch.StartNew();
            int kk = 0;
            foreach (var encoded in frames)
            {
                decoder.Decode(encoded, 0, encoded.Length, noDelay: true, out DecodingState ds, ref rgbb);
            }
            sw2.Stop();

            Console.WriteLine();
            Console.WriteLine($"[Benchmark Result] Decoded 1000 frames in {sw2.ElapsedMilliseconds} ms:");
            Console.WriteLine($"[Benchmark Result] Throughput: {((numFrame / sw2.Elapsed.TotalMilliseconds) * numFrame).ToString("N2")} fps");
            Console.WriteLine();

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
            config.NumThreads = th;
            config.EnableCustomthreadPool = 4;//414
            Converter.SetConfig(config);

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            var bytes = File.ReadAllBytes("RawBgr.bin");
            var data = new RgbImage(ImageFormat.Bgra, 1920, 1080, 1920 * 4, bytes);
            int w = data.Width;
            int h = data.Height;

            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraCaptureAdvancedHP);
            decoder.Initialize();
            List<byte[]> frames = new List<byte[]>();
            RgbImage rgbb = new RgbImage(ImageFormat.Rgb, w, h);

            Stopwatch sw = Stopwatch.StartNew();
            for (int i = 0; i < numFrame; i++)
            {
                if(!encoder.Encode(data, out EncodedData[] ec)) continue;
                
                foreach (var encoded in ec)
                {
                    frames.Add(encoded.GetBytes());
                }
            }
            sw.Stop();

            Console.WriteLine();
            Console.WriteLine($"[Benchmark Result] Encoded 1000 frames in {sw.ElapsedMilliseconds} ms:");
            Console.WriteLine($"[Benchmark Result] Throughput: {((numFrame / sw.Elapsed.TotalMilliseconds)* numFrame).ToString("N2") } fps");
            Console.WriteLine();

            Stopwatch sw2 = Stopwatch.StartNew();
            foreach (var encoded in frames)
            {
                decoder.Decode(encoded, 0, encoded.Length, noDelay: true, out DecodingState ds, ref rgbb);

            }
            sw2.Stop();

            Console.WriteLine();
            Console.WriteLine($"[Benchmark Result] Decoded 1000 frames in {sw2.ElapsedMilliseconds} ms:");
            Console.WriteLine($"[Benchmark Result] Throughput: {((numFrame / sw2.Elapsed.TotalMilliseconds) * numFrame).ToString("N2")} fps");
            Console.WriteLine();

                

        }

        private static void BenchmarkConverter()
        {
            Console.WriteLine();
            Console.WriteLine("##### Benchmarking Converter");

            int numFrame = 5000;

            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.EnableAvx512 = 1;
            config.EnableCustomthreadPool = 1;
            config.NumThreads =4;
            Converter.SetConfig(config);

            var bytes = File.ReadAllBytes("RawBgr.bin");
            var source = new RgbImage(ImageFormat.Bgra, 1920, 1080, 1920 * 4, bytes);
            int w = source.Width;
            int h = source.Height;

            YuvImage yuvImage = new YuvImage(w, h);
            RgbImage rgb = new RgbImage(ImageFormat.Rgb, w, h);

            Converter.Rgb2Yuv(source, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);

            Stopwatch sw2 = new Stopwatch();

            for (int i = 0; i < 5; i++)
            {
                sw2.Restart();
                Converter.Yuv2Rgb(yuvImage, rgb);
                sw2.Stop();
                Console.WriteLine($">>>>>>>>>>>>>>>>YUV2TGB frame in {sw2.Elapsed.TotalMilliseconds} ms.");
                Thread.Sleep(100);
                sw2.Restart();
                Converter.Rgb2Yuv(rgb, yuvImage);
                sw2.Stop();
                Console.WriteLine($"<<RGB2YUV frame in {sw2.Elapsed.TotalMilliseconds} ms.");
                Thread.Sleep(100);


            }
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
            config.ForceNaiveConversion = 1;
            config.EnableAvx2 = 1;
            config.NumThreads = th;
            Converter.SetConfig(config);

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            var bytes = File.ReadAllBytes("RawBgr.bin");
            var data = new RgbImage(ImageFormat.Bgra, 1920, 1080, 1920 * 4, bytes);
            int w = data.Width;
            int h = data.Height;

            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraBasic);
            decoder.Initialize();

            RgbImage rgbb = new RgbImage(ImageFormat.Rgb, w, h);

            var yuv = new YuvImage(w, h);
            var nv12 = new YUVNV12ImagePointer(Converter.AllocAllignedNative((w * h) + (w * h) / 2), 1920, 1080);
            Converter.Rgb2Yuv(data, yuv);
            ConvertI420ToNV12(yuv.ImageBytes, w, h, nv12.Y);

            encoder.Encode(nv12, out EncodedData[] ec);
            foreach (var encoded in ec)
            {

                if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgbb))
                {
                    var b = rgbb.GetBytes();
                    File.WriteAllBytes("randomEncoded.bin", b);

                    var encodedIm = rgbb.GetBytes();
                    var knownEncoded = File.ReadAllBytes("randomEncoded.bin");

                    Console.WriteLine();
                    Console.WriteLine("Testing Complete");
                    if (CheckSeq(knownEncoded, encodedIm))
                    {
                        Console.WriteLine("[Test Result] Encode/Decode Test PASS");
                        return;
                    }
                    else
                    {
                        Console.WriteLine("[Test Result] Encode/Decode Test FAIL!");
                        return;
                    }

                }


            }
            Console.WriteLine("[Test Result] Encode/Decode Test FAIL!");

        }

        static void TestCorrectness()
        {
            Console.WriteLine();
            Console.WriteLine("##### Testing Converter Correctness");

            var config = ConverterConfig.Default;
            config.EnableSSE = 0;
            config.EnableNeon = 0;
            config.EnableAvx2 = 0;
            config.NumThreads = th;
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
            Converter.AllocAllignedNative(1920 * 1080 * 4);
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

            var bytes = File.ReadAllBytes("random.bin");
            var srcExtRandom = new RgbImage(ImageFormat.Bgra, 1920, 1080, 1920 * 4, bytes);

            Converter.SetConfig(config);

            YuvImage yuvImage = new YuvImage(w, h);
            YuvImage yuvImage1 = new YuvImage(w, h);
            YuvImage yuvImage2 = new YuvImage(w, h);
            YuvImage yuvImage3 = new YuvImage(w, h);

            var rgb = new RgbImage(ImageFormat.Rgb,w, h);
            var rgb1 = new RgbImage(ImageFormat.Bgr, w, h);
            var rgb2 = new RgbImage(ImageFormat.Rgba, w, h);
            var rgb3 = new RgbImage(ImageFormat.Bgra, w, h);


            var src = new RgbImage(ImageFormat.Rgb, w, h, w, randImage3);
            var src1 = new RgbImage(ImageFormat.Bgr, w, h, w, randImage3);
            var src2 = new RgbImage(ImageFormat.Rgba, w, h, w, randImage4);
            var src3 = new RgbImage(ImageFormat.Bgra, w, h, w, randImage4);
             

            Convert(src, yuvImage, rgb);
            Convert(src, yuvImage1, rgb1);
            Convert(src2, yuvImage2, rgb2);
            Convert(srcExtRandom, yuvImage3, rgb3);

            return new TestData(yuvImage, yuvImage1, yuvImage2, yuvImage3,
                                        rgb, rgb1, rgb2, rgb3);

           
        }

        private static void Convert(RgbImage source,YuvImage yuvImage, RgbImage rgb)
        {
            Converter.Rgb2Yuv(source, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
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
                 Console.WriteLine("---yuv----");
                res &= CheckSeq(y1.GetBytes(), other.y1.GetBytes());
                res &= CheckSeq(y2.GetBytes(), other.y2.GetBytes());
                res &= CheckSeq(y3.GetBytes(), other.y3.GetBytes());
                res &= CheckSeq(y4.GetBytes(), other.y4.GetBytes());
                Console.WriteLine("---rgb-----");

                res &= CheckSeq(r1.GetBytes(), other.r1.GetBytes());
                res &= CheckSeq(r2.GetBytes(), other.r2.GetBytes());
                res &= CheckSeq(r3.GetBytes(), other.r3.GetBytes());
                res &= CheckSeq(r4.GetBytes(), other.r4.GetBytes());

                if (!res) Console.WriteLine("-------------------");
                if (!res) Console.WriteLine(" - FAIL"); else Console.WriteLine(" - PASS");

                return res;

            }
        }
        static bool CheckSeq(byte[] b1, byte[] b2)
        {
            bool ret = true;
            int max = 0;
            for (int i = 0; i < b1.Length; i++)
            {
                if (b1[i] != b2[i] && Math.Abs(b1[i] - b2[i]) > 0)
                {
                    //Console.WriteLine($"Mismatch at {i} : {b1[i]} != {b2[i]}");
                    ret = false;
                    max = Math.Max(max, Math.Abs(b1[i] - b2[i]));
                }

            }
            //if(!ret)
                Console.WriteLine("max ▲ : " + max);

            return ret;
        }

        public static void ConvertI420ToNV12(IntPtr ImageBytes, int width, int height, IntPtr NV12Buffer)
        {
            int ySize = width * height;
            int uvSize = ySize / 4;
            unsafe
            {
                byte* src = (byte*)ImageBytes.ToPointer();
                byte* dst = (byte*)NV12Buffer.ToPointer();
                for (int i = 0; i < ySize; i++)
                {
                    dst[i] = src[i];
                }

                byte* uSrc = src + ySize;
                byte* vSrc = uSrc + uvSize;
                byte* uvDst = dst + ySize;

                for (int i = 0; i < uvSize; i++)
                {
                    uvDst[2 * i] = uSrc[i];
                    uvDst[2 * i + 1] = vSrc[i];
                }
            }
        }


    }

}
