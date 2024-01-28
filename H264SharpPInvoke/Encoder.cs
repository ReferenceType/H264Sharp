using System;
//using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace H264PInvoke
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
        /// Advanced configuration camera capture
        /// </summary>
        CameraCaptureAdvanced,
        /// <summary>
        /// Advanced configuration screen capture
        /// </summary>
        ScreenCaptureAdvanced
    };

    public class Encoder:IDisposable
    {
        #region Dll Import
        
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private static extern IntPtr GetEncoderx64(string dllName);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern void InitializeEncoderx64(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "Encode", CallingConvention = CallingConvention.StdCall)]
        private static extern bool Encodex64(IntPtr encoder, ref UnsafeGenericImage s, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "Encode1", CallingConvention = CallingConvention.StdCall)]
        private static extern bool Encode1x64(IntPtr encoder, ref byte yuv, ref FrameContainer fc);
       
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.StdCall)]
        private static extern int ForceIntraFramex64(IntPtr encoder);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.StdCall)]
        private static extern void SetMaxBitratex64(IntPtr encoder, int target);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.StdCall)]
        private static extern void SetTargetFpsx64(IntPtr encoder, float target);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.StdCall)]
        private static extern void FreeEncoderx64(IntPtr encoder);

        //---

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private static extern IntPtr GetEncoderx86(string dllName);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern void InitializeEncoderx86(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "Encode", CallingConvention = CallingConvention.StdCall)]
        private static extern bool Encodex86(IntPtr encoder, ref UnsafeGenericImage s, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "Encode1", CallingConvention = CallingConvention.StdCall)]
        private static extern bool Encode1x86(IntPtr encoder, ref byte yuv, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.StdCall)]
        private static extern int ForceIntraFramex86(IntPtr encoder);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.StdCall)]
        private static extern void SetMaxBitratex86(IntPtr encoder, int target);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.StdCall)]
        private static extern void SetTargetFpsx86(IntPtr encoder, float target);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.StdCall)]
        private static extern void FreeEncoderx86(IntPtr encoder);
        #endregion

        private readonly IntPtr encoder;
        private bool disposedValue;
        private readonly bool x64 = Environment.Is64BitProcess;

        /// <summary>
        /// Creates new instance. You can change the cisco dll name with <see cref="Defines"></see> class before initialisation
        /// </summary>
        public Encoder()
        {
            encoder = x64? GetEncoderx64(Defines.CiscoDllName64bit):GetEncoderx86(Defines.CiscoDllName32bit);
        }

        /// <summary>
        /// Initialises the encoder.
        /// </summary>
        /// <param name="width">Expected frame width</param>
        /// <param name="height">Expected frame height</param>
        /// <param name="bitrate">Target bitrate in bits</param>
        /// <param name="fps">Target frames per second</param>
        /// <param name="configType">Configuration type</param>
        public void Initialize(int width, int height, int bitrate, int fps, ConfigType configType)
        {
            if (x64)
                InitializeEncoderx64(encoder, width, height, bitrate, fps, (int)configType);
            else 
                InitializeEncoderx86(encoder, width, height, bitrate, fps, (int)configType);
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

        /// <summary>
        /// Encodes Rgb,Bgr,Rgba,Bgra images
        /// </summary>
        /// <param name="img">Rgb,Bgr,Rgba,Bgra format image pointer class</param>
        /// <param name="ed">Encoded data</param>
        /// <returns></returns>
        public bool Encode(GenericImage img, out EncodedData[] ed)
        {
            var fc = new FrameContainer();
            var ugi = Convert(img);

            var success = x64 ? Encodex64(encoder, ref ugi, ref fc) : Encodex86(encoder, ref ugi, ref fc);
            ed = Convert(fc);
            return success;
        }

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
            return success;
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
            EncodedData[] data = new EncodedData[fc.Lenght];
            for (int i = 0; i < fc.Lenght; i++)
            {
                var ef = *fc.Frames;
                data[i] = new EncodedData(new IntPtr(ef.Data), ef.Length, (FrameType)ef.Type, ef.LayerNum);

                fc.Frames++;
            }
            return data;
        }

        private unsafe UnsafeGenericImage Convert(GenericImage im)
        {
            return new UnsafeGenericImage()
            {
                ImageBytes = (byte*)im.ImageBytes,
                Width = im.Width,
                Height = im.Height,
                Stride = im.Stride,
                ImgType = im.ImgType,
            };
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                FreeEncoderx64(encoder);
                disposedValue = true;
            }
        }

        ~Encoder()
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
