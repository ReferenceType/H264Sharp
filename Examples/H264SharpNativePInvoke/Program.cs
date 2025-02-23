using H264Sharp;
using H264SharpBitmapExtentions;
using OpenCvSharp;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace H264PInvoke
{
#pragma warning disable CA1416 // Validate platform compatibility

    internal class Program
    {
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
        static unsafe void Main(string[] args)
        {
            BencmarkConverter();
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

            //var img = System.Drawing.Image.FromFile("random.bmp");
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

            var yuv = new YuvImage(w, h);
            var yuv2 = new YuvImage(w, h);
            Converter.Rgb2Yuv(data, yuv);

            ConvertI420ToNV12(yuv.ImageBytes, w, h, yuv2.ImageBytes);
            YUVNV12ImagePointer nv12 = new YUVNV12ImagePointer(yuv2.ImageBytes, 1920, 1080);

            RgbImage rgbb = new RgbImage(w, h);
            Stopwatch sw = Stopwatch.StartNew();

            for (int j = 0; j < 1; j++)
            {

                if (!encoder.Encode(nv12, out EncodedData[] ec))
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


                    if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref  rgbb))
                    {
                        //Console.WriteLine($"F:{encoded.FrameType} size: {encoded.Length}");
                        var result = rgbb.ToBitmap();
                        result.Save("OUT2.bmp");

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
            int numIterations = 3000;
            int numThreads = Environment.ProcessorCount;

            var config = ConverterConfig.Default;
            config.NumThreads = numThreads;
            config.EnableDebugPrints = 1;
            Converter.SetConfig(config);

            Cv2.SetNumThreads(numThreads);

            var img = System.Drawing.Image.FromFile("ocean 3840x2160.jpg");
            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);


            YuvImage yuvImage = new YuvImage(w, h);
            RgbImage rgb = new RgbImage(w, h);

            var data = bmp.ToImageData();
            Converter.Rgb2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
            //rgb.ToBitmap().Save("converted.bmp");


            BenchmarkOpenCv(w, h, yuvImage, rgb,numIterations);
            BenchmarkH264SharpConverters(yuvImage, rgb, data, numIterations);
            BenchmarkOpenCv(w, h, yuvImage, rgb, numIterations);
            Thread.Sleep(2000);
            BenchmarkH264SharpConverters(yuvImage, rgb, data, numIterations);
            BenchmarkOpenCv(w, h, yuvImage, rgb , numIterations);
            BenchmarkH264SharpConverters(yuvImage, rgb, data, numIterations);

        }

        private static void BenchmarkOpenCv(int w, int h, YuvImage yuvImage, RgbImage rgb,int mumIter)
        {
            Mat yuvI420Mat = Mat.FromPixelData(h * 3 / 2, w, MatType.CV_8UC1, yuvImage.ImageBytes);
            Mat rgbMat = Mat.FromPixelData(h, w, MatType.CV_8UC3, rgb.ImageBytes);
            // Bencmark OpenCV

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

        private static void BenchmarkH264SharpConverters(YuvImage yuvImage, RgbImage rgb, ImageData data, int numIter)
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

