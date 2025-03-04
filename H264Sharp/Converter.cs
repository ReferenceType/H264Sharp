using System;
using System.Runtime.InteropServices;

namespace H264Sharp
{
   
    /// <summary>
    /// Image format converter
    /// </summary>
    public class Converter
    {
        /// <summary>
        /// Sets global configuration for converter.
        /// </summary>
        /// <param name="config"></param>
        public static void SetConfig(ConverterConfig config) => Defines.Native.ConverterSetConfig(config);

        /// <summary>
        /// Gets default configuration.
        /// </summary>
        /// <returns></returns>
        public static ConverterConfig GetCurrentConfig()
        {
            ConverterConfig cnf =  new ConverterConfig();
            Defines.Native.ConverterGetConfig(ref cnf);
            return cnf;
        }

        /// <summary>
        /// Sets config option
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        public static void SetOption(ConverterOption option, int value)
        {
           var currConf =  GetCurrentConfig();
            switch (option)
            {
                case ConverterOption.NumThreads:
                    currConf.NumThreads = value;
                    break;
                case ConverterOption.EnableSSE:
                    currConf.EnableSSE = value;
                    break;
                case ConverterOption.EnableNeon:
                    currConf.EnableNeon = value;
                    break;
                case ConverterOption.EnableAvx2:
                    currConf.EnableAvx2 = value;
                    break;
                case ConverterOption.EnableAvx512:
                    currConf.EnableAvx512 = value;
                    break;
                case ConverterOption.EnableCustomThreadPool:
                    currConf.EnableCustomthreadPool = value;
                    break;

                case ConverterOption.EnableDebugPrints:
                    currConf.EnableDebugPrints = value;
                    break;
               
                default:
                    break;
            }
            SetConfig(currConf);
        }

        /// <summary>
        /// Allocates aligned native memory. Must be released with <see cref="FreeAllignedNative"/>.
        /// </summary>
        /// <param name="size">The size, in bytes, of the memory to allocate.</param>
        /// <returns>A pointer to the allocated memory, or <see cref="IntPtr.Zero"/> if allocation fails.</returns>
        public static IntPtr AllocAllignedNative(int size) => //Marshal.AllocHGlobal(size);
            Defines.Native.AllocAllignedNative(size);

        /// <summary>
        /// Frees native memory allocated by <see cref="AllocAllignedNative"/>.
        /// </summary>
        /// <param name="p"></param>
        public static void FreeAllignedNative(IntPtr p) => //Marshal.FreeHGlobal(p); 
            Defines.Native.FreeAllignedNative(p);

        /// <summary>
        /// Converts RGB,BGR,RGBA,BGRA to YUVI420P
        /// </summary>
        /// <param name="from"></param>
        /// <param name="yuv"></param>
        public static void Rgb2Yuv(RgbImage from, YuvImage yuv)
        {
            unsafe
            {
                if (from.isManaged)
                {
                    fixed (byte* dp = &from.ManagedBytes[from.dataOffset])
                    {

                        var ugi = new UnsafeGenericRgbImage()
                        {
                            ImageBytes = dp,
                            Width = from.Width,
                            Height = from.Height,
                            Stride = from.Stride,
                            ImgType = from.Format,
                        };
                        var refe = yuv.ToYUVImagePointer();
                        Defines.Native.RGBXtoYUV(ref ugi, ref refe);
                    }
                }
                else
                {
                    var ugi = new UnsafeGenericRgbImage()
                    {
                        ImageBytes = (byte*)from.NativeBytes.ToPointer(),
                        Width = from.Width,
                        Height = from.Height,
                        Stride = from.Stride,
                        ImgType = from.Format,
                    };
                    var refe = yuv.ToYUVImagePointer();
                    Defines.Native.RGBXtoYUV(ref ugi, ref refe);
                }
                
                   
            }
           
        }
        private Converter() { }

       
        /// <summary>
        /// Converts YUV420p image to RGB format
        /// </summary>
        /// <param name="yuv"></param>
        /// <param name="image"></param>
        public static void Yuv2Rgb(YuvImage yuv,RgbImage image)
        {
            Yuv2Rgb(yuv.ToYUVImagePointer(), image);
        }

        public static void Yuv2Rgb(YUVImagePointer yuv, RgbImage image)
        {
            unsafe
            {
                if (image.isManaged)
                {
                    fixed (byte* dp = &image.ManagedBytes[image.dataOffset])
                    {

                        var ugi = new UnsafeGenericRgbImage()
                        {
                            ImageBytes = dp,
                            Width = image.Width,
                            Height = image.Height,
                            Stride = image.Stride,
                            ImgType = image.Format,
                        };
                        Defines.Native.YUV2RGB(ref yuv, ref ugi);
                    }
                }
                else
                {
                    var ugi = new UnsafeGenericRgbImage()
                    {
                        ImageBytes = (byte*)image.NativeBytes.ToPointer(),
                        Width = image.Width,
                        Height = image.Height,
                        Stride = image.Stride,
                        ImgType = image.Format,
                    };
                    Defines.Native.YUV2RGB(ref yuv, ref ugi);
                }


            }
        }

