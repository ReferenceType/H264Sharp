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
        /*
         * Reference to H264SHarpPInvoke dll (.Net Standard 2.0).
         * Add H264SharpNative-win32/64.dll to your executable directory.
         * You need to use System.Drawing.Common nuget package to work with Bitmaps.(not needed on WPF/forms etc)
         */
        static void Main(string[] args)
        {
            //Defines.CiscoDllName64bit = "C:\\Users\\dcano\\Desktop\\OpenH264Wrapper2\\Examples\\H264SharpNativePInvoke\\bin\\Release\\net6.0\\runtimes\\win-x64\\native\\openh264-2.4.0-win64.dll";

            //Defines.CiscoDllName64bit = "openh264-2.4.0-win64.dll";
            //Defines.CiscoDllName32bit = "openh264-2.4.0-win32.dll";
            Console.WriteLine(Defines.CiscoDllName64bit);
            Encoder encoder = new Encoder();
            Decoder decoder = new Decoder();

            var img = System.Drawing.Image.FromFile("ocean.jpg");
            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);

            encoder.Initialize(w, h, 20_000_000, 30, ConfigType.CameraBasic);
            Console.WriteLine("Initialised");
            Stopwatch sw = Stopwatch.StartNew();

            for (int j = 0; j < 100; j++)
            {
                var data = BitmapToGenericImage(bmp);
                encoder.Encode(data, out EncodedData[] ec);

                //encoder.ForceIntraFrame();
                //encoder.SetMaxBitrate(2000000);
                //encoder.SetTargetFps(16.9f);

                foreach (var encoded in ec)
                {
                    //encoded.GetBytes();
                    //encoded.CopyTo(buffer,offset,count);

                    if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, out RGBImage rgb))
                    {
                        Bitmap result = RgbToBitmap(rgb);
                        //result.Save("Ok.bmp");
                    }
                }
            }

            sw.Stop();
            Console.WriteLine(sw.ElapsedMilliseconds);

            encoder.Dispose();
            decoder.Dispose();
            Console.ReadLine();
        }

        private static Bitmap RgbToBitmap(RGBImage img)
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
        private static GenericImage BitmapToGenericImage(Bitmap bmp)
        {
            int width = bmp.Width;
            int height = bmp.Height;
            BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, width, height),
                                              ImageLockMode.ReadOnly,
                                              PixelFormat.Format32bppArgb);
            var bmpScan = bmpData.Scan0;

            //PixelFormat.Format32bppArgb is default
            var img = new GenericImage();
            switch (bmp.PixelFormat)
            {
                case PixelFormat.Format32bppArgb:
                    img.ImgType = ImageType.Bgra; //endianness
                    break;
                case PixelFormat.Format32bppRgb:
                    img.ImgType = ImageType.Bgra;
                    break;
                case PixelFormat.Format24bppRgb:
                    img.ImgType = ImageType.Bgr;
                    break;
                default:
                    throw new NotSupportedException($"Format {bmp.PixelFormat} is not supported");

            }

            img.Width = width;
            img.Height = height;
            img.Stride = bmpData.Stride;
            img.ImageBytes = bmpScan;

            bmp.UnlockBits(bmpData);
            return img;
        }
    }
}
#pragma warning restore CA1416 // Validate platform compatibility

