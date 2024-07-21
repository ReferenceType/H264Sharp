#if NETSTANDARD2_0
using System;
using System.Runtime.InteropServices;

namespace H264Sharp
{
    internal static class H264NativeApi
    {
        #region Encoder

        public static IntPtr GetEncoder()
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.GetEncoder(Defines.CiscoDllWin64)
                : H264NativeApiWin32.GetEncoder(Defines.CiscoDllWin32);
        }

        public static IntPtr GetEncoder(string dllName)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.GetEncoder(dllName)
                : H264NativeApiWin32.GetEncoder(dllName);
        }

        public static int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.InitializeEncoderBase(encoder, param)
                : H264NativeApiWin32.InitializeEncoderBase(encoder, param);
        }

        public static int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.InitializeEncoder(encoder, width, height, bps, fps, configType)
                : H264NativeApiWin32.InitializeEncoder(encoder, width, height, bps, fps, configType);
        }

        public static int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.GetDefaultParams(encoder, ref param)
                : H264NativeApiWin32.GetDefaultParams(encoder, ref param);
        }

        public static int InitializeEncoder2(IntPtr encoder, TagEncParamExt param)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.InitializeEncoder2(encoder, param)
                : H264NativeApiWin32.InitializeEncoder2(encoder, param);
        }

        public static int Encode(IntPtr encoder, ref UnsafeGenericImage s, ref FrameContainer fc)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.Encode(encoder, ref s, ref fc)
                : H264NativeApiWin32.Encode(encoder, ref s, ref fc);
        }

        public static int Encode1(IntPtr encoder, ref byte yuv, ref FrameContainer fc)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.Encode1(encoder, ref yuv, ref fc)
                : H264NativeApiWin32.Encode1(encoder, ref yuv, ref fc);
        }

        public static int ForceIntraFrame(IntPtr encoder)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.ForceIntraFrame(encoder)
                : H264NativeApiWin32.ForceIntraFrame(encoder);
        }

        public static void SetMaxBitrate(IntPtr encoder, int target)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.SetMaxBitrate(encoder, target);
            else
                H264NativeApiWin32.SetMaxBitrate(encoder, target);
        }

        public static void SetTargetFps(IntPtr encoder, float target)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.SetTargetFps(encoder, target);
            else
                H264NativeApiWin32.SetTargetFps(encoder, target);
        }

        public static void FreeEncoder(IntPtr encoder)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.FreeEncoder(encoder);
            else
                H264NativeApiWin32.FreeEncoder(encoder);
        }

        public static void SetParallelConverterEnc(IntPtr encoder, int isParallel)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.SetParallelConverterEnc(encoder, isParallel);
            else
                H264NativeApiWin32.SetParallelConverterEnc(encoder, isParallel);
        }

        public static int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.GetOptionEncoder(encoder, option, value)
                : H264NativeApiWin32.GetOptionEncoder(encoder, option, value);
        }

        public static int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.SetOptionEncoder(encoder, option, value)
                : H264NativeApiWin32.SetOptionEncoder(encoder, option, value);
        }

        #endregion

        #region Decoder

        public static IntPtr GetDecoder()
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.GetDecoder(Defines.CiscoDllWin64)
                : H264NativeApiWin32.GetDecoder(Defines.CiscoDllWin32);
        }

        public static IntPtr GetDecoder(string s)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.GetDecoder(s)
                : H264NativeApiWin32.GetDecoder(s);
        }

        public static int InitializeDecoderDefault(IntPtr dec)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.InitializeDecoderDefault(dec)
                : H264NativeApiWin32.InitializeDecoderDefault(dec);
        }

        public static int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.InitializeDecoder(dec, param)
                : H264NativeApiWin32.InitializeDecoder(dec, param);
        }

        public static bool DecodeAsRGB(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref RGBImagePointer decoded)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.DecodeAsRGB(decoder, ref frame, lenght, noDelay, ref state, ref decoded)
                : H264NativeApiWin32.DecodeAsRGB(decoder, ref frame, lenght, noDelay, ref state, ref decoded);
        }

        public static bool DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.DecodeAsYUV(decoder, ref frame, lenght, noDelay, ref state, ref decoded)
                : H264NativeApiWin32.DecodeAsYUV(decoder, ref frame, lenght, noDelay, ref state, ref decoded);
        }

        public static unsafe bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.DecodeRgbInto(decoder, ref frame, lenght, noDelay, ref state, buffer)
                : H264NativeApiWin32.DecodeRgbInto(decoder, ref frame, lenght, noDelay, ref state, buffer);
        }

        public static void FreeDecoder(IntPtr decoder)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.FreeDecoder(decoder);
            else
                H264NativeApiWin32.FreeDecoder(decoder);
        }

        public static void SetParallelConverterDec(IntPtr decoder, int isParallel)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.SetParallelConverterDec(decoder, isParallel);
            else
                H264NativeApiWin32.SetParallelConverterDec(decoder, isParallel);
        }

        public static void UseSSEConverterDec(IntPtr decoder, bool isSSE)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.UseSSEConverterDec(decoder, isSSE);
            else
                H264NativeApiWin32.UseSSEConverterDec(decoder, isSSE);
        }

        public static int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.GetOptionDecoder(decoder, option, value)
                : H264NativeApiWin32.GetOptionDecoder(decoder, option, value);
        }

        public static int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value)
        {
            return RuntimeScanApi.Is64BitProcess
                ? H264NativeApiWin64.SetOptionDecoder(decoder, option, value)
                : H264NativeApiWin32.SetOptionDecoder(decoder, option, value);
        }

        #endregion

        #region Converter

        //todo
        public static void RGBtoYUV(ref RGBImagePointer rgb, ref YUVImagePointer yuv, int numThreads)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.RGBtoYUV(ref rgb, ref yuv, numThreads);
            else
                H264NativeApiWin32.RGBtoYUV(ref rgb, ref yuv, numThreads);
        }

        public static void YUV2RGB(ref YUVImagePointer rgb, ref RGBImagePointer yuv, int numThreads)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.YUV2RGB(ref rgb, ref yuv, numThreads);
            else
                H264NativeApiWin32.YUV2RGB(ref rgb, ref yuv, numThreads);
        }

        public static void DownscaleImg(ref UnsafeGenericImage from, ref UnsafeGenericImage to, int mul)
        {
            if (RuntimeScanApi.Is64BitProcess)
                H264NativeApiWin64.DownscaleImg(ref from, ref to, mul);
            else
                H264NativeApiWin32.DownscaleImg(ref from, ref to, mul);
        }

        #endregion
    }

    internal static class H264NativeApiWin32
    {
        #region Encoder

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr GetEncoder(string dllName);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Encode(IntPtr encoder, ref UnsafeGenericImage s, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Encode1(IntPtr encoder, ref byte yuv, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeEncoder(IntPtr encoder);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "SetParallelConverterEnc", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetParallelConverterEnc(IntPtr encoder, int isParallel);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        #endregion

        #region Decoder

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr GetDecoder(string s);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool DecodeAsRGB(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref RGBImagePointer decoded);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        public static extern unsafe bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeDecoder(IntPtr decoder);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "UseSSEYUVConverter", CallingConvention = CallingConvention.Cdecl)]
        public static extern void UseSSEConverterDec(IntPtr decoder, bool isSSE);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        #endregion

        #region Converter

        //todo
        [DllImport(Defines.WrapperDllWin32, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void RGBtoYUV(ref RGBImagePointer rgb, ref YUVImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void YUV2RGB(ref YUVImagePointer rgb, ref RGBImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDllWin32, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DownscaleImg(ref UnsafeGenericImage from, ref UnsafeGenericImage to, int mul);

        #endregion
    }

    internal static class H264NativeApiWin64
    {
        #region Encoder

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr GetEncoder(string dllName);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Encode(IntPtr encoder, ref UnsafeGenericImage s, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Encode1(IntPtr encoder, ref byte yuv, ref FrameContainer fc);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeEncoder(IntPtr encoder);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "SetParallelConverterEnc", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetParallelConverterEnc(IntPtr encoder, int isParallel);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        #endregion

        #region Decoder

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr GetDecoder(string s);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool DecodeAsRGB(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref RGBImagePointer decoded);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        public static extern unsafe bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeDecoder(IntPtr decoder);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "UseSSEYUVConverter", CallingConvention = CallingConvention.Cdecl)]
        public static extern void UseSSEConverterDec(IntPtr decoder, bool isSSE);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        #endregion

        #region Converter

        //todo
        [DllImport(Defines.WrapperDllWin64, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void RGBtoYUV(ref RGBImagePointer rgb, ref YUVImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void YUV2RGB(ref YUVImagePointer rgb, ref RGBImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDllWin64, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DownscaleImg(ref UnsafeGenericImage from, ref UnsafeGenericImage to, int mul);

        #endregion
    }
}
#endif