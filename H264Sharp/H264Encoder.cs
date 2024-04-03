using System;
using System.Runtime.CompilerServices;

//using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Text;

namespace H264Sharp
{
    public enum ConfigType
    {
        /// <summary>
        /// Standard setting for camera capture.
        /// <br/>This is recommended option for camera capture
        /// </summary>
        CameraBasic,
        /// <summary>
        /// Standard setting for screen capture
        /// <br/>This is recommended option for screen capture
        /// </summary>
        ScreenCaptureBasic,
        /// <summary>
        /// Advanced configuration camera capture.
        /// Uses LTR references, 0 intra, and high profile
        /// </summary>
        CameraCaptureAdvanced,
        /// <summary>
        /// Advanced configuration screen capture
        /// </summary>
        ScreenCaptureAdvanced
    };

    public class H264Encoder:IDisposable
    {
        #region Dll Import
        
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr GetEncoderx64([MarshalAs(UnmanagedType.LPWStr)] string dllName);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeEncoderBasex64(IntPtr encoder, TagEncParamBase param);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeEncoderx64(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetDefaultParamsx64(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeEncoder2x64(IntPtr encoder, TagEncParamExt param);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Encodex64(IntPtr encoder, ref UnsafeGenericImage s, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Encode1x64(IntPtr encoder, ref byte yuv, ref FrameContainer fc);
       
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ForceIntraFramex64(IntPtr encoder);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetMaxBitratex64(IntPtr encoder, int target);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetTargetFpsx64(IntPtr encoder, float target);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern void FreeEncoderx64(IntPtr encoder);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "SetParallelConverterEnc", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetParallelConverterEncx64(IntPtr encoder,  int isParallel);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetOptionEncoderx64(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetOptionEncoderx64(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        //---

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr GetEncoderx86([MarshalAs(UnmanagedType.LPWStr)] string dllName);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeEncoderBasex86(IntPtr encoder, TagEncParamBase param);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeEncoderx86(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetDefaultParamsx86(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeEncoder2x86(IntPtr encoder, TagEncParamExt param);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Encodex86(IntPtr encoder, ref UnsafeGenericImage s, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Encode1x86(IntPtr encoder, ref byte yuv, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ForceIntraFramex86(IntPtr encoder);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetMaxBitratex86(IntPtr encoder, int target);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetTargetFpsx86(IntPtr encoder, float target);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern void FreeEncoderx86(IntPtr encoder);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "SetParallelConverterEnc", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetParallelConverterEncx86(IntPtr encoder,  int isParallel);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetOptionEncoderx86(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetOptionEncoderx86(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

       
        #endregion

        private readonly IntPtr encoder;
        private bool disposedValue;
        private int converterNumberOfThreads;
        private readonly bool x64 = Environment.Is64BitProcess;
        public int ConverterNumberOfThreads
        {
            get => converterNumberOfThreads;
            set
            {
                converterNumberOfThreads = value;

                if (x64)
                    SetParallelConverterEncx64(encoder, value);
                else
                    SetParallelConverterEncx86(encoder, value);
            }
        }
        /// <summary>
        /// Creates new instance. You can change the cisco dll name with <see cref="Defines"></see> class before initialisation
        /// </summary>
        public H264Encoder()
        {
            encoder = x64? GetEncoderx64(Defines.CiscoDllName64bit):GetEncoderx86(Defines.CiscoDllName32bit);
        }
        public TagEncParamExt GetDefaultParameters()
        {
            TagEncParamExt paramExt = new TagEncParamExt();
            if (x64)
                GetDefaultParamsx64(encoder, ref paramExt);
            else
                GetDefaultParamsx86(encoder, ref paramExt);
            return paramExt;
        }
        /// <summary>
        /// Initialises the encoder.
        /// </summary>
        /// <param name="width">Expected frame width</param>
        /// <param name="height">Expected frame height</param>
        /// <param name="bitrate">Target bitrate in bits</param>
        /// <param name="fps">Target frames per second</param>
        /// <param name="configType">Configuration type</param>
        public int Initialize(int width, int height, int bitrate, int fps, ConfigType configType)
        {
            if (x64)
                return InitializeEncoderx64(encoder, width, height, bitrate, fps, (int)configType);
            else 
                return InitializeEncoderx86(encoder, width, height, bitrate, fps, (int)configType);
        }

        public int Initialize(TagEncParamBase param)
        {
            if (x64)
                return InitializeEncoderBasex64(encoder, param);
            else
                return InitializeEncoderBasex86(encoder, param);
        }

        public int Initialize(TagEncParamExt param)
        {
            if (x64)
                return InitializeEncoder2x64(encoder, param);
            else
                return InitializeEncoder2x86(encoder, param);
        }

        /// <summary>
        /// Forces an intra frame with instant decoder refresh on next encode.
        /// This is used to refresh decoder in case of lost frames.
        /// Be aware that Inta frames are large.
        /// </summary>
        /// <returns></returns>
        public bool ForceIntraFrame()
        {
            var ret = x64 ? ForceIntraFramex64(encoder):
                            ForceIntraFramex86(encoder);
            return ret == 0 ? true : false;
        }

        /// <summary>
        /// Sets the target max bitrate.
        /// you can call this at any point
        /// </summary>
        /// <param name="target"></param>
        public void SetMaxBitrate(int target)
        {
            if(x64)
                SetMaxBitratex64(encoder, target);
            else
                SetMaxBitratex86(encoder, target);
        }

        /// <summary>
        /// Sets target fps.
        /// you can call this at any point.
        /// </summary>
        /// <param name="target"></param>
        public void SetTargetFps(float target)
        {
            if (x64)
                SetTargetFpsx64(encoder, target);
            else 
                SetTargetFpsx86(encoder, target);
        }

        /// <summary>
        /// Gets an option
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public bool GetOption<T>(ENCODER_OPTION option, out T value) where T : struct
        {
            unsafe
            {

                value = new T();
                if (value is bool)
                {
                    bool v = (bool)((object)value);
                    byte toSet = v ? (byte)1 : (byte)0;
                    {
                        int r = x64 ? GetOptionEncoderx64(encoder, option, new IntPtr(&toSet)) :
                            GetOptionEncoderx86(encoder, option, new IntPtr(&toSet));
                        var success = (r == 0);
                        value = (T)(object)(toSet == 1 ? true : false);
                        return success;
                    }
                }
                fixed (T* V = &value)
                {
                    int r = x64 ? GetOptionEncoderx64(encoder, option, new IntPtr(V)) :
                             GetOptionEncoderx86(encoder, option, new IntPtr(V));
                    value = *V;
                    return (r == 0);
                }
            }
        }

        /// <summary>
        /// Gets an option, allows reuse of the value
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public bool GetOptionRef<T>(ENCODER_OPTION option, ref T value) where T : struct
        {
            unsafe
            {
                fixed (T* V = &value)
                {
                    int r = x64 ? GetOptionEncoderx64(encoder, option, new IntPtr(V)) :
                             GetOptionEncoderx86(encoder, option, new IntPtr(V));
                    value = *V;
                    return (r == 0);
                }
            }
        }

        /// <summary>
        /// Sets an option
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public bool SetOption<T>(ENCODER_OPTION option, T value) where T : struct
        {
            
            unsafe
            {
                if (value is bool)
                {
                    bool v = (bool)((object)value);
                    byte toSet = v ? (byte)1 : (byte)0;
                    if (x64)
                        return SetOptionEncoderx64(encoder, option, new IntPtr(&toSet)) == 0;
                    else
                        return SetOptionEncoderx86(encoder, option, new IntPtr(&toSet)) == 0;
                }
                if (x64)
                    return SetOptionEncoderx64(encoder, option, new IntPtr(&value)) == 0;
                else
                    return SetOptionEncoderx86(encoder, option, new IntPtr(&value)) == 0;
            }
           

        }

        /// <summary>
        /// Encodes imagee provided on Image data.
        /// </summary>
        /// <param name="im"></param>
        /// <param name="ed"></param>
        /// <returns></returns>
        public bool Encode(ImageData im, out EncodedData[] ed)
        {
            unsafe
            {
                var fc = new FrameContainer();
                if (im.isManaged)
                {
                    fixed (byte* dp = &im.data[im.dataOffset])
                    {
                        var ugi = new UnsafeGenericImage()
                        {
                            ImageBytes = dp,
                            Width = im.Width,
                            Height = im.Height,
                            Stride = im.Stride,
                            ImgType = im.ImgType,
                        };


                        var success = x64 ? Encodex64(encoder, ref ugi, ref fc) : Encodex86(encoder, ref ugi, ref fc);
                        ed = Convert(fc);
                        return success == 1;
                    }
                }
                else
                {
                    
                        var ugi = new UnsafeGenericImage()
                        {
                            ImageBytes = (byte*)im.imageData.ToPointer(),
                            Width = im.Width,
                            Height = im.Height,
                            Stride = im.Stride,
                            ImgType = im.ImgType,
                        };


                        var success = x64 ? Encodex64(encoder, ref ugi, ref fc) : Encodex86(encoder, ref ugi, ref fc);
                        ed = Convert(fc);
                        return success == 1;
                    
                }
                
            }
        }

        ///// <summary>
        ///// Encodes bitmap image
        ///// </summary>
        ///// <param name="bmp"></param>
        ///// <param name="ed"></param>
        ///// <returns></returns>
        //public bool Encode(Bitmap bmp, out EncodedData[] ed)
        //{
        //    var fc = new FrameContainer();

        //    var img = BitmapToStruct(bmp);
        //    var val = x64 ? Encodex64(encoder, ref img, ref fc) : Encodex86(encoder, ref img, ref fc);
        //    ed = Convert(fc);
        //    return val;
        //}

        ///// <summary>
        ///// Encodes Rgb,Bgr,Rgba,Bgra images
        ///// </summary>
        ///// <param name="img">Rgb,Bgr,Rgba,Bgra format image pointer class</param>
        ///// <param name="ed">Encoded data</param>
        ///// <returns></returns>
        //public bool Encode(UnmanagedImage img, out EncodedData[] ed)
        //{
        //    var fc = new FrameContainer();
        //    var ugi = Convert(img);

        //    var success = x64 ? Encodex64(encoder, ref ugi, ref fc) : Encodex86(encoder, ref ugi, ref fc);
        //    ed = Convert(fc);
        //    return success == 1;
        //}

       /// <summary>
       /// Encodes Yuv402P images
       /// </summary>
       /// <param name="YUV">start pointer</param>
       /// <param name="startIndex">data lenght</param>
       /// <param name="ed"></param>
       /// <returns></returns>
        public unsafe bool Encode(byte* YUV, out EncodedData[] ed)
        {
            var fc = new FrameContainer();
            var success = x64 ? Encode1x64(encoder, ref YUV[0], ref fc) : Encode1x86(encoder, ref YUV[0], ref fc);
            ed = Convert(fc);
            return success == 1;
        }

        //private static UnsafeGenericImage BitmapToStruct(Bitmap bmp)
        //{
        //    unsafe
        //    {
        //        int width = bmp.Width;
        //        int height = bmp.Height;
        //        BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
        //        byte* bmpScan = (byte*)bmpData.Scan0.ToPointer();

        //        var img = new UnsafeGenericImage();
        //        img.ImgType = ImageType.Bgra;
        //        img.Width = width;
        //        img.Height = height;
        //        img.Stride = bmpData.Stride;
        //        img.ImageBytes = bmpScan;
        //        bmp.UnlockBits(bmpData);

        //        return img;
        //    }
        //}
      
        private unsafe EncodedData[] Convert(FrameContainer fc)
        {
            EncodedData[] data = new EncodedData[fc.FrameCount];
            for (int i = 0; i < fc.FrameCount; i++)
            {
                var ef = *fc.Frames;
                data[i] = new EncodedData(ef);

                fc.Frames++;
            }
            return data;
        }

       

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                FreeEncoderx64(encoder);
                disposedValue = true;
            }
        }

        ~H264Encoder()
        {
            Dispose(disposing: false);
        }

        public void Dispose()
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }
}
