using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace H264Sharp
{
   
    public class NativeBindings
    {
        #region Delegate

        //---------------------------------------Definition-----------------------------------------------
        // Encoder
        private delegate void EnableDebugLogsd(int val);
        private delegate IntPtr GetEncoderd(string dllName);
        private delegate int InitializeEncoderBased(IntPtr encoder, TagEncParamBase param);
        private delegate int InitializeEncoderd(IntPtr encoder, int width, int height, int bps, int fps, int configType);
        private delegate int GetDefaultParamsd(IntPtr encoder, ref TagEncParamExt param);
        private delegate int InitializeEncoder2d(IntPtr encoder, TagEncParamExt param);
        private delegate int Encoded(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);
        private delegate int Encode1d(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);
        private delegate int Encode2d(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);
        private delegate int ForceIntraFramed(IntPtr encoder);
        private delegate void SetMaxBitrated(IntPtr encoder, int target);
        private delegate void SetTargetFpsd(IntPtr encoder, float target);
        private delegate void FreeEncoderd(IntPtr encoder);
        private delegate int GetOptionEncoderd(IntPtr encoder, ENCODER_OPTION option, IntPtr value);
        private delegate int SetOptionEncoderd(IntPtr encoder, ENCODER_OPTION option, IntPtr value);
        // Decoder
        private delegate IntPtr GetDecoderd(string s);
        private delegate int InitializeDecoderDefaultd(IntPtr dec);
        private delegate int InitializeDecoderd(IntPtr dec, TagSVCDecodingParam param);
        private delegate int DecodeAsYUVd(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);
        private unsafe delegate bool DecodeRgbIntod(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);
        private delegate void FreeDecoderd(IntPtr decoder);
        private delegate void SetParallelConverterDecd(IntPtr decoder, int isParallel);
        private delegate int GetOptionDecoderd(IntPtr decoder, DECODER_OPTION option, IntPtr value);
        private delegate int SetOptionDecoderd(IntPtr decoder, DECODER_OPTION option, IntPtr value);
        // Converter
        private delegate void RGBXtoYUVd(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);
        private delegate void YUV2RGBd(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);
        private delegate void YUVNV122RGBd(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);
        private delegate void YUVNV12toYV12d(ref YUVNV12ImagePointer rgb, ref YUVImagePointer yuv);
        private delegate void DownscaleImgd(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);
        private delegate void SetConverterConfigd(ConverterConfig config);
        private delegate void GetConverterConfigd(ref ConverterConfig p);

        private delegate IntPtr AllocAllignedNatived(int size);
        private delegate void FreeAllignedNatived(IntPtr p);

        //---------------------------------------Decleration-----------------------------------------------
        // Encoder
        private EnableDebugLogsd encoderEnableDebugLogs;
        private GetEncoderd getEncoder;
        private InitializeEncoderBased initializeEncoderBase;
        private InitializeEncoderd initializeEncoder;
        private GetDefaultParamsd getDefaultParams;
        private InitializeEncoder2d initializeEncoder2;
        private Encoded encode;
        private Encode1d encode1;
        private Encode2d encode2;
        private ForceIntraFramed forceIntraFrame;
        private SetMaxBitrated setMaxBitrate;
        private SetTargetFpsd setTargetFps;
        private FreeEncoderd freeEncoder;
        private GetOptionEncoderd getOptionEncoder;
        private SetOptionEncoderd setOptionEncoder;
        // Decoder
        private EnableDebugLogsd decoderEnableDebugLogs;
        private GetDecoderd getDecoder;
        private InitializeDecoderDefaultd initializeDecoderDefault;
        private InitializeDecoderd initializeDecoder;
        private DecodeAsYUVd decodeAsYUV;
        private DecodeAsYUVd decodeAsYUVext;
        private DecodeRgbIntod decodeRgbInto;
        private FreeDecoderd freeDecoder;
        private SetParallelConverterDecd setParallelConverterDec;
        private GetOptionDecoderd getOptionDecoder;
        private SetOptionDecoderd setOptionDecoder;
        // Converter
        private RGBXtoYUVd rGBXtoYUV;
        private YUV2RGBd yUV2RGB;
        private YUVNV122RGBd YuvNV12ToRGB;
        private YUVNV12toYV12d YuvNV12ToYV12;
        private DownscaleImgd downscaleImg;
        private GetConverterConfigd getConfig;
        private SetConverterConfigd setConfig;
        private AllocAllignedNatived allocAllognedNative;
        private FreeAllignedNatived freeAllognedNative;

        #endregion

        public NativeBindings()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                switch (RuntimeInformation.ProcessArchitecture)
                {
                    case Architecture.X86:
                        LoadWindowsX86Bindings();
                        break;
                    case Architecture.X64:
                        LoadWindowsX64Bindings();
                        break;
                    default:
                        throw new PlatformNotSupportedException("Unsupported architecture.");
                }
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                switch (RuntimeInformation.ProcessArchitecture)
                {
                    case Architecture.X86:
                        LoadLinuxX86Bindings();
                        break;
                    case Architecture.X64:
                        LoadLinuxX64Bindings();
                        break;
                    case Architecture.Arm:
                        LoadLinuxArmBindings();
                        break;
                    case Architecture.Arm64:
                        LoadLinuxArm64Bindings();
                        break;
                    default:
                        throw new PlatformNotSupportedException("Unsupported architecture.");
                } 
            }
            else if (Defines.IsRunningOnAndroid())
            {
                switch (RuntimeInformation.ProcessArchitecture)
                {

                    case Architecture.Arm:
                        LoadAndroidArmBindings();
                        break;
                    case Architecture.Arm64:
                        LoadAndroidArm64Bindings();
                        break;
                    case Architecture.X64:
                        LoadAndroidx64Bindings();
                        break;
                    default:
                        throw new PlatformNotSupportedException("Unsupported architecture.");
                }
            }
            else
            {
                throw new PlatformNotSupportedException("Unsupported platform.");
            }
        }

        

        private void LoadWindowsX86Bindings()
        {
            // Encoder
            encoderEnableDebugLogs = Winx86.EncoderEnableDebugLogs;
            getEncoder = Winx86.GetEncoder;
            initializeEncoderBase = Winx86.InitializeEncoderBase;
            initializeEncoder = Winx86.InitializeEncoder;
            getDefaultParams = Winx86.GetDefaultParams;
            initializeEncoder2 = Winx86.InitializeEncoder2;
            encode = Winx86.Encode;
            encode1 = Winx86.Encode1;
            encode2 = Winx86.Encode2;
            forceIntraFrame = Winx86.ForceIntraFrame;
            setMaxBitrate = Winx86.SetMaxBitrate;
            setTargetFps = Winx86.SetTargetFps;
            freeEncoder = Winx86.FreeEncoder;
            getOptionEncoder = Winx86.GetOptionEncoder;
            setOptionEncoder = Winx86.SetOptionEncoder;
            // Decoder
            decoderEnableDebugLogs = Winx86.DecoderEnableDebugLogs;
            getDecoder = Winx86.GetDecoder;
            initializeDecoderDefault = Winx86.InitializeDecoderDefault;
            initializeDecoder = Winx86.InitializeDecoder;
            decodeAsYUV = Winx86.DecodeAsYUV;
            decodeAsYUVext = Winx86.DecodeAsYUVExt;
            decodeRgbInto = Winx86.DecodeRgbInto;
            freeDecoder = Winx86.FreeDecoder;
            setParallelConverterDec = Winx86.SetParallelConverterDec;
            getOptionDecoder = Winx86.GetOptionDecoder;
            setOptionDecoder = Winx86.SetOptionDecoder;
            // Converter
            rGBXtoYUV = Winx86.RGBXtoYUV;
            yUV2RGB = Winx86.YUV2RGB;
            YuvNV12ToRGB = Winx86.YUVNV122RGB;
            YuvNV12ToYV12 = Winx86.YuvNV12ToYV12;
            downscaleImg = Winx86.DownscaleImg;
            setConfig = Winx86.ConverterSetConfig;
            getConfig = Winx86.ConverterGetConfig;

            allocAllognedNative = Winx86.AllocAllignedNative;
            freeAllognedNative = Winx86.FreeAllignedNative;
        }

        private void LoadWindowsX64Bindings()
        {
            // Encoder
            encoderEnableDebugLogs = Winx64.EncoderEnableDebugLogs;
            getEncoder = Winx64.GetEncoder;
            initializeEncoderBase = Winx64.InitializeEncoderBase;
            initializeEncoder = Winx64.InitializeEncoder;
            getDefaultParams = Winx64.GetDefaultParams;
            initializeEncoder2 = Winx64.InitializeEncoder2;
            encode = Winx64.Encode;
            encode1 = Winx64.Encode1;
            encode2 = Winx64.Encode2;
            forceIntraFrame = Winx64.ForceIntraFrame;
            setMaxBitrate = Winx64.SetMaxBitrate;
            setTargetFps = Winx64.SetTargetFps;
            freeEncoder = Winx64.FreeEncoder;
            getOptionEncoder = Winx64.GetOptionEncoder;
            setOptionEncoder = Winx64.SetOptionEncoder;
            // Decoder
            decoderEnableDebugLogs = Winx64.DecoderEnableDebugLogs;
            getDecoder = Winx64.GetDecoder;
            initializeDecoderDefault = Winx64.InitializeDecoderDefault;
            initializeDecoder = Winx64.InitializeDecoder;
            decodeAsYUV = Winx64.DecodeAsYUV;
            decodeAsYUVext = Winx64.DecodeAsYUVExt;
            decodeRgbInto = Winx64.DecodeRgbInto;
            freeDecoder = Winx64.FreeDecoder;
            setParallelConverterDec = Winx64.SetParallelConverterDec;
            getOptionDecoder = Winx64.GetOptionDecoder;
            setOptionDecoder = Winx64.SetOptionDecoder;
            // Converter
            rGBXtoYUV = Winx64.RGBXtoYUV;
            yUV2RGB = Winx64.YUV2RGB;
            YuvNV12ToRGB = Winx64.YUVNV122RGB;
            YuvNV12ToYV12 = Winx64.YuvNV12ToYV12;
            downscaleImg = Winx64.DownscaleImg;
            setConfig = Winx64.ConverterSetConfig;
            getConfig = Winx64.ConverterGetConfig;

            allocAllognedNative = Winx64.AllocAllignedNative;
            freeAllognedNative = Winx64.FreeAllignedNative;
        }

        private void LoadLinuxX86Bindings()
        {
            // Encoder
            encoderEnableDebugLogs = Linuxx86.EncoderEnableDebugLogs;
            getEncoder = Linuxx86.GetEncoder;
            initializeEncoderBase = Linuxx86.InitializeEncoderBase;
            initializeEncoder = Linuxx86.InitializeEncoder;
            getDefaultParams = Linuxx86.GetDefaultParams;
            initializeEncoder2 = Linuxx86.InitializeEncoder2;
            encode = Linuxx86.Encode;
            encode1 = Linuxx86.Encode1;
            encode2 = Linuxx86.Encode2;
            forceIntraFrame = Linuxx86.ForceIntraFrame;
            setMaxBitrate = Linuxx86.SetMaxBitrate;
            setTargetFps = Linuxx86.SetTargetFps;
            freeEncoder = Linuxx86.FreeEncoder;
            getOptionEncoder = Linuxx86.GetOptionEncoder;
            setOptionEncoder = Linuxx86.SetOptionEncoder;
            // Decoder
            decoderEnableDebugLogs = Linuxx86.DecoderEnableDebugLogs;
            getDecoder = Linuxx86.GetDecoder;
            initializeDecoderDefault = Linuxx86.InitializeDecoderDefault;
            initializeDecoder = Linuxx86.InitializeDecoder;
            decodeAsYUV = Linuxx86.DecodeAsYUV;
            decodeAsYUVext = Linuxx86.DecodeAsYUVExt;
            decodeRgbInto = Linuxx86.DecodeRgbInto;
            freeDecoder = Linuxx86.FreeDecoder;
            setParallelConverterDec = Linuxx86.SetParallelConverterDec;
            getOptionDecoder = Linuxx86.GetOptionDecoder;
            setOptionDecoder = Linuxx86.SetOptionDecoder;
            // Converter
            rGBXtoYUV = Linuxx86.RGBXtoYUV;
            yUV2RGB = Linuxx86.YUV2RGB;
            YuvNV12ToRGB = Linuxx86.YUVNV122RGB;
            YuvNV12ToYV12 = Linuxx86.YuvNV12ToYV12;
            downscaleImg = Linuxx86.DownscaleImg;
            setConfig = Linuxx86.ConverterSetConfig;
            getConfig = Linuxx86.ConverterGetConfig;

            allocAllognedNative = Linuxx86.AllocAllignedNative;
            freeAllognedNative = Linuxx86.FreeAllignedNative;
        }

        private void LoadLinuxX64Bindings()
        {
            // Encoder
            encoderEnableDebugLogs = Linuxx64.EncoderEnableDebugLogs;
            getEncoder = Linuxx64.GetEncoder;
            initializeEncoderBase = Linuxx64.InitializeEncoderBase;
            initializeEncoder = Linuxx64.InitializeEncoder;
            getDefaultParams = Linuxx64.GetDefaultParams;
            initializeEncoder2 = Linuxx64.InitializeEncoder2;
            encode = Linuxx64.Encode;
            encode1 = Linuxx64.Encode1;
            encode2 = Linuxx64.Encode2;
            forceIntraFrame = Linuxx64.ForceIntraFrame;
            setMaxBitrate = Linuxx64.SetMaxBitrate;
            setTargetFps = Linuxx64.SetTargetFps;
            freeEncoder = Linuxx64.FreeEncoder;
            getOptionEncoder = Linuxx64.GetOptionEncoder;
            setOptionEncoder = Linuxx64.SetOptionEncoder;
            // Decoder
            decoderEnableDebugLogs = Linuxx64.DecoderEnableDebugLogs;
            getDecoder = Linuxx64.GetDecoder;
            initializeDecoderDefault = Linuxx64.InitializeDecoderDefault;
            initializeDecoder = Linuxx64.InitializeDecoder;
            decodeAsYUV = Linuxx64.DecodeAsYUV;
            decodeAsYUVext = Linuxx64.DecodeAsYUVExt;
            decodeRgbInto = Linuxx64.DecodeRgbInto;
            freeDecoder = Linuxx64.FreeDecoder;
            setParallelConverterDec = Linuxx64.SetParallelConverterDec;
            getOptionDecoder = Linuxx64.GetOptionDecoder;
            setOptionDecoder = Linuxx64.SetOptionDecoder;
            // Converter
            rGBXtoYUV = Linuxx64.RGBXtoYUV;
            yUV2RGB = Linuxx64.YUV2RGB;
            YuvNV12ToRGB = Linuxx64.YUVNV122RGB;
            YuvNV12ToYV12 = Linuxx64.YuvNV12ToYV12;
            downscaleImg = Linuxx64.DownscaleImg;
            setConfig = Linuxx64.ConverterSetConfig;
            getConfig = Linuxx64.ConverterGetConfig;

            allocAllognedNative = Linuxx64.AllocAllignedNative;
            freeAllognedNative = Linuxx64.FreeAllignedNative;
        }

        private void LoadLinuxArmBindings()
        {
            // Encoder
            encoderEnableDebugLogs = LinuxArm32.EncoderEnableDebugLogs;
            getEncoder = LinuxArm32.GetEncoder;
            initializeEncoderBase = LinuxArm32.InitializeEncoderBase;
            initializeEncoder = LinuxArm32.InitializeEncoder;
            getDefaultParams = LinuxArm32.GetDefaultParams;
            initializeEncoder2 = LinuxArm32.InitializeEncoder2;
            encode = LinuxArm32.Encode;
            encode1 = LinuxArm32.Encode1;
            encode2 = LinuxArm32.Encode2;
            forceIntraFrame = LinuxArm32.ForceIntraFrame;
            setMaxBitrate = LinuxArm32.SetMaxBitrate;
            setTargetFps = LinuxArm32.SetTargetFps;
            freeEncoder = LinuxArm32.FreeEncoder;
            getOptionEncoder = LinuxArm32.GetOptionEncoder;
            setOptionEncoder = LinuxArm32.SetOptionEncoder;
            // Decoder
            decoderEnableDebugLogs = LinuxArm32.DecoderEnableDebugLogs;
            getDecoder = LinuxArm32.GetDecoder;
            initializeDecoderDefault = LinuxArm32.InitializeDecoderDefault;
            initializeDecoder = LinuxArm32.InitializeDecoder;
            decodeAsYUV = LinuxArm32.DecodeAsYUV;
            decodeAsYUVext = LinuxArm32.DecodeAsYUVExt;
            decodeRgbInto = LinuxArm32.DecodeRgbInto;
            freeDecoder = LinuxArm32.FreeDecoder;
            setParallelConverterDec = LinuxArm32.SetParallelConverterDec;
            getOptionDecoder = LinuxArm32.GetOptionDecoder;
            setOptionDecoder = LinuxArm32.SetOptionDecoder;
            // Converter
            rGBXtoYUV = LinuxArm32.RGBXtoYUV;
            yUV2RGB = LinuxArm32.YUV2RGB;
            YuvNV12ToRGB = LinuxArm32.YUVNV122RGB;
            YuvNV12ToYV12 = LinuxArm32.YuvNV12ToYV12;
            downscaleImg = LinuxArm32.DownscaleImg;
            setConfig = LinuxArm32.ConverterSetConfig;
            getConfig = LinuxArm32.ConverterGetConfig;

            allocAllognedNative = LinuxArm32.AllocAllignedNative;
            freeAllognedNative = LinuxArm32.FreeAllignedNative;
        }

        private void LoadLinuxArm64Bindings()
        {
            // Encoder
            encoderEnableDebugLogs = LinuxArm64.EncoderEnableDebugLogs;
            getEncoder = LinuxArm64.GetEncoder;
            initializeEncoderBase = LinuxArm64.InitializeEncoderBase;
            initializeEncoder = LinuxArm64.InitializeEncoder;
            getDefaultParams = LinuxArm64.GetDefaultParams;
            initializeEncoder2 = LinuxArm64.InitializeEncoder2;
            encode = LinuxArm64.Encode;
            encode1 = LinuxArm64.Encode1;
            encode2 = LinuxArm64.Encode2;
            forceIntraFrame = LinuxArm64.ForceIntraFrame;
            setMaxBitrate = LinuxArm64.SetMaxBitrate;
            setTargetFps = LinuxArm64.SetTargetFps;
            freeEncoder = LinuxArm64.FreeEncoder;
            getOptionEncoder = LinuxArm64.GetOptionEncoder;
            setOptionEncoder = LinuxArm64.SetOptionEncoder;
            // Decoder
            decoderEnableDebugLogs = LinuxArm64.DecoderEnableDebugLogs;
            getDecoder = LinuxArm64.GetDecoder;
            initializeDecoderDefault = LinuxArm64.InitializeDecoderDefault;
            initializeDecoder = LinuxArm64.InitializeDecoder;
            decodeAsYUV = LinuxArm64.DecodeAsYUV;
            decodeAsYUVext = LinuxArm64.DecodeAsYUVExt;
            decodeRgbInto = LinuxArm64.DecodeRgbInto;
            freeDecoder = LinuxArm64.FreeDecoder;
            setParallelConverterDec = LinuxArm64.SetParallelConverterDec;
            getOptionDecoder = LinuxArm64.GetOptionDecoder;
            setOptionDecoder = LinuxArm64.SetOptionDecoder;
            // Converter
            rGBXtoYUV = LinuxArm64.RGBXtoYUV;
            yUV2RGB = LinuxArm64.YUV2RGB;
            YuvNV12ToRGB = LinuxArm64.YUVNV122RGB;
            YuvNV12ToYV12 = LinuxArm64.YuvNV12ToYV12;
            downscaleImg = LinuxArm64.DownscaleImg;
            setConfig = LinuxArm64.ConverterSetConfig;
            getConfig = LinuxArm64.ConverterGetConfig;

            allocAllognedNative = LinuxArm64.AllocAllignedNative;
            freeAllognedNative = LinuxArm64.FreeAllignedNative;
        }

        private void LoadAndroidArm64Bindings()
        {
            // Encoder
            encoderEnableDebugLogs = AndroidArm64.EncoderEnableDebugLogs;
            getEncoder = AndroidArm64.GetEncoder;
            initializeEncoderBase = AndroidArm64.InitializeEncoderBase;
            initializeEncoder = AndroidArm64.InitializeEncoder;
            getDefaultParams = AndroidArm64.GetDefaultParams;
            initializeEncoder2 = AndroidArm64.InitializeEncoder2;
            encode = AndroidArm64.Encode;
            encode1 = AndroidArm64.Encode1;
            encode2 = AndroidArm64.Encode2;
            forceIntraFrame = AndroidArm64.ForceIntraFrame;
            setMaxBitrate = AndroidArm64.SetMaxBitrate;
            setTargetFps = AndroidArm64.SetTargetFps;
            freeEncoder = AndroidArm64.FreeEncoder;
            getOptionEncoder = AndroidArm64.GetOptionEncoder;
            setOptionEncoder = AndroidArm64.SetOptionEncoder;
            // Decoder
            decoderEnableDebugLogs = AndroidArm64.DecoderEnableDebugLogs;
            getDecoder = AndroidArm64.GetDecoder;
            initializeDecoderDefault = AndroidArm64.InitializeDecoderDefault;
            initializeDecoder = AndroidArm64.InitializeDecoder;
            decodeAsYUV = AndroidArm64.DecodeAsYUV;
            decodeAsYUVext = AndroidArm64.DecodeAsYUVExt;
            decodeRgbInto = AndroidArm64.DecodeRgbInto;
            freeDecoder = AndroidArm64.FreeDecoder;
            setParallelConverterDec = AndroidArm64.SetParallelConverterDec;
            getOptionDecoder = AndroidArm64.GetOptionDecoder;
            setOptionDecoder = AndroidArm64.SetOptionDecoder;
            // Converter
            rGBXtoYUV = AndroidArm64.RGBXtoYUV;
            yUV2RGB = AndroidArm64.YUV2RGB;
            YuvNV12ToRGB = AndroidArm64.YUVNV122RGB;
            YuvNV12ToYV12 = AndroidArm64.YuvNV12ToYV12;
            downscaleImg = AndroidArm64.DownscaleImg;
            setConfig = AndroidArm64.ConverterSetConfig;
            getConfig = AndroidArm64.ConverterGetConfig;

            allocAllognedNative = AndroidArm64.AllocAllignedNative;
            freeAllognedNative = AndroidArm64.FreeAllignedNative;
        }

        private void LoadAndroidArmBindings()
        {
            encoderEnableDebugLogs = AndroidArm32.EncoderEnableDebugLogs;
            getEncoder = AndroidArm32.GetEncoder;
            initializeEncoderBase = AndroidArm32.InitializeEncoderBase;
            initializeEncoder = AndroidArm32.InitializeEncoder;
            getDefaultParams = AndroidArm32.GetDefaultParams;
            initializeEncoder2 = AndroidArm32.InitializeEncoder2;
            encode = AndroidArm32.Encode;
            encode1 = AndroidArm32.Encode1;
            encode2 = AndroidArm32.Encode2;
            forceIntraFrame = AndroidArm32.ForceIntraFrame;
            setMaxBitrate = AndroidArm32.SetMaxBitrate;
            setTargetFps = AndroidArm32.SetTargetFps;
            freeEncoder = AndroidArm32.FreeEncoder;
            getOptionEncoder = AndroidArm32.GetOptionEncoder;
            setOptionEncoder = AndroidArm32.SetOptionEncoder;
            // Decoder
            decoderEnableDebugLogs = AndroidArm32.DecoderEnableDebugLogs;
            getDecoder = AndroidArm32.GetDecoder;
            initializeDecoderDefault = AndroidArm32.InitializeDecoderDefault;
            initializeDecoder = AndroidArm32.InitializeDecoder;
            decodeAsYUV = AndroidArm32.DecodeAsYUV;
            decodeAsYUVext = AndroidArm32.DecodeAsYUVExt;
            decodeRgbInto = AndroidArm32.DecodeRgbInto;
            freeDecoder = AndroidArm32.FreeDecoder;
            setParallelConverterDec = AndroidArm32.SetParallelConverterDec;
            getOptionDecoder = AndroidArm32.GetOptionDecoder;
            setOptionDecoder = AndroidArm32.SetOptionDecoder;
            // Converter
            rGBXtoYUV = AndroidArm32.RGBXtoYUV;
            yUV2RGB = AndroidArm32.YUV2RGB;
            YuvNV12ToRGB = AndroidArm32.YUVNV122RGB;
            YuvNV12ToYV12 = AndroidArm32.YuvNV12ToYV12;
            downscaleImg = AndroidArm32.DownscaleImg;
            setConfig = AndroidArm32.ConverterSetConfig;
            getConfig = AndroidArm32.ConverterGetConfig;

            allocAllognedNative = AndroidArm32.AllocAllignedNative;
            freeAllognedNative = AndroidArm32.FreeAllignedNative;
        }

        private void LoadAndroidx64Bindings()
        {
            encoderEnableDebugLogs = Androidx64.EncoderEnableDebugLogs;
            getEncoder = Androidx64.GetEncoder;
            initializeEncoderBase = Androidx64.InitializeEncoderBase;
            initializeEncoder = Androidx64.InitializeEncoder;
            getDefaultParams = Androidx64.GetDefaultParams;
            initializeEncoder2 = Androidx64.InitializeEncoder2;
            encode = Androidx64.Encode;
            encode1 = Androidx64.Encode1;
            encode2 = Androidx64.Encode2;
            forceIntraFrame = Androidx64.ForceIntraFrame;
            setMaxBitrate = Androidx64.SetMaxBitrate;
            setTargetFps = Androidx64.SetTargetFps;
            freeEncoder = Androidx64.FreeEncoder;
            getOptionEncoder = Androidx64.GetOptionEncoder;
            setOptionEncoder = Androidx64.SetOptionEncoder;
            // Decoder
            decoderEnableDebugLogs = Androidx64.DecoderEnableDebugLogs;
            getDecoder = Androidx64.GetDecoder;
            initializeDecoderDefault = Androidx64.InitializeDecoderDefault;
            initializeDecoder = Androidx64.InitializeDecoder;
            decodeAsYUV = Androidx64.DecodeAsYUV;
            decodeAsYUVext = Androidx64.DecodeAsYUVExt;
            decodeRgbInto = Androidx64.DecodeRgbInto;
            freeDecoder = Androidx64.FreeDecoder;
            setParallelConverterDec = Androidx64.SetParallelConverterDec;
            getOptionDecoder = Androidx64.GetOptionDecoder;
            setOptionDecoder = Androidx64.SetOptionDecoder;
            // Converter
            rGBXtoYUV = Androidx64.RGBXtoYUV;
            yUV2RGB = Androidx64.YUV2RGB;
            YuvNV12ToRGB = Androidx64.YUVNV122RGB;
            YuvNV12ToYV12 = Androidx64.YuvNV12ToYV12;
            downscaleImg = Androidx64.DownscaleImg;
            setConfig = Androidx64.ConverterSetConfig;
            getConfig = Androidx64.ConverterGetConfig;

            allocAllognedNative = Androidx64.AllocAllignedNative;
            freeAllognedNative = Androidx64.FreeAllignedNative;
        }

        #region Interface

        internal IntPtr GetEncoder(string dllName)
                     => getEncoder(dllName);
        internal int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param)
                  => initializeEncoderBase(encoder, param);
        internal int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType)
                  => initializeEncoder(encoder, width, height, bps, fps, configType);
        internal int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param)
                  => getDefaultParams(encoder, ref param);
        internal int InitializeEncoder2(IntPtr encoder, TagEncParamExt param)
                   => initializeEncoder2(encoder, param);
        internal int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc)
                  => encode(encoder, ref s, ref fc);
        internal int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc)
                  => encode1(encoder, ref yuv, ref fc);
        internal int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc)
                 => encode2(encoder, ref yuv, ref fc);
        internal int ForceIntraFrame(IntPtr encoder)
                  => forceIntraFrame(encoder);
        internal void SetMaxBitrate(IntPtr encoder, int target)
                   => setMaxBitrate(encoder, target);
        internal void SetTargetFps(IntPtr encoder, float target)
                   => setTargetFps(encoder, target);
        internal void FreeEncoder(IntPtr encoder)
                   => freeEncoder(encoder);
        internal int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value)
                  => getOptionEncoder(encoder, option, value);
        internal int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value)
                   => setOptionEncoder(encoder, option, value);
        internal void EncoderEnableDebugLogs(int val)
                 => encoderEnableDebugLogs(val);
        // Decoder

        internal IntPtr GetDecoder(string s)
                     => getDecoder(s);
        internal int InitializeDecoderDefault(IntPtr dec)
                  => initializeDecoderDefault(dec);
        internal int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param)
                  => initializeDecoder(dec, param);
        internal int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded)
                   => decodeAsYUV(decoder, ref frame, lenght, noDelay, ref state, ref decoded);
        internal int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded)
                  => decodeAsYUVext(decoder, ref frame, lenght, noDelay, ref state, ref decoded);
        internal unsafe bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer)
                          => decodeRgbInto(decoder, ref frame, lenght, noDelay, ref state, buffer);
        internal void FreeDecoder(IntPtr decoder)
                   => freeDecoder(decoder);
        internal int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value)
                  => getOptionDecoder(decoder, option, value);
        internal int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value)
                  => setOptionDecoder(decoder, option, value);
        internal void DecoderEnableDebugLogs(int val)
                  => decoderEnableDebugLogs(val);
        // Converter
        internal void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv)
                   => rGBXtoYUV(ref rgb, ref yuv);
        internal void YUV2RGB(ref YUVImagePointer yuv, ref UnsafeGenericRgbImage rgb)
                   => yUV2RGB(ref yuv, ref rgb);
        internal void YUVNV12ToRGB(ref YUVNV12ImagePointer nv12, ref UnsafeGenericRgbImage yv12)
                 => YuvNV12ToRGB(ref nv12, ref yv12);
        internal void YUVNV12ToYV12(ref YUVNV12ImagePointer nv12, ref YUVImagePointer yv12)
                  => YuvNV12ToYV12(ref nv12, ref yv12);
        internal void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul)
                   => downscaleImg(ref from, ref to, mul);
        internal void ConverterGetConfig(ref ConverterConfig c)
                  => getConfig(ref c);
        internal void ConverterSetConfig(ConverterConfig val)
                  => setConfig(val);
        internal IntPtr AllocAllignedNative( int size)
                   => allocAllognedNative(size);
        internal void FreeAllignedNative(IntPtr p)
                  => freeAllognedNative(p);

        #endregion
    }

    public unsafe class Winx86
    {
        const string DllName = Defines.WrapperDllWinx86;

        // Encoder
        [DllImport(DllName, EntryPoint = "EncoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EncoderEnableDebugLogs(int val);
        [DllImport(DllName, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetEncoder(string dllName);

        [DllImport(DllName, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(DllName, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(DllName, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(DllName, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeEncoder(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        // Decoder

        [DllImport(DllName, EntryPoint = "DecoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DecoderEnableDebugLogs(int val);

        [DllImport(DllName, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetDecoder(string s);

        [DllImport(DllName, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(DllName, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(DllName, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsYUVExt", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        internal static unsafe extern bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(DllName, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeDecoder(IntPtr decoder);

        [DllImport(DllName, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(DllName, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        // Converter

        [DllImport(DllName, EntryPoint = "RGBX2YUV420", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);

        [DllImport(DllName, EntryPoint = "YUV420ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUV2RGB(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUVNV122RGB(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToYV12", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void YuvNV12ToYV12(ref YUVNV12ImagePointer from, ref YUVImagePointer to);

        [DllImport(DllName, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);

        [DllImport(DllName, EntryPoint = "ConverterSetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterSetConfig(ConverterConfig conf);

        [DllImport(DllName, EntryPoint = "AllocAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr AllocAllignedNative(int size);

        [DllImport(DllName, EntryPoint = "FreeAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeAllignedNative(IntPtr p);

        [DllImport(DllName, EntryPoint = "ConverterGetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterGetConfig(ref ConverterConfig conf);
    }

    public unsafe class Winx64
    {
        const string DllName = Defines.WrapperDllWinx64;

        // Encoder
        [DllImport(DllName, EntryPoint = "EncoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EncoderEnableDebugLogs(int val);
        [DllImport(DllName, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetEncoder(string dllName);

        [DllImport(DllName, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(DllName, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(DllName, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(DllName, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeEncoder(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        // Decoder

        [DllImport(DllName, EntryPoint = "DecoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DecoderEnableDebugLogs(int val);

        [DllImport(DllName, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetDecoder(string s);

        [DllImport(DllName, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(DllName, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);


        [DllImport(DllName, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsYUVExt", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        internal static unsafe extern bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(DllName, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeDecoder(IntPtr decoder);

        [DllImport(DllName, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(DllName, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        // Converter

        [DllImport(DllName, EntryPoint = "RGBX2YUV420", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);

        [DllImport(DllName, EntryPoint = "YUV420ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUV2RGB(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUVNV122RGB(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToYV12", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void YuvNV12ToYV12(ref YUVNV12ImagePointer from, ref YUVImagePointer to);

        [DllImport(DllName, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);

        [DllImport(DllName, EntryPoint = "ConverterSetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterSetConfig(ConverterConfig conf);

        [DllImport(DllName, EntryPoint = "AllocAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr AllocAllignedNative(int size);

        [DllImport(DllName, EntryPoint = "FreeAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeAllignedNative(IntPtr p);

        [DllImport(DllName, EntryPoint = "ConverterGetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterGetConfig(ref ConverterConfig conf);
    }

    public unsafe class Linuxx86
    {
        const string DllName = Defines.WrapperDllLinuxx86;

        // Encoder
        [DllImport(DllName, EntryPoint = "EncoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EncoderEnableDebugLogs(int val);
        [DllImport(DllName, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetEncoder(string dllName);

        [DllImport(DllName, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(DllName, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(DllName, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(DllName, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeEncoder(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        // Decoder

        [DllImport(DllName, EntryPoint = "DecoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DecoderEnableDebugLogs(int val);

        [DllImport(DllName, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetDecoder(string s);

        [DllImport(DllName, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(DllName, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(DllName, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsYUVExt", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        internal static unsafe extern bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(DllName, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeDecoder(IntPtr decoder);

        [DllImport(DllName, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(DllName, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        // Converter

        [DllImport(DllName, EntryPoint = "RGBX2YUV420", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);

        [DllImport(DllName, EntryPoint = "YUV420ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUV2RGB(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUVNV122RGB(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToYV12", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void YuvNV12ToYV12(ref YUVNV12ImagePointer from, ref YUVImagePointer to);

        [DllImport(DllName, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);

        [DllImport(DllName, EntryPoint = "ConverterSetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterSetConfig(ConverterConfig conf);

        [DllImport(DllName, EntryPoint = "AllocAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr AllocAllignedNative(int size);

        [DllImport(DllName, EntryPoint = "FreeAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeAllignedNative(IntPtr p);

        [DllImport(DllName, EntryPoint = "ConverterGetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterGetConfig(ref ConverterConfig conf);
    }

    public unsafe class Linuxx64
    {
        const string DllName = Defines.WrapperDllLinuxx64;

        // Encoder
        [DllImport(DllName, EntryPoint = "EncoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EncoderEnableDebugLogs(int val);
        [DllImport(DllName, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetEncoder(string dllName);

        [DllImport(DllName, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(DllName, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(DllName, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(DllName, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeEncoder(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        // Decoder

        [DllImport(DllName, EntryPoint = "DecoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DecoderEnableDebugLogs(int val);

        [DllImport(DllName, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetDecoder(string s);

        [DllImport(DllName, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(DllName, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(DllName, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsYUVExt", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        internal static unsafe extern bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(DllName, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeDecoder(IntPtr decoder);

        [DllImport(DllName, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(DllName, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        // Converter

        [DllImport(DllName, EntryPoint = "RGBX2YUV420", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);

        [DllImport(DllName, EntryPoint = "YUV420ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUV2RGB(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUVNV122RGB(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToYV12", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void YuvNV12ToYV12(ref YUVNV12ImagePointer from, ref YUVImagePointer to);

        [DllImport(DllName, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);

        [DllImport(DllName, EntryPoint = "ConverterSetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterSetConfig(ConverterConfig conf);

        [DllImport(DllName, EntryPoint = "AllocAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr AllocAllignedNative(int size);

        [DllImport(DllName, EntryPoint = "FreeAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeAllignedNative(IntPtr p);

        [DllImport(DllName, EntryPoint = "ConverterGetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterGetConfig(ref ConverterConfig conf);
    }

    public unsafe class LinuxArm32
    {
        const string DllName = Defines.WrapperDllLinuxArm32;

        // Encoder
        [DllImport(DllName, EntryPoint = "EncoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EncoderEnableDebugLogs(int val);
        [DllImport(DllName, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetEncoder(string dllName);

        [DllImport(DllName, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(DllName, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(DllName, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(DllName, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeEncoder(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        // Decoder

        [DllImport(DllName, EntryPoint = "DecoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DecoderEnableDebugLogs(int val);

        [DllImport(DllName, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetDecoder(string s);

        [DllImport(DllName, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(DllName, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(DllName, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsYUVExt", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        internal static unsafe extern bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(DllName, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeDecoder(IntPtr decoder);

        [DllImport(DllName, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(DllName, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        // Converter

        [DllImport(DllName, EntryPoint = "RGBX2YUV420", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);

        [DllImport(DllName, EntryPoint = "YUV420ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUV2RGB(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUVNV122RGB(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToYV12", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void YuvNV12ToYV12(ref YUVNV12ImagePointer from, ref YUVImagePointer to);

        [DllImport(DllName, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);

        [DllImport(DllName, EntryPoint = "ConverterSetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterSetConfig(ConverterConfig conf);

        [DllImport(DllName, EntryPoint = "AllocAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr AllocAllignedNative(int size);

        [DllImport(DllName, EntryPoint = "FreeAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeAllignedNative(IntPtr p);

        [DllImport(DllName, EntryPoint = "ConverterGetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterGetConfig(ref ConverterConfig conf);
    }

    public unsafe class LinuxArm64
    {
        const string DllName = Defines.WrapperDllLinuxArm64;

        // Encoder
        [DllImport(DllName, EntryPoint = "EncoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EncoderEnableDebugLogs(int val);
        [DllImport(DllName, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetEncoder(string dllName);

        [DllImport(DllName, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(DllName, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(DllName, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(DllName, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeEncoder(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        // Decoder

        [DllImport(DllName, EntryPoint = "DecoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DecoderEnableDebugLogs(int val);

        [DllImport(DllName, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetDecoder(string s);

        [DllImport(DllName, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(DllName, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(DllName, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsYUVExt", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        internal static unsafe extern bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(DllName, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeDecoder(IntPtr decoder);

        [DllImport(DllName, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(DllName, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        // Converter

        [DllImport(DllName, EntryPoint = "RGBX2YUV420", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);

        [DllImport(DllName, EntryPoint = "YUV420ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUV2RGB(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUVNV122RGB(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToYV12", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void YuvNV12ToYV12(ref YUVNV12ImagePointer from, ref YUVImagePointer to);

        [DllImport(DllName, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);

        [DllImport(DllName, EntryPoint = "ConverterSetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterSetConfig(ConverterConfig conf);

        [DllImport(DllName, EntryPoint = "AllocAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr AllocAllignedNative(int size);

        [DllImport(DllName, EntryPoint = "FreeAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeAllignedNative(IntPtr p);

        [DllImport(DllName, EntryPoint = "ConverterGetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterGetConfig(ref ConverterConfig conf);
    }

    public unsafe class AndroidArm64
    {
        const string DllName = Defines.WrapperDllAndroidArm64;

        // Encoder
        [DllImport(DllName, EntryPoint = "EncoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EncoderEnableDebugLogs(int val);
        [DllImport(DllName, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetEncoder(string dllName);

        [DllImport(DllName, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(DllName, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(DllName, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(DllName, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeEncoder(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        // Decoder

        [DllImport(DllName, EntryPoint = "DecoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DecoderEnableDebugLogs(int val);

        [DllImport(DllName, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetDecoder(string s);

        [DllImport(DllName, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(DllName, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(DllName, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsYUVExt", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        internal static unsafe extern bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(DllName, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeDecoder(IntPtr decoder);

        [DllImport(DllName, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(DllName, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        // Converter

        [DllImport(DllName, EntryPoint = "RGBX2YUV420", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);

        [DllImport(DllName, EntryPoint = "YUV420ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUV2RGB(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUVNV122RGB(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToYV12", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void YuvNV12ToYV12(ref YUVNV12ImagePointer from, ref YUVImagePointer to);

        [DllImport(DllName, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);

        [DllImport(DllName, EntryPoint = "ConverterSetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterSetConfig(ConverterConfig conf);

        [DllImport(DllName, EntryPoint = "AllocAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr AllocAllignedNative(int size);

        [DllImport(DllName, EntryPoint = "FreeAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeAllignedNative(IntPtr p);

        [DllImport(DllName, EntryPoint = "ConverterGetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterGetConfig(ref ConverterConfig conf);
    }

    public unsafe class AndroidArm32
    {
        const string DllName = Defines.WrapperDllAndroidArm32;

        // Encoder
        [DllImport(DllName, EntryPoint = "EncoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EncoderEnableDebugLogs(int val);
        [DllImport(DllName, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetEncoder(string dllName);

        [DllImport(DllName, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(DllName, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(DllName, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(DllName, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeEncoder(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        // Decoder

        [DllImport(DllName, EntryPoint = "DecoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DecoderEnableDebugLogs(int val);

        [DllImport(DllName, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetDecoder(string s);

        [DllImport(DllName, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(DllName, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(DllName, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsYUVExt", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        internal static unsafe extern bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(DllName, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeDecoder(IntPtr decoder);

        [DllImport(DllName, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(DllName, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        // Converter

        [DllImport(DllName, EntryPoint = "RGBX2YUV420", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);

        [DllImport(DllName, EntryPoint = "YUV420ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUV2RGB(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUVNV122RGB(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToYV12", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void YuvNV12ToYV12(ref YUVNV12ImagePointer from, ref YUVImagePointer to);

        [DllImport(DllName, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);

        [DllImport(DllName, EntryPoint = "ConverterSetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterSetConfig(ConverterConfig conf);

        [DllImport(DllName, EntryPoint = "AllocAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr AllocAllignedNative(int size);

        [DllImport(DllName, EntryPoint = "FreeAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeAllignedNative(IntPtr p);

        [DllImport(DllName, EntryPoint = "ConverterGetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterGetConfig(ref ConverterConfig conf);
    }

    public unsafe class Androidx64
    {
        const string DllName = Defines.WrapperDllAndroidx64;

        // Encoder
        [DllImport(DllName, EntryPoint = "EncoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void EncoderEnableDebugLogs(int val);
        [DllImport(DllName, EntryPoint = "GetEncoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetEncoder(string dllName);

        [DllImport(DllName, EntryPoint = "InitializeEncoderBase", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoderBase(IntPtr encoder, TagEncParamBase param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder(IntPtr encoder, int width, int height, int bps, int fps, int configType);

        [DllImport(DllName, EntryPoint = "GetDefaultParams", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetDefaultParams(IntPtr encoder, ref TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "InitializeEncoder2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeEncoder2(IntPtr encoder, TagEncParamExt param);

        [DllImport(DllName, EntryPoint = "Encode", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode(IntPtr encoder, ref UnsafeGenericRgbImage s, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode1", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode1(IntPtr encoder, ref YUVImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "Encode2", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Encode2(IntPtr encoder, ref YUVNV12ImagePointer yuv, ref FrameContainer fc);

        [DllImport(DllName, EntryPoint = "ForceIntraFrame", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ForceIntraFrame(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "SetMaxBitrate", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetMaxBitrate(IntPtr encoder, int target);

        [DllImport(DllName, EntryPoint = "SetTargetFps", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetTargetFps(IntPtr encoder, float target);

        [DllImport(DllName, EntryPoint = "FreeEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeEncoder(IntPtr encoder);

        [DllImport(DllName, EntryPoint = "GetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionEncoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionEncoder(IntPtr encoder, ENCODER_OPTION option, IntPtr value);

        // Decoder

        [DllImport(DllName, EntryPoint = "DecoderEnableDebugLogs", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DecoderEnableDebugLogs(int val);

        [DllImport(DllName, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        internal static extern IntPtr GetDecoder(string s);

        [DllImport(DllName, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoderDefault(IntPtr dec);

        [DllImport(DllName, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int InitializeDecoder(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(DllName, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUV(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsYUVExt", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int DecodeAsYUVExt(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(DllName, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        internal static unsafe extern bool DecodeRgbInto(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(DllName, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeDecoder(IntPtr decoder);

        [DllImport(DllName, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void SetParallelConverterDec(IntPtr decoder, int isParallel);

        [DllImport(DllName, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int GetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(DllName, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int SetOptionDecoder(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        // Converter

        [DllImport(DllName, EntryPoint = "RGBX2YUV420", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void RGBXtoYUV(ref UnsafeGenericRgbImage rgb, ref YUVImagePointer yuv);

        [DllImport(DllName, EntryPoint = "YUV420ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUV2RGB(ref YUVImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToRGB", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        internal static extern void YUVNV122RGB(ref YUVNV12ImagePointer rgb, ref UnsafeGenericRgbImage yuv);

        [DllImport(DllName, EntryPoint = "YUVNV12ToYV12", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void YuvNV12ToYV12(ref YUVNV12ImagePointer from, ref YUVImagePointer to);

        [DllImport(DllName, EntryPoint = "DownscaleImg", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void DownscaleImg(ref UnsafeGenericRgbImage from, ref UnsafeGenericRgbImage to, int mul);

        [DllImport(DllName, EntryPoint = "ConverterSetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterSetConfig(ConverterConfig conf);

        [DllImport(DllName, EntryPoint = "AllocAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr AllocAllignedNative(int size);

        [DllImport(DllName, EntryPoint = "FreeAlligned", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void FreeAllignedNative(IntPtr p);

        [DllImport(DllName, EntryPoint = "ConverterGetConfig", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ConverterGetConfig(ref ConverterConfig conf);
    }


}
