#pragma once
#include "pch.h";
#include "Encoder.h"
#include "Decoder.h"
using namespace H264Sharp;


	
	extern "C" __declspec(dllexport)  int __stdcall Hello()
	{
		return 42;
	}

	extern "C" __declspec(dllexport)  H264Sharp::Encoder * __stdcall GetEncoder(const wchar_t* dllname)
	{
		return new H264Sharp::Encoder(dllname);
	}

	extern "C" __declspec(dllexport)  void __cdecl InitializeEncoder(Encoder* encoder,int width, int height, int bps, int fps, int configType)
	{
		encoder->Initialize(width, height, bps, fps, static_cast<ConfigType>(configType));
	}
	
	extern "C" __declspec(dllexport)  bool __stdcall Encode(Encoder * encoder, GenericImage* img, FrameContainer* fc)
	{
		return encoder->Encode(*img, *fc);
	}

	extern "C" __declspec(dllexport)  bool __stdcall Encode1(Encoder * encoder, byte * yuv, FrameContainer * fc)
	{
		return encoder->Encode(yuv, *fc);
	}

	extern "C" __declspec(dllexport)  int __stdcall ForceIntraFrame(Encoder * encoder)
	{
		return encoder->ForceIntraFrame();
	}

	extern "C" __declspec(dllexport)  void __stdcall SetMaxBitrate(Encoder * encoder, int target)
	{
		 encoder->SetMaxBitrate(target);
	}

	extern "C" __declspec(dllexport)  void __stdcall SetTargetFps(Encoder * encoder, float target)
	{
		encoder->SetTargetFps(target);
	}
	
	extern "C" __declspec(dllexport)  void __stdcall FreeEncoder(Encoder * encoder)
	{
		delete encoder;
	}
	//-----------------------------------------------------------------------------------

	
	extern "C" __declspec(dllexport)  H264Sharp::Decoder * __cdecl GetDecoder(const wchar_t* dllname)
	{
		return new H264Sharp::Decoder(dllname);
	}

	extern "C" __declspec(dllexport)  bool __stdcall 
		DecodeAsYUV(Decoder* decoder, unsigned char* frame,int lenght, bool noDelay, DecodingState* state, Yuv420p* decoded)
	{
		return decoder->Decode(frame, lenght, noDelay, *state, *decoded);
	}

	extern "C" __declspec(dllexport)  bool __stdcall
		DecodeAsRGB(Decoder * decoder, unsigned char* frame, int lenght, bool noDelay, DecodingState * state, RgbImage * decoded)
	{
		return decoder->Decode(frame, lenght, noDelay, *state, *decoded);
	}

	extern "C" __declspec(dllexport)  void __stdcall FreeDecoder(Decoder * decoder)
	{
		delete decoder;
	}

