using H264Sharp;
using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace H264PInvoke
{
#pragma warning disable CA1416 // Validate platform compatibility
   
    internal class Program
    {
        static void Main(string[] args)
        {
            //Defines.CiscoDllName64bit = "openh264-2.4.0-win64.dll";
            //Defines.CiscoDllName32bit = "openh264-2.4.0-win32.dll";

            H264Encoder encoder = new H264Encoder();
            H264Decoder decoder = new H264Decoder();
            
            encoder.ConverterNumberOfThreads = 4;
            decoder.ConverterNumberOfThreads = 4;
            decoder.EnableSSEYUVConversion = true;

            decoder.Initialize();

            var img = System.Drawing.Image.FromFile("ocean1080.jpg");
            int w = img.Width; 
            int h = img.Height;
            var bmp = new Bitmap(img);
            Console.WriteLine($"{w}x{h}");

            encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraBasic);
            Console.WriteLine("Initialised Encoder");

            Stopwatch sw = Stopwatch.StartNew();
            var data = BitmapToImageData(bmp);

            //Converter converter = new Converter();
            //RgbImage to = new RgbImage(data.Width / 2, data.Height / 2);
            //converter.Downscale(data,to,2);
            //var bb = RgbToBitmap(to);
            //bb.Save("Dowmscaled.bmp");

            RgbImage rgbb = new RgbImage(w, h);
            for (int j = 0; j < 1000; j++)
            {
              
                if(!encoder.Encode(data, out EncodedData[] ec))
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

        private static Bitmap RgbToBitmap(RGBImagePointer img)
        {
            Bitmap bmp = new Bitmap(img.Width,
                                    img.Height,
                                    img.Width * 3,
                                    PixelFormat.Format24bppRgb,
                                    img.ImageBytes);
            return bmp;
        }

        private static Bitmap RgbToBitmap(RgbImage img)
        {
            Bitmap bmp = new Bitmap(img.Width,
                                    img.Height,
                                    img.Width * 3,
                                    PixelFormat.Format24bppRgb,
                                    img.ImageBytes);
            return bmp;
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

            //this is for endoded bmp files(i.e. on disc)

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




            bmp.UnlockBits(bmpData);
            return img;
        }
    }
}
#pragma warning restore CA1416 // Validate platform compatibility

