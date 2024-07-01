using System;
using System.Runtime.InteropServices;

namespace H264Sharp
{
    public class Converter
    {
        //todo
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void RGBtoYUVx64(ref RGBImagePointer rgb, ref YUVImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.Cdecl)]
        private static extern void RGBtoYUVx86( ref RGBImagePointer rgb, ref YUVImagePointer yuv,int numThreads);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void YUV2RGBx64(ref YUVImagePointer rgb, ref RGBImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YUV2RGBx86(ref YUVImagePointer rgb, ref RGBImagePointer yuv, int numThreads);
        //

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        private static extern void DownscaleImgx86(ref UnsafeGenericImage from, ref UnsafeGenericImage to, int mul);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        private static extern void DownscaleImgx64(ref UnsafeGenericImage from, ref UnsafeGenericImage to, int mul);
        


        private readonly bool x64 = Environment.Is64BitProcess;

        /// <summary>
        /// Downslales image by given factor efficiently 
        /// i.e multiplier 2 gives w/2,h/2.
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        /// <param name="multiplier"></param>
        public void Downscale(ImageData from, RgbImage to, int multiplier)
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

                        if (x64)
                        {
                            DownscaleImgx64(ref ugi, ref t, multiplier);
                        }
                        else
                        {
                            DownscaleImgx86(ref ugi, ref t, multiplier);
                        }

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

                    if (x64)
                    {
                        DownscaleImgx64(ref ugi, ref t, multiplier);
                    }
                    else
                    {
                        DownscaleImgx86(ref ugi, ref t, multiplier);
                    }
                }
            }
        }
    }
}



