using H264Sharp;
using H264SharpBitmapExtentions;
using H264SharpNativePInvoke;
using OpenCvSharp;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO.Compression;
using System.Runtime.InteropServices;
using System.Text.Json;

namespace H264PInvoke
{
#pragma warning disable CA1416 // Validate platform compatibility

    internal class Program
    {
        static unsafe void Main(string[] args)
        {
            //Helper.SaveRawRGBFrames("drone.mp4", "frames2.bin", 1280, 720, 30);
            
           ConverterVersusOpenCV();
           EncodeDecodeRealVideo();

            MinimalSampleCode();


        }

        static void MinimalSampleCode()
        {
            //Defines.CiscoDllName64bit = "openh264-2.5.0-win64.dll";
            //Defines.CiscoDllName32bit = "openh264-2.4.0-win32.dll";

            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.NumThreads = Environment.ProcessorCount;
            config.EnableCustomthreadPool = 1;
            Converter.SetConfig(config);

            var img = System.Drawing.Image.FromFile("ocean 1920x1080.jpg");
            int w = img.Width;
            int h = img.Height;
            var bitmap = new Bitmap(img);

            RgbImage rgbIn = bitmap.ToRgbImage();

            //rgbIn = rgbIn.ChangeAspectRatio(4, 3);
            //w = rgbIn.Width;
            //h = rgbIn.Height;


            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            decoder.Initialize();
            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraCaptureAdvanced);

            RgbImage rgbOut = new RgbImage(H264Sharp.ImageFormat.Rgb, w, h);

