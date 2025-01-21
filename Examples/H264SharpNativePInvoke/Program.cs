using H264Sharp;
using H264SharpBitmapExtentions;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;

namespace H264PInvoke
{
#pragma warning disable CA1416 // Validate platform compatibility

    internal class Program
    {
        static unsafe void Main(string[] args)
        {
            Converter.EnableSSE = true;
            Converter.NumThreads = 4;
            Defines.UseCustomThreadPool = true;
            //BencmarkConverter();
            //return;
            // You can change version or specify the path for cisco dll.

            //Defines.CiscoDllName64bit = "openh264-2.5.0-win64.dll";
            //Defines.CiscoDllName32bit = "openh264-2.4.0-win32.dll";

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();

          
            decoder.Initialize();

             var img = System.Drawing.Image.FromFile("ocean 1920x1080.jpg");
            //var img = System.Drawing.Image.FromFile("ocean 3840x2160.jpg");

            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);
            Console.WriteLine($"{w}x{h}");

            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraBasic);
            Console.WriteLine("Initialised Encoder");

            Stopwatch sw = Stopwatch.StartNew();
            var data = bmp.ToImageData();

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

