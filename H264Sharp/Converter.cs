using System;
using System.Runtime.InteropServices;

namespace H264Sharp
{
    public class Converter
    {
       
        /// <summary>
        /// Converts RGB,BGR,RGBA,BGRA to YUV420P
        /// </summary>
        /// <param name="from"></param>
        /// <param name="yuv"></param>
        /// <param name="numchunks"></param>
        public static void Rgbx2Yuv(ImageData from, YuvImage yuv, int numchunks)
        {
            unsafe
            {
                if (from.isManaged)
                {
                    fixed (byte* dp = &from.data[from.dataOffset])
                    {

                        var ugi = new UnsafeGenericImage()
                        {
                            ImageBytes = dp,
                            Width = from.Width,
                            Height = from.Height,
                            Stride = from.Stride,
                            ImgType = from.ImgType,
                        };
                        var refe = yuv.GetImagePointer();
                        Defines.Native.RGBXtoYUV(ref ugi, ref refe, numchunks);
                    }
                }
                else
                {
                    var ugi = new UnsafeGenericImage()
                    {
                        ImageBytes = (byte*)from.imageData.ToPointer(),
                        Width = from.Width,
                        Height = from.Height,
                        Stride = from.Stride,
                        ImgType = from.ImgType,
                    };
                    var refe = yuv.GetImagePointer();
                    Defines.Native.RGBXtoYUV(ref ugi, ref refe, numchunks);
                }
                
                   
            }
           
        }
        /// <summary>
        /// Converts Rgb to Yuv420p
        /// </summary>
        /// <param name="from"></param>
        /// <param name="yuv"></param>
        /// <param name="numchunks"></param>
        public static void Rgb2Yuv(RgbImage from, YuvImage yuv, int numchunks)
        {
            unsafe
            {
                var ugi = new UnsafeGenericImage()
                {
                    ImageBytes = (byte*)from.ImageBytes.ToPointer(),
                    Width = from.Width,
                    Height = from.Height,
                    Stride = from.Stride,
                    ImgType = ImageType.Rgb,
                };
                var refe = yuv.GetImagePointer();
                Defines.Native.RGBXtoYUV(ref ugi, ref refe, numchunks);
            }
            
        }


        public static void Yuv2Rgb(YuvImage yuv,RgbImage image, int numchunks)
        {
            var rgb = new RGBImagePointer(image.Width, image.Height,image.Stride,image.ImageBytes);
            var yuvref = yuv.GetImagePointer();
            Defines.Native.YUV2RGB(ref yuvref, ref rgb, numchunks);
        }
        public static void Yuv2Rgb(YUVImagePointer yuv, RgbImage image, int numchunks)
        {
            var rgb = new RGBImagePointer(image.Width, image.Height, image.Stride, image.ImageBytes);
            Defines.Native.YUV2RGB(ref yuv, ref rgb, numchunks);
        }

        /// <summary>
        /// Downslales image by given factor efficiently 
        /// i.e multiplier 2 gives w/2,h/2.
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        /// <param name="multiplier"></param>
        public static void Downscale(ImageData from, RgbImage to, int multiplier)
        {
            unsafe
            {
                if (from.isManaged)
                {
                    fixed (byte* dp = &from.data[from.dataOffset])
                    {
                        var ugi = new UnsafeGenericImage()
                        {
                            ImageBytes = dp,
                            Width = from.Width,
                            Height = from.Height,
                            Stride = from.Stride,
                            ImgType = from.ImgType,
                        };

                        var t = new UnsafeGenericImage();

                        t.ImageBytes = (byte*)to.ImageBytes;
                        t.Width = from.Width;
                        t.Height = from.Height;
                        t.Stride = from.Stride;
                        t.ImgType = ImageType.Rgb;

                        Defines.Native.DownscaleImg(ref ugi, ref t, multiplier);
                    }
                }
                else
                {

                    var ugi = new UnsafeGenericImage()
                    {
                        ImageBytes = (byte*)from.imageData.ToPointer(),
                        Width = from.Width,
                        Height = from.Height,
                        Stride = from.Stride,
                        ImgType = from.ImgType,
                    };

                    var t = new UnsafeGenericImage();

                    t.ImageBytes = (byte*)to.ImageBytes;
                    t.Width = from.Width;
                    t.Height = from.Height;
                    t.Stride = from.Stride;
                    t.ImgType = ImageType.Rgb;

                    Defines.Native.DownscaleImg(ref ugi, ref t, multiplier);
                    
                }
            }
        }


    }
}



