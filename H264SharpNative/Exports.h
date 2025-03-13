#ifndef TRANSCODER_FACTORY
#define TRANSCODER_FACTORY
#include "Encoder.h"
#include "Decoder.h"
using namespace H264Sharp;

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(__GNUC__) || defined(__clang__)
#define EXPORT_API __attribute__((dllexport))
#else
#define EXPORT_API __declspec(dllexport)
#endif
#else
#if (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
#define EXPORT_API __attribute__((visibility("default")))
#else
#define EXPORT_API
#endif
#endif


#define DLL_EXPORT EXTERN_C EXPORT_API


    //----------------------Encoder--------------------------------------------------------

    DLL_EXPORT int Hello() {
        return 42;
    }

    DLL_EXPORT Encoder* GetEncoder(const char* dllname, int* error)
    {
        auto encoder = new Encoder();
        *error = encoder->LoadCisco(dllname);
        if (*error > 0)
        {
            delete encoder;
            return nullptr;
        }
        return encoder;
    }

    DLL_EXPORT int InitializeEncoder(Encoder* encoder, int width, int height, int bps, int fps, int configType) {
        return encoder->Initialize(width, height, bps, fps, static_cast<ConfigType>(configType));
    }

    DLL_EXPORT int GetDefaultParams(Encoder* encoder, SEncParamExt* params) {
        return encoder->GetDefaultParams(*params);
    }

    DLL_EXPORT int InitializeEncoderBase(Encoder* encoder, SEncParamBase params) {
        return encoder->Initialize(params);
    }

    DLL_EXPORT int InitializeEncoder2(Encoder* encoder, SEncParamExt params) {
        return encoder->Initialize(params);
    }

    DLL_EXPORT int Encode(Encoder* encoder, GenericImage* img, FrameContainer* fc) {
        return encoder->Encode(*img, *fc);
    }

    DLL_EXPORT int Encode1(Encoder* encoder, YuvNative* yuv, FrameContainer* fc) {
        return encoder->Encode(yuv, *fc);
    }

    DLL_EXPORT int Encode2(Encoder* encoder, YuvNV12Native* yuv, FrameContainer* fc) {
        return encoder->Encode(yuv, *fc);
    }

    DLL_EXPORT int ForceIntraFrame(Encoder* encoder) {
        return encoder->ForceIntraFrame();
    }

    DLL_EXPORT void SetMaxBitrate(Encoder* encoder, int target) {
        encoder->SetMaxBitrate(target);
    }

    DLL_EXPORT void SetTargetFps(Encoder* encoder, float target) {
        encoder->SetTargetFps(target);
    }


    DLL_EXPORT void FreeEncoder(Encoder* encoder) {
        delete encoder;
    }

    DLL_EXPORT int SetOptionEncoder(Encoder* encoder, ENCODER_OPTION option, void* value) {
        return encoder->SetOption(option, value);
    }

    DLL_EXPORT int GetOptionEncoder(Encoder* encoder, ENCODER_OPTION option, void* value) {
        return encoder->GetOption(option, value);
    }
    DLL_EXPORT void EncoderEnableDebugLogs(int value) {
         Encoder::EnableDebugLogs = value;
       
    }

    //----------------------Decoder--------------------------------------------------------

    DLL_EXPORT Decoder* GetDecoder(const char* dllname, int* error)
    {
        auto decoder = new Decoder();
        *error = decoder->LoadCisco(dllname);
        if (*error > 0)
        {
            delete decoder;
            return nullptr;
        }
        return decoder;
    }

    DLL_EXPORT int InitializeDecoderDefault(Decoder* decoder) {
        return decoder->Initialize();
    }

    DLL_EXPORT int InitializeDecoder(Decoder* decoder, SDecodingParam decParam) {
        return decoder->Initialize(decParam);
    }

    DLL_EXPORT int DecodeAsYUV(Decoder* decoder, unsigned char* frame, int length, bool noDelay, DecodingState* state, YuvNative* decoded) {
        return decoder->Decode(frame, length, noDelay, *state, *decoded);
    }

    DLL_EXPORT int DecodeAsYUVExt(Decoder* decoder, unsigned char* frame, int length, bool noDelay, DecodingState* state, YuvNative* decoded) {
        return decoder->DecodeExt(frame, length, noDelay, *state, *decoded);
    }

    DLL_EXPORT void FreeDecoder(Decoder* decoder) {
        delete decoder;
    }

    DLL_EXPORT int SetOptionDecoder(Decoder* decoder, DECODER_OPTION option, void* value) {
        return decoder->SetOption(option, value);
    }

    DLL_EXPORT int GetOptionDecoder(Decoder* decoder, DECODER_OPTION option, void* value) {
        return decoder->GetOption(option, value);
    }
    DLL_EXPORT void DecoderEnableDebugLogs(int value) 
    {
        Decoder::EnableDebugLogs = value;
    }

    //-------------Coverter-------------------------------------------------------------

    DLL_EXPORT void ConverterGetConfig(ConverterConfig* config) 
    {
        *config = Converter::Config;
    }
   
    DLL_EXPORT void ConverterSetConfig(ConverterConfig config) 
    {
        Converter::SetConfig(config);
    }

   
    DLL_EXPORT void YUV420ToRGB(YuvNative* from, GenericImage* to)
    {
        switch (to->Type)
        {
            case ImageFormat::Rgb:
                Converter::Yuv420PtoRGB<3, true>(to->ImageBytes, from->Y, from->U, from->V, from->width, from->height, from->yStride, from->uvStride, to ->Stride);
                break;
            case ImageFormat::Bgr:
                Converter::Yuv420PtoRGB<3, false>(to->ImageBytes, from->Y, from->U, from->V, from->width, from->height, from->yStride, from->uvStride, to->Stride);
                break;
            case ImageFormat::Rgba:
                Converter::Yuv420PtoRGB<4, true>(to->ImageBytes, from->Y, from->U, from->V, from->width, from->height, from->yStride, from->uvStride, to->Stride);
                break;
            case ImageFormat::Bgra:
                Converter::Yuv420PtoRGB<4, false>(to->ImageBytes, from->Y, from->U, from->V, from->width, from->height, from->yStride, from->uvStride, to->Stride);
                break;
            default:
                break;

        }
    }

    DLL_EXPORT void RGBX2YUV420(GenericImage* from, YuvNative* to) {
        switch (from->Type)
        {
        case ImageFormat::Rgb:
            Converter::RGBXtoYUV420Planar<3,true>(from->ImageBytes, to->Y, from->Width, from->Height, from->Stride);
            break;
        case ImageFormat::Bgr:
            Converter::RGBXtoYUV420Planar<3, false>(from->ImageBytes, to->Y, from->Width, from->Height, from->Stride);
            break;
        case ImageFormat::Rgba:
            Converter::RGBXtoYUV420Planar<4, true>(from->ImageBytes, to->Y, from->Width, from->Height, from->Stride);
            break;
        case ImageFormat::Bgra:
            Converter::RGBXtoYUV420Planar<4, false>(from->ImageBytes, to->Y, from->Width, from->Height, from->Stride);
            break;
        default:
            break;

        }
    }

    DLL_EXPORT void YUVNV12ToYV12(YuvNV12Native* from, YuvNative* to)
    {
        Converter::Yuv_NV12ToYV12(*from, *to);
    }

    DLL_EXPORT void YUVNV12ToRGB(YuvNV12Native* from, GenericImage* to)
    {
        switch (to->Type)
        {
        case ImageFormat::Rgb:
            Converter::Yuv_NV12ToRGB<3, true>(*from, to->ImageBytes, to->Stride);
            break;
        case ImageFormat::Bgr:
            Converter::Yuv_NV12ToRGB<3, false>(*from, to->ImageBytes, to->Stride);
            break;
        case ImageFormat::Rgba:
            Converter::Yuv_NV12ToRGB<4, true>(*from, to->ImageBytes, to->Stride);
            break;
        case ImageFormat::Bgra:
            Converter::Yuv_NV12ToRGB<4, false>(*from, to->ImageBytes, to->Stride);
            break;
        default:
            break;
        }
        
    }

    DLL_EXPORT void* AllocAlligned(uint32_t size)
    {
        return AllignAlloc(size);
    }

    DLL_EXPORT void FreeAlligned(void* p)
    {
        FreeAllignAlloc(p);
    }

    DLL_EXPORT void DownscaleImg(GenericImage* from, GenericImage* to, int multiplier) {
        ImageFormat imtype = from->Type;
        switch (imtype) {
        case ImageFormat::Rgb:
        case ImageFormat::Bgr:
            Converter::Downscale24(from->ImageBytes, from->Width, from->Height, from->Stride, to->ImageBytes, multiplier);
            break;

        default:
            Converter::Downscale32(from->ImageBytes, from->Width, from->Height, from->Stride, to->ImageBytes, multiplier);
            break;
        }
    }




#endif