        /// <summary>
        /// Converts Yuv NV12 planar image to  Rgb Color Space
        /// </summary>
        /// <param name="yuv"></param>
        /// <param name="image"></param>
        public static void Yuv2Rgb(YUVNV12ImagePointer yuv, RgbImage image)
        {
            unsafe
            {
                if (image.isManaged)
                {
                    fixed (byte* dp = &image.ManagedBytes[image.dataOffset])
                    {

                        var ugi = new UnsafeGenericRgbImage()
                        {
                            ImageBytes = dp,
                            Width = image.Width,
                            Height = image.Height,
                            Stride = image.Stride,
                            ImgType = image.Format,
                        };
                        Defines.Native.YUVNV12ToRGB(ref yuv, ref ugi);
                    }
                }
                else
                {
                    var ugi = new UnsafeGenericRgbImage()
                    {
                        ImageBytes = (byte*)image.NativeBytes.ToPointer(),
                        Width = image.Width,
                        Height = image.Height,
                        Stride = image.Stride,
                        ImgType = image.Format,
                    };
                    Defines.Native.YUVNV12ToRGB(ref yuv, ref ugi);
                }


            }
        }

        /// <summary>
        /// Converts Yuv I420 planar image to  YUV NV12
        /// </summary>
        /// <param name="nv12"></param>
        /// <param name="yv12"></param>
        public static void YuvNV12toYV12(YUVNV12ImagePointer nv12, YuvImage yv12)
        {
            var yvr = yv12.ToYUVImagePointer();
            Defines.Native.YUVNV12ToYV12(ref nv12, ref yvr);
        }

        /// <summary>
        /// Downslales image by given factor efficiently 
        /// i.e multiplier 2 gives w/2,h/2.
        /// </summary>
        /// <param name="from"></param>
        /// <param name="to"></param>
        /// <param name="multiplier"></param>
        public static void Downscale(RgbImage from, RgbImage to, int multiplier)
        {
            unsafe
            {
                if (from.isManaged)
                {
                    fixed (byte* dp = &from.ManagedBytes[from.dataOffset])
                    {
                        var ugi = new UnsafeGenericRgbImage()
                        {
                            ImageBytes = dp,
                            Width = from.Width,
                            Height = from.Height,
                            Stride = from.Stride,
                            ImgType = from.Format,
                        };

                        var t = new UnsafeGenericRgbImage();

                        t.ImageBytes = (byte*)to.NativeBytes;
                        t.Width = from.Width;
                        t.Height = from.Height;
                        t.Stride = from.Stride;
                        t.ImgType = ImageFormat.Rgb;

                        Defines.Native.DownscaleImg(ref ugi, ref t, multiplier);
                    }
                }
                else
                {

                    var ugi = new UnsafeGenericRgbImage()
                    {
                        ImageBytes = (byte*)from.NativeBytes.ToPointer(),
                        Width = from.Width,
                        Height = from.Height,
                        Stride = from.Stride,
                        ImgType = from.Format,
                    };

                    var t = new UnsafeGenericRgbImage();

                    t.ImageBytes = (byte*)to.NativeBytes;
                    t.Width = from.Width;
                    t.Height = from.Height;
                    t.Stride = from.Stride;
                    t.ImgType = ImageFormat.Rgb;

                    Defines.Native.DownscaleImg(ref ugi, ref t, multiplier);
                    
                }
            }
        }


    }

    /// <summary>
    /// ConverterOption
    /// </summary>
    public enum ConverterOption
    {
        /// <summary>
        /// Number of chunks that image is divided and sent to threadpool.
        /// </summary>
        NumThreads,

        /// <summary>
        /// Allows use of SSE SIMD implementations of Converter operations. Does nothing on ARM.
        /// </summary>
        EnableSSE,

        /// <summary>
        /// Allows use of NEON SIMD implementations of Converter operations. Does nothing on x86 systems.
        /// </summary>
        EnableNeon,

        /// <summary>
        /// Allows use of AVX2 SIMD implementations of Converter operations. Does nothing on ARM.
        /// </summary>
        EnableAvx2,

        /// <summary>
        /// Not supported yet.
        /// </summary>
        EnableAvx512,

        /// <summary>
        /// Enables use of Custom Threadpool. On windows default pool is Windows poool provided on ppl.h.
        /// You can disable this behaviour and use custom pool. Depending hardware performance may vary.
        /// </summary>
        EnableCustomThreadPool,

        /// <summary>
        /// EnablesDebugPrints
        /// </summary>
        EnableDebugPrints,

        /// <summary>
        /// For test purposes only, when no SIMD enabled, uses Fixed point approximation naive converter
        /// </summary>
        ForceNaiveConversion,

    }
}



