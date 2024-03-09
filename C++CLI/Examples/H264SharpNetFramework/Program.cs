using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using System.Diagnostics;
using System.Threading;
using System.Drawing.Imaging;
using H264Sharp;

namespace EncoderTest
{
    internal class Program
    {
        static H264Sharp.Encoder encoder;
        static H264Sharp.Decoder decoder;
        static void Main(string[] args)
        {
            decoder = new H264Sharp.Decoder();
            encoder = new H264Sharp.Encoder();

            var img = System.Drawing.Image.FromFile("ocean.jpg");
            int w = img.Width;
            int h = img.Height;
            var bmp = new Bitmap(img);

            Console.WriteLine($"Image Loaded with Width{w}, Height:{h}");

            encoder.Initialize(w, h, bps: 200_000_000, fps: 30, H264Sharp.Encoder.ConfigType.CameraBasic);

            // Emulating video frames
            Stopwatch sw = Stopwatch.StartNew();
            for (int i = 0; i < 100; i++)
            {
                if (encoder.Encode(bmp, out EncodedFrame[] frames))
                {
                    foreach (var frame in frames)
                    {
                        //frame.CopyTo(bb,0);

                        if (decoder.Decode(frame.Data, frame.Length, noDelay: true, out DecodingState ds, out Bitmap b))
                        {
                            b.Dispose();
                            // bmp.Save("t.bmp");
                        }
                    }

                }
            }
            Console.WriteLine("\n Time: " + sw.ElapsedMilliseconds);

            Console.ReadLine();
        }

       
    }
}
