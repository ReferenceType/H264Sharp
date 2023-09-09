#pragma once
#include "pch.h"
#using <System.Drawing.dll>
using namespace System;

namespace H264Sharp {

	typedef void (*Covert2Yuv420)(unsigned char* bgra, unsigned char* dst, int width, int height, int stride);
	static Covert2Yuv420 Bgra2Yuv420;
	static Covert2Yuv420 Bgr2Yuv420;
	static Covert2Yuv420 Rgba2Yuv420;
	static Covert2Yuv420 Rgb2Yuv420;

	public ref class Encoder
	{
		public:
			Encoder(String^ dllName);
			Encoder();

			enum class ConfigType { CameraBasic, ScreenCaptureBasic, CameraCaptureAdvanced, ScreenCaptureAdvanced };
			int Initialize(int width, int height, int bps, float fps, ConfigType configNo);

			bool Encode(System::Drawing::Bitmap ^bmp, array<EncodedFrame^>^% frame);
			bool Encode(BgrImage ^bgr, array<EncodedFrame^>^% frame);
			bool Encode(BgraImage ^bgra, array<EncodedFrame^>^% frame);
			bool Encode(RgbImage ^rgb, array<EncodedFrame^>^% frame);
			bool Encode(RgbaImage ^rgba, array<EncodedFrame^>^% frame);
			bool Encode(array<Byte> ^i420, int startIndex, array<EncodedFrame^>^% frame);
			bool Encode(unsigned char* i420, array<EncodedFrame^>^% frame);
			bool Encode(IntPtr^ i420, array<EncodedFrame^>^% frame);

			int ForceIntraFrame();
			void SetMaxBitrate(int target);
			void SetTargetFps(float target);

	private:
		int buffer_size;
		unsigned char* i420_buffer;
		unsigned char* innerBuffer;
		int innerBufLen;

		ISVCEncoder* encoder;
		SSourcePicture* pic;
		SFrameBSInfo* bsi;

		typedef int(__stdcall* WelsCreateSVCEncoder)(ISVCEncoder** ppEncoder);
		WelsCreateSVCEncoder CreateEncoderFunc;
		typedef void(__stdcall* WelsDestroySVCEncoder)(ISVCEncoder* ppEncoder);
		WelsDestroySVCEncoder DestroyEncoderFunc;

		void Create(const wchar_t* dllName);
		int InitializeInternal(int width, int height, int bps, float fps, ConfigType configType);
		void EnsureCapacity(int capacity);
		array<EncodedFrame^>^ GetEncodedFrames(const SFrameBSInfo% info);

		~Encoder();
		!Encoder();
	};
}