            for (int j = 0; j < 2; j++)
            {

                if (!encoder.Encode(rgbIn, out EncodedData[] ec))
                {
                    Console.WriteLine("skipped");
                    continue;
                }

                // You can manupulate encoder settings on runtime.
                //encoder.ForceIntraFrame();
                //encoder.SetMaxBitrate(2000000);
                //encoder.SetTargetFps(16.9f);

                foreach (var encoded in ec)
                {
                    bool keyframe = encoded.FrameType == FrameType.I || encoded.FrameType == FrameType.IDR;

                    // You cant extract the bytes
                    //encoded.GetBytes();
                    //encoded.CopyTo(buffer,offset);


                    if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgbOut))
                    {
                        //Console.WriteLine($"F:{encoded.FrameType} size: {encoded.Length}");
                        var result = rgbOut.ToBitmap();
                        result.Save("OUT2.bmp");

                    }

                }
            }

            encoder.Dispose();
            decoder.Dispose();
            Console.ReadLine();
        }

        static void EncodeDecodeRealVideo()
        {
            int numFrame = 1000;

            var data = Helper.LoadRawFrames();
            int w = data.w;
            int h = data.h; 
            int frameCount = data.frameCount;
            List<RgbImage> rawframes = data.rawframes;

            Console.WriteLine($"Testing Video Resolution:{w}x{h}");

            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.NumThreads = 32;
            config.EnableCustomthreadPool = 1;
            Converter.SetConfig(config);


            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

            decoder.Initialize();
            encoder.Initialize(w, h, 2_500_000, 30, ConfigType.CameraCaptureAdvancedHP);

            int idx = 0;
            int dir = 1;
            int next()
            {
                idx += dir;
                if (idx == frameCount - 1) dir = -1;
                if (idx == 0) dir = 1;
                return idx;
            }
            List<byte[]> encodedFrames = new List<byte[]>();

            Stopwatch sw = Stopwatch.StartNew();
            for (int i = 0; i < numFrame; i++)
            {
                if (!encoder.Encode(rawframes[next()], out EncodedData[] ec)) continue;

                foreach (var encoded in ec)
                {
                    encodedFrames.Add(encoded.GetBytes());
                }
            }
            sw.Stop();

            Console.WriteLine();
            Console.WriteLine($"[Benchmark Result] Encoded 1000 frames in {sw.ElapsedMilliseconds} ms:");
            Console.WriteLine($"[Benchmark Result] Throughput: {((numFrame / sw.Elapsed.TotalMilliseconds) * numFrame).ToString("N2")} fps");
            Console.WriteLine();

            var rgbb = new RgbImage(H264Sharp.ImageFormat.Rgba, w, h);
            var yuv = new YuvImage(w, h);
            Stopwatch sw2 = Stopwatch.StartNew();
            foreach (var encoded in encodedFrames)
            {
                decoder.Decode(encoded, 0, encoded.Length, noDelay: true, out DecodingState ds, ref yuv);

                Converter.Yuv2Rgb(yuv, rgbb);
                Mat mat = Mat.FromPixelData(h, w, MatType.CV_8UC4, rgbb.NativeBytes);
                //Cv2.ImShow("Frame", mat);
                //Cv2.WaitKey(1);
            }
            sw2.Stop();

            Console.WriteLine();
            Console.WriteLine($"[Benchmark Result] Decoded 1000 frames in {sw2.ElapsedMilliseconds} ms:");
            Console.WriteLine($"[Benchmark Result] Throughput: {((numFrame / sw2.Elapsed.TotalMilliseconds) * numFrame).ToString("N2")} fps");
            Console.WriteLine();

        }

       

        private static void ConverterVersusOpenCV()
        {
            Config conf = JsonSerializer.Deserialize<Config>(System.IO.File.ReadAllText("config.json"))!;
            if (conf == null)
                conf = new Config();

            conf.Print();

            int numIterations = conf.NumIterations;
            int numThreads = conf.Numthreads;

            var config = ConverterConfig.Default;
            config.NumThreads = numThreads;
            config.EnableDebugPrints = 1;
            config.EnableCustomthreadPool = conf.EnableCustomThreadPool;
            Converter.SetConfig(config);

            Cv2.SetNumThreads(numThreads);

            var img = System.Drawing.Image.FromFile("ocean 1920x1080.jpg");
            //var img = System.Drawing.Image.FromFile("ocean 3840x2160.jpg");
            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);

            Console.WriteLine($"Benchmarking {w}x{h} Image YUV->RGB->YUV with {numIterations} iterations started");

            YuvImage yuvImage = new YuvImage(w, h);
            RgbImage rgb = new RgbImage(H264Sharp.ImageFormat.Rgb, w, h);

            var data = bmp.ToRgbImage();
            Converter.Rgb2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);


            BenchmarkOpenCv(w, h, yuvImage, rgb, numIterations);
            Thread.Sleep(1);

            LoadImage(w, h, bmp, out yuvImage, out rgb, out data);
            BenchmarkH264SharpConverter(yuvImage, rgb, data, numIterations);
            Thread.Sleep(1);

            LoadImage(w, h, bmp, out yuvImage, out rgb, out data);
            BenchmarkOpenCv(w, h, yuvImage, rgb, numIterations);

            for (int i = 0; i < 5; i++)
            {
                Console.WriteLine("Cooldown " + (5 - i));
                Thread.Sleep(1000);
            }

            LoadImage(w, h, bmp, out yuvImage, out rgb, out data);
            BenchmarkH264SharpConverter(yuvImage, rgb, data, numIterations);
            Thread.Sleep(1);

            LoadImage(w, h, bmp, out yuvImage, out rgb, out data);
            BenchmarkOpenCv(w, h, yuvImage, rgb, numIterations);
            Thread.Sleep(1);

            LoadImage(w, h, bmp, out yuvImage, out rgb, out data);
            BenchmarkH264SharpConverter(yuvImage, rgb, data, numIterations);

        }

        private static void LoadImage(int w, int h, Bitmap bmp, out YuvImage yuvImage, out RgbImage rgb, out RgbImage data)
        {
            yuvImage = new YuvImage(w, h);
            rgb = new RgbImage(H264Sharp.ImageFormat.Rgb, w, h);
            data = bmp.ToRgbImage();
            Converter.Rgb2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
        }

        private static void BenchmarkOpenCv(int w, int h, YuvImage yuvImage, RgbImage rgb,int mumIter)
        {
            Mat yuvI420Mat = Mat.FromPixelData(h * 3 / 2, w, MatType.CV_8UC1, yuvImage.ImageBytes);
            Mat rgbMat = Mat.FromPixelData(h, w, MatType.CV_8UC3, rgb.NativeBytes);

            // WarmUp
            Cv2.CvtColor(yuvI420Mat, rgbMat, ColorConversionCodes.YUV2RGB_I420);
            Cv2.CvtColor(rgbMat, yuvI420Mat, ColorConversionCodes.RGB2YUV_I420);

            Stopwatch sw0 = Stopwatch.StartNew();
            for (int i = 0; i < mumIter; i++)
            {
                Cv2.CvtColor(yuvI420Mat, rgbMat, ColorConversionCodes.YUV2RGB_I420);
                Cv2.CvtColor(rgbMat, yuvI420Mat, ColorConversionCodes.RGB2YUV_I420);
            }
            sw0.Stop();
            Console.WriteLine("OpenCV bechmark result: " + sw0.ElapsedMilliseconds);
        }

        private static void BenchmarkH264SharpConverter(YuvImage yuvImage, RgbImage rgb, RgbImage data, int numIter)
        {
            //WarmUp
            Converter.Rgb2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);

            Stopwatch sw = Stopwatch.StartNew();
            for (int i = 0; i < numIter; i++)
            {
                Converter.Yuv2Rgb(yuvImage, rgb);
                Converter.Rgb2Yuv(rgb, yuvImage);
            }
            sw.Stop();
            Console.WriteLine("H264Sharp Converter benchmark result: " + sw.ElapsedMilliseconds);
        }


    }
}

#pragma warning restore CA1416 // Validate platform compatibility

