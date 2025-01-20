#ifndef TRANSCODER_FACTORY
#define TRANSCODER_FACTORY
#include "Encoder.h"
#include "Decoder.h"
using namespace H264Sharp;

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define DLL_EXPORT __attribute__((dllexport))
#else
#define DLL_EXPORT __declspec(dllexport)
#endif
#else
#if __GNUC__ >= 4
#define DLL_EXPORT __attribute__((visibility("default")))
#else
#define DLL_EXPORT
#endif
#endif
    using namespace H264Sharp;
    //----------------------Encoder--------------------------------------------------------

    DLL_EXPORT int Hello() {
        return 42;
    }

    DLL_EXPORT H264Sharp::Encoder* GetEncoder(const char* dllname) {
        return new H264Sharp::Encoder(dllname);
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

    DLL_EXPORT bool Encode(Encoder* encoder, GenericImage* img, FrameContainer* fc) {
        return encoder->Encode(*img, *fc);
    }

    DLL_EXPORT bool Encode1(Encoder* encoder, byte* yuv, FrameContainer* fc) {
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

    DLL_EXPORT void SetParallelConverterEnc(Encoder* encoder, int threadCount) {
        encoder->threadCount = threadCount;
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

    //----------------------Decoder--------------------------------------------------------

    DLL_EXPORT Decoder* GetDecoder(const char* dllname) {
        return new Decoder(dllname);
    }

    DLL_EXPORT int InitializeDecoderDefault(Decoder* decoder) {
        return decoder->Initialize();
    }

    DLL_EXPORT int InitializeDecoder(Decoder* decoder, SDecodingParam decParam) {
        return decoder->Initialize(decParam);
    }

    DLL_EXPORT bool DecodeAsYUV(Decoder* decoder, unsigned char* frame, int length, bool noDelay, DecodingState* state, YuvNative* decoded) {
        return decoder->Decode(frame, length, noDelay, *state, *decoded);
    }

    DLL_EXPORT bool DecodeAsRGB(Decoder* decoder, unsigned char* frame, int length, bool noDelay, DecodingState* state, RgbImage* decoded) {
        return decoder->Decode(frame, length, noDelay, *state, *decoded);
    }

    DLL_EXPORT bool DecodeAsRGBInto(Decoder* decoder, unsigned char* frame, int length, bool noDelay, DecodingState* state, unsigned char* decoded) {
        return decoder->DecodeExt(frame, length, noDelay, *state, decoded);
    }

    DLL_EXPORT bool DecodeAsRGBInto2(int a) {
        return true;
    }

    DLL_EXPORT void FreeDecoder(Decoder* decoder) {
        delete decoder;
    }

    DLL_EXPORT void SetParallelConverterDec(Decoder* decoder, int threadCount) {
        decoder->threadCount = threadCount;
    }

    DLL_EXPORT void UseSSEYUVConverter(Decoder* decoder, bool isSSE) {
        decoder->UseSSEConverter(isSSE);
    }

    DLL_EXPORT int SetOptionDecoder(Decoder* decoder, DECODER_OPTION option, void* value) {
        return decoder->SetOption(option, value);
    }

    DLL_EXPORT int GetOptionDecoder(Decoder* decoder, DECODER_OPTION option, void* value) {
        return decoder->GetOption(option, value);
    }

    //-----

    /*DLL_EXPORT void YUV420ToRGB(YuvNative* from, RgbImage* to, int threadCount) {
        Converter::Yuv420PtoRGB(*from,to->ImageBytes, true, threadCount);
    }*/
    DLL_EXPORT void YUV420ToRGB(YuvNative* from, RgbImage* to, int threadCount) {
        Converter::Yuv420PtoRGB(to->ImageBytes, from->Y, from->U, from->V, to->Width, to->Height, from->yStride, from->uvStride, to->Width * 3, true, threadCount);
    }

    DLL_EXPORT void RGBX2YUV420(GenericImage* from, YuvNative* to, int threadCount) {
        switch (from->Type)
        {
        case ImageType::Rgb:
            Converter::RGBtoYUV420Planar(from->ImageBytes, to->Y, from->Width, from->Height, from->Stride, threadCount);
            break;
        case ImageType::Bgr:
            Converter::BGRtoYUV420Planar(from->ImageBytes, to->Y, from->Width, from->Height, from->Stride, threadCount);
            break;
        case ImageType::Rgba:
            Converter::RGBAtoYUV420Planar(from->ImageBytes, to->Y, from->Width, from->Height, from->Stride, threadCount);
            break;
        case ImageType::Bgra:
            Converter::BGRAtoYUV420Planar(from->ImageBytes, to->Y, from->Width, from->Height, from->Stride, threadCount);
            break;
        default:
            break;

        }
    }

   

    DLL_EXPORT void DownscaleImg(GenericImage* from, GenericImage* to, int multiplier) {
        ImageType imtype = from->Type;
        switch (imtype) {
        case ImageType::Rgb:
        case ImageType::Bgr:
            Converter::Downscale24(from->ImageBytes, from->Width, from->Height, from->Stride, to->ImageBytes, multiplier);
            break;

        default:
            Converter::Downscale32(from->ImageBytes, from->Width, from->Height, from->Stride, to->ImageBytes, multiplier);
            break;
        }
    }

#ifdef __cplusplus
}
#endif


#endif

