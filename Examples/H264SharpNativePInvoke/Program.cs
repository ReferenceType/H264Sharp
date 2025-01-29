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
        static Bitmap RawRgbToBitmap(byte[] rawRgbData, int width, int height)
        {
            if (rawRgbData.Length != width * height * 3)
                throw new ArgumentException("The size of the raw RGB data does not match the specified dimensions.");

            // Create a new Bitmap
            Bitmap bitmap = new Bitmap(width, height, PixelFormat.Format24bppRgb);

            // Lock the Bitmap's bits for writing
            BitmapData bitmapData = bitmap.LockBits(
                new Rectangle(0, 0, width, height),
                ImageLockMode.WriteOnly,
                PixelFormat.Format24bppRgb);

            // Copy the raw RGB data into the Bitmap's memory
            IntPtr ptr = bitmapData.Scan0;
            int stride = bitmapData.Stride;
            int offset = stride - width * 3;

            // Handle cases where stride is not equal to width * 3
            if (offset == 0)
            {
                Marshal.Copy(rawRgbData, 0, ptr, rawRgbData.Length);
            }
            else
            {
                // Copy row by row if stride padding exists
                for (int y = 0; y < height; y++)
                {
                    Marshal.Copy(rawRgbData, y * width * 3, ptr + y * stride, width * 3);
                }
            }

            // Unlock the Bitmap's bits
            bitmap.UnlockBits(bitmapData);

            return bitmap;
        }
        static unsafe void Main(string[] args)
        {
            //var bytes = File.ReadAllBytes("Output.bin");

            //Bitmap bp = RawRgbToBitmap(bytes, 1920, 1080);
            //bp.Save("CVR.bmp");

            //var bytes1 = File.ReadAllBytes("Output1.bin");

            //Bitmap bp1 = RawRgbToBitmap(bytes1, 1920, 1080);
            //bp1.Save("CVR1.bmp");


            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.NumthreadsRgb2Yuv = 1;
            config.NumthreadsYuv2Rgb = 1;
            config.EnableCustomthreadPool = 0;
           
            Converter.SetConfig(config);


            var img1 = System.Drawing.Image.FromFile("ocean 1920x1080.jpg");
            var bmp1 = new Bitmap(img1);
            var imd = bmp1.ToImageData();
            YuvImage yuv = new YuvImage(bmp1.Width, bmp1.Height);
            Converter.Rgbx2Yuv(imd,yuv);
            RgbImage rgb = new RgbImage(bmp1.Width, bmp1.Height);
            Converter.Yuv2Rgb(yuv, rgb);

            var nbmp = rgb.ToBitmap();
            nbmp.Save("OUT.bmp");

           
            Converter.Rgbx2Yuv(imd, yuv);
            Stopwatch swa = Stopwatch.StartNew();
            for (int i = 0; i < 1000; i++)
            {
               //Converter.Rgbx2Yuv(imd, yuv);

                Converter.Yuv2Rgb(yuv, rgb);
            }
           swa.Stop();
            Console.WriteLine(swa.ElapsedMilliseconds);
            Thread.Sleep(1000);
           // return;


            H264Encoder.EnableDebugPrints = true;   
            H264Decoder.EnableDebugPrints = true;   
            
           // BencmarkConverter();
            //return;
            // You can change version or specify the path for cisco dll.

            //Defines.CiscoDllName64bit = "openh264-2.5.0-win64.dll";
            //Defines.CiscoDllName32bit = "openh264-2.4.0-win32.dll";

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

          
            decoder.Initialize();

            // var img = System.Drawing.Image.FromFile("ocean 1920x1080.jpg");
            var img = System.Drawing.Image.FromFile("ocean 3840x2160.jpg");

            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);
            Console.WriteLine($"{w}x{h}");

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


                    if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgbb))
                    {
                        //Console.WriteLine($"F:{encoded.FrameType} size: {encoded.Length}");
                        //var result = rgbb.ToBitmap();
                        //result.Save("Ok1.bmp");

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
            //var img = System.Drawing.Image.FromFile("ocean 3840x2160.jpg");
            var img = System.Drawing.Image.FromFile("ocean 1920x1080.jpg");

            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);


            YuvImage yuvImage = new YuvImage(w, h);
            RgbImage rgb = new RgbImage(w, h);

            var data = BitmapToImageData(bmp);

            Converter.Rgbx2Yuv(data, yuvImage);
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

        /*
         * Pixel data is ARGB, 1 byte for alpha, 1 for red, 1 for green, 1 for blue. 
         * Alpha is the most significant byte, blue is the least significant.
         * On a little-endian machine, like yours and many others,
         * the little end is stored first, so the byte order is b g r a.
         */
        private static ImageData BitmapToImageData(Bitmap bmp)
        {
            int width = bmp.Width;
            int height = bmp.Height;
            BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, width, height),
                                              ImageLockMode.ReadOnly,
                                              PixelFormat.Format32bppArgb);
            var bmpScan = bmpData.Scan0;

            //PixelFormat.Format32bppArgb is default
            ImageType type = ImageType.Rgb;
            switch (bmp.PixelFormat)
            {
                case PixelFormat.Format32bppArgb:
                    type = ImageType.Bgra; //endianness
                    break;
                case PixelFormat.Format32bppRgb:
                    type = ImageType.Bgra;
                    break;
                case PixelFormat.Format24bppRgb:
                    type = ImageType.Bgr;
                    break;
                default:
                    throw new NotSupportedException($"Format {bmp.PixelFormat} is not supported");

            }

            var img = new H264Sharp.ImageData(type, width, height, bmpData.Stride, bmpScan);

            //------------------------------------------------------------------------------
            //this is for endoded bmp files(i.e. on disc).

            // MemoryStream stream = new MemoryStream();
            // bmp.Save(stream, ImageFormat.Bmp);
            // int stride = ((((width * 32) + 31) & ~31) >> 3);//24 or 32 bit depth
            // int lineLenght = stride;
            // int data_ptr = lineLenght * (height - 1); // ptr to last line

            // var buff = stream.GetBuffer();
            // // 54 is info header of encoded bmp.
            //// var rgb = new EncodedBmp(stream, desc.Width, desc.Height, -stride, data_ptr + 54);
            // unsafe
            // {
            //     fixed (byte* ptr = &buff[data_ptr + 54]) 
            //     {

            //         img.Stride = -stride;
            //         img.Width = width;
            //         img.Height = height;
            //         img.ImageBytes = new IntPtr(ptr);
            //         return img;
            //     }

            // }
            //-------------------------------------------------------------------------
            //To save raw bgra without metadata

            //var bytes = new byte[bmpData.Stride * height];
            //unsafe
            //{
            //    fixed (byte* ptr = bytes)
            //    {
            //        Buffer.MemoryCopy((byte*)bmpScan, ptr, bytes.Length, bytes.Length);

            //    }

            //}
            //File.WriteAllBytes("RawBgr.bin",bytes);
            //img = new ImageData(ImageType.Bgra, width, height, bmpData.Stride, bytes);

            bmp.UnlockBits(bmpData);
            return img;
        }
    }
}
#pragma warning restore CA1416 // Validate platform compatibility

