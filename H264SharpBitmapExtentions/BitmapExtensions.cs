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

        ///// <summary>
        ///// Decodes an encoded data into Bitmap Image with PixelFormat.Format24bppRgb.
        ///// </summary>
        ///// <param name="encoded">Data buffer</param>
        ///// <param name="offset">Data buffer offset</param>
        ///// <param name="count">Data count</param>
        ///// <param name="noDelay">Specifies wether to decode immediately.<br/> This is a Cisco feature and its reccomended to be set to true</param>
        ///// <param name="state">Decoding state determines the state of the operation and decoder</param>
        ///// <param name="img"></param>
        ///// <returns></returns>
        //public static bool Decode(this  H264Decoder decoder,byte[] encoded, int offset, int count, bool noDelay, out DecodingState state, out Bitmap img)
        //{
        //    img = null;
        //    var success = decoder.Decode(encoded, offset, count, noDelay, out state, out RGBImagePointer rgb);
        //    if (success)
        //        img = rgb.ToBitmap();

        //    return success;
        //}

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
                    break;
                case H264Sharp.ImageFormat.Bgra:
                    break;
            }
            return new Bitmap(img.Width,
                              img.Height,
                              img.Width * 3,
                              PixelFormat.Format24bppRgb,
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
        public static void BitmapToRawBytes(this Bitmap bmp, MemoryStream stream)
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
            int byteLen = bmpData.Stride * height;
            if(stream.Capacity - stream.Position < byteLen)
                stream.Capacity = byteLen+(int)stream.Position;

            var bytes = stream.GetBuffer();
            unsafe
            {
                fixed (byte* ptr = bytes)
                {
                    Buffer.MemoryCopy((byte*)bmpScan, ptr, byteLen, byteLen);
                }
            }

            bmp.UnlockBits(bmpData);
            stream.Position = stream.Position + bmpData.Stride * height;
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
