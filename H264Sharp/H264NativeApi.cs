#if NETCOREAPP3_0_OR_GREATER
using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace H264Sharp
{
    internal static class H264NativeApi
    {
        static H264NativeApi()
        {
            NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), DllImportResolver);
        }

        static string GetLibraryName(string libraryName)
        {
            if (libraryName != Defines.WrapperDll)
            {
                return libraryName;
            }

            return RuntimeScanApi.OperatingSystem switch
            {
                OperatingSystem.Windows => RuntimeScanApi.Is64BitProcess
                    ? Defines.WrapperDllWin64
                    : Defines.WrapperDllWin32,
                OperatingSystem.Linux => RuntimeScanApi.Is64BitProcess
                    ? Defines.WrapperDllLinux64
                    : Defines.WrapperDllLinux32,
                _ => libraryName
            };
        }

        static IntPtr DllImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            var library = GetLibraryName(libraryName);
            IntPtr handle;
            NativeLibrary.TryLoad(library, assembly, searchPath, out handle);
            return handle;
        }

        #region Encoder

        public static IntPtr GetEncoder()
        {
            var ciscoDll = RuntimeScanApi.OperatingSystem switch
            {
                OperatingSystem.Windows => RuntimeScanApi.Is64BitProcess
                    ? Defines.CiscoDllWin64
                    : Defines.CiscoDllWin32,
                OperatingSystem.Linux => RuntimeScanApi.Is64BitProcess
                    ? Defines.CiscoDllLinux64
                    : Defines.CiscoDllLinux32,
                _ => throw new NotSupportedException($"{RuntimeScanApi.OperatingSystem} OS is not supported")
            };

            return GetEncoder(ciscoDll);
        }

        [DllImport(Defines.WrapperDll, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr GetEncoder(string dllName);

        [DllImport(Defines.WrapperDll, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(Defines.WrapperDll, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(Defines.WrapperDll, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(Defines.WrapperDll, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(Defines.WrapperDll, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Encode(IntPtr encoder, ref UnsafeGenericImage s, ref FrameContainer fc);

        [DllImport(Defines.WrapperDll, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        public static extern int Encode1(IntPtr encoder, ref byte yuv, ref FrameContainer fc);

        [DllImport(Defines.WrapperDll, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        public static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(Defines.WrapperDll, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(Defines.WrapperDll, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(Defines.WrapperDll, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeEncoder(IntPtr encoder);

        [DllImport(Defines.WrapperDll, EntryPoint = "SetParallelConverterEnc", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetParallelConverterEnc(IntPtr encoder, int isParallel);

        [DllImport(Defines.WrapperDll, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDll, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        #endregion

        #region Decoder

        public static IntPtr GetDecoder()
        {
            var ciscoDll = RuntimeScanApi.OperatingSystem switch
            {
                OperatingSystem.Windows => RuntimeScanApi.Is64BitProcess
                    ? Defines.CiscoDllWin64
                    : Defines.CiscoDllWin32,
                OperatingSystem.Linux => RuntimeScanApi.Is64BitProcess
                    ? Defines.CiscoDllLinux64
                    : Defines.CiscoDllLinux32,
                _ => throw new NotSupportedException($"{RuntimeScanApi.OperatingSystem} OS is not supported")
            };

            return GetDecoder(ciscoDll);
        }

        [DllImport(Defines.WrapperDll, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr GetDecoder(string s);

        [DllImport(Defines.WrapperDll, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(Defines.WrapperDll, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(Defines.WrapperDll, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool DecodeAsRGB(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref RGBImagePointer decoded);

        [DllImport(Defines.WrapperDll, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(Defines.WrapperDll, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        public static extern unsafe bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(Defines.WrapperDll, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern void FreeDecoder(IntPtr decoder);

        [DllImport(Defines.WrapperDll, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(Defines.WrapperDll, EntryPoint = "UseSSEYUVConverter", CallingConvention = CallingConvention.Cdecl)]
        public static extern void UseSSEConverterDec(IntPtr decoder, bool isSSE);

        [DllImport(Defines.WrapperDll, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDll, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        public static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        #endregion

        #region Converter

        //todo
        [DllImport(Defines.WrapperDll, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void RGBtoYUV(ref RGBImagePointer rgb, ref YUVImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDll, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void YUV2RGB(ref YUVImagePointer rgb, ref RGBImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDll, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DownscaleImg(ref UnsafeGenericImage from, ref UnsafeGenericImage to, int mul);

        #endregion
    }
}
#endif