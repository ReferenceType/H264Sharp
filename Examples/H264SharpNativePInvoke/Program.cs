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
        static unsafe void Main(string[] args)
        {
            //BencmarkConverter();
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


                    if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref  rgbb))
                    {
                        //Console.WriteLine($"F:{encoded.FrameType} size: {encoded.Length}");
                        //var result = rgbb.ToBitmap();
                        //result.Save("OUT2.bmp");

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

           // GenerateRandomBitmap(1920, 1080, "random.bmp");

            //var ss = System.Drawing.Image.FromFile("random.bmp");
            //var bmpss = new Bitmap(ss);
            //var raw = bmpss.BitmapToRawBytes();
            //File.WriteAllBytes("rawss.bin", raw);

          
            var config = ConverterConfig.Default;
            config.EnableSSE = 1;
            config.EnableNeon = 1;
            config.EnableAvx2 = 1;
            config.EnableAvx512 = 1;
            config.NumThreads = 1;
            config.EnableCustomthreadPool = 1;
            config.EnableDebugPrints = 1;
            Converter.SetConfig(config);

            var cnf = Converter.GetCurrentConfig();

            //var img = System.Drawing.Image.FromFile("ocean 3840x2160.jpg");
            var img = System.Drawing.Image.FromFile("random.bmp");

            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);


            YuvImage yuvImage = new YuvImage(w, h);
            RgbImage rgb = new RgbImage(w, h);

            var data = bmp.ToImageData();

            Converter.Rgb2Yuv(data, yuvImage);
            Converter.Yuv2Rgb(yuvImage, rgb);
            rgb.ToBitmap().Save("converted.bmp");

            Stopwatch sw = Stopwatch.StartNew();
            for (int i = 0; i < 5000; i++)
            {

                Converter.Yuv2Rgb(yuvImage, rgb);

                Converter.Rgb2Yuv(rgb, yuvImage);

            }
            Console.WriteLine(sw.ElapsedMilliseconds);

        }

        public static void GenerateRandomBitmap(int width, int height, string filename)
        {
            // Create a new bitmap image
            using (Bitmap bmp = new Bitmap(width, height, PixelFormat.Format24bppRgb)) // 24bpp for standard RGB
            {
                // Lock the bitmap bits for direct access (faster)
                Rectangle rect = new Rectangle(0, 0, width, height);
                BitmapData bmpData = bmp.LockBits(rect, ImageLockMode.WriteOnly, bmp.PixelFormat);

                // Get the address of the first line of pixels
                IntPtr ptr = bmpData.Scan0;

                // Declare an array to hold the pixel data (3 bytes per pixel: B, G, R)
                int bytesPerPixel = 3;  // Assuming 24bpp RGB
                int bytesPerRow = bmpData.Stride; // Important: Stride may be padded!
                int totalBytes = height * bytesPerRow;
                byte[] pixels = new byte[totalBytes];

                // Generate random RGB values and fill the pixel data array
                Random rand = new Random(42);
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        int index = y * bytesPerRow + x * bytesPerPixel;
                        pixels[index] = (byte)rand.Next(0, 256);        // Blue
                        pixels[index + 1] = (byte)rand.Next(0, 256);    // Green
                        pixels[index + 2] = (byte)rand.Next(0, 256);    // Red
                    }
                }

                // Copy the pixel data to the bitmap
                System.Runtime.InteropServices.Marshal.Copy(pixels, 0, ptr, totalBytes);

                // Unlock the bits
                bmp.UnlockBits(bmpData);

                // Save the image as a BMP file
                bmp.Save(filename, ImageFormat.Bmp);
                Console.WriteLine($"Bitmap image saved to {filename}");
            } // The 'using' statement ensures the Bitmap is disposed of correctly
        }


    }
}
#pragma warning restore CA1416 // Validate platform compatibility

