using H264Sharp;
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace H264SharpBitmapExtentions
{
    public static class BitmapExtensions
    {

        public static bool Encode(this H264Encoder encoder,Bitmap image, out EncodedData[] ed)
        {
            var gi=image.ToRgbImage();
            return encoder.Encode(gi,out ed);
        }


        public static Bitmap ToBitmap(this RgbImage img)
        {
            PixelFormat format = PixelFormat.Format24bppRgb;
            switch (img.Format)
            {
                case H264Sharp.ImageFormat.Rgb:
                    format = PixelFormat.Format24bppRgb;
                    break;
                case H264Sharp.ImageFormat.Bgr:
                    format = PixelFormat.Format24bppRgb;
                    break;
                case H264Sharp.ImageFormat.Rgba:
                    format = PixelFormat.Format32bppArgb;
                    break;
                case H264Sharp.ImageFormat.Bgra:
                    format = PixelFormat.Format32bppArgb;
                    break;
            }
            return new Bitmap(img.Width,
                              img.Height,
                              img.Width * 3,
                              format,
                              img.NativeBytes);
        }

        /*
         * Pixel data is ARGB, 1 byte for alpha, 1 for red, 1 for green, 1 for blue. 
         * Alpha is the most significant byte, blue is the least significant.
         * On a little-endian machine, like yours and many others,
         * the little end is stored first, so the byte order is b g r a.
         */
        public static H264Sharp.RgbImage ToRgbImage(this Bitmap bmp)
        {
            int width = bmp.Width;
            int height = bmp.Height;
            BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, width, height),
                                              ImageLockMode.ReadOnly,
                                              PixelFormat.Format32bppArgb);
            var bmpScan = bmpData.Scan0;

            //PixelFormat.Format32bppArgb is default
            H264Sharp.ImageFormat type= H264Sharp.ImageFormat.Rgb;
            switch (bmp.PixelFormat)
            {
                case PixelFormat.Format32bppArgb:
                    type = H264Sharp.ImageFormat.Bgra; //endianness
                    break;
                case PixelFormat.Format32bppRgb:
                    type = H264Sharp.ImageFormat.Bgra;
                    break;
                case PixelFormat.Format24bppRgb:
                    type = H264Sharp.ImageFormat.Bgr;
                    break;
                default:
                    throw new NotSupportedException($"Format {bmp.PixelFormat} is not supported");

            }
            
            var img = new H264Sharp.RgbImage(type, width,height,bmpData.Stride,bmpScan);

            bmp.UnlockBits(bmpData);
            return img;

        }

        /// <summary>
        /// Gets the raw bytes from bitmap without the metadata
        /// </summary>
        /// <param name="bmp"></param>
        /// <returns></returns>
        /// <exception cref="NotSupportedException"></exception>
        public static byte[] BitmapToRawBytes(this Bitmap bmp)
        {

            int width = bmp.Width;
            int height = bmp.Height;
            BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, width, height),
                                              ImageLockMode.ReadOnly,
                                              PixelFormat.Format32bppArgb);
            var bmpScan = bmpData.Scan0;

            //PixelFormat.Format32bppArgb is default
            H264Sharp.ImageFormat type = H264Sharp.ImageFormat.Rgb;
            switch (bmp.PixelFormat)
            {
                case PixelFormat.Format32bppArgb:
                    type = H264Sharp.ImageFormat.Bgra; //endianness
                    break;
                case PixelFormat.Format32bppRgb:
                    type = H264Sharp.ImageFormat.Bgra;
                    break;
                case PixelFormat.Format24bppRgb:
                    type = H264Sharp.ImageFormat.Bgr;
                    break;
                default:
                    throw new NotSupportedException($"Format {bmp.PixelFormat} is not supported");

            }

            var img = new H264Sharp.RgbImage(type, width, height, bmpData.Stride, bmpScan);
            //To save raw bgra without metadata

            var bytes = new byte[bmpData.Stride * height];
            unsafe
            {
                fixed (byte* ptr = bytes)
                {
                    Buffer.MemoryCopy((byte*)bmpScan, ptr, bytes.Length, bytes.Length);
                }
            }

            bmp.UnlockBits(bmpData);
            return bytes;
        }
       

        public static Bitmap RawRgbToBitmap(byte[] rawRgbData, int width, int height)
        {
            PixelFormat format;
            if (rawRgbData.Length == width * height * 3)
                format = PixelFormat.Format24bppRgb;
            else if(rawRgbData.Length == width * height * 4)
                format = PixelFormat.Format32bppArgb;
            else
                throw new NotSupportedException("Format not supported");

            Bitmap bitmap = new Bitmap(width, height, format);

            BitmapData bitmapData = bitmap.LockBits(
                new Rectangle(0, 0, width, height),
                ImageLockMode.WriteOnly,
                format);

            IntPtr ptr = bitmapData.Scan0;
            int stride = bitmapData.Stride;
            int offset = stride - width * 3;

            Marshal.Copy(rawRgbData, 0, ptr, rawRgbData.Length);
 
            // Unlock the Bitmap's bits
            bitmap.UnlockBits(bitmapData);

            return bitmap;
        }

    }
}
