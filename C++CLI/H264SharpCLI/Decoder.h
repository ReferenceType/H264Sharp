#pragma once
#include "pch.h"
#using <System.Drawing.dll>

using namespace System;

namespace H264Sharp 
{
	public enum class DecodingState {
		/**
		* Errors derived from bitstream parsing
		*/
		dsErrorFree = 0x00,   ///< bit stream error-free
		dsFramePending = 0x01,   ///< need more throughput to generate a frame output,
		dsRefLost = 0x02,   ///< layer lost at reference frame with temporal id 0
		dsBitstreamError = 0x04,   ///< error bitstreams(maybe broken internal frame) the decoder cared
		dsDepLayerLost = 0x08,   ///< dependented layer is ever lost
		dsNoParamSets = 0x10,   ///< no parameter set NALs involved
		dsDataErrorConcealed = 0x20,   ///< current data error concealed specified
		dsRefListNullPtrs = 0x40, ///<ref picure list contains null ptrs within uiRefCount range

		/**
		* Errors derived from logic level
		*/
		dsInvalidArgument = 0x1000, ///< invalid argument specified
		dsInitialOptExpected = 0x2000, ///< initializing operation is expected
		dsOutOfMemory = 0x4000, ///< out of memory due to new request

		dsDstBufNeedExpan = 0x8000  ///< actual picture size exceeds size of dst pBuffer feed in decoder, so need expand its size

	};

	typedef void(*Yuv2Rgb)(unsigned char* dst_ptr,
		const unsigned char* y_ptr,
		const unsigned char* u_ptr,
		const unsigned char* v_ptr,
		signed   int   width,
		signed   int   height,
		signed   int   y_span,
		signed   int   uv_span,
		signed   int   dst_span,
		signed   int   dither);

	static Yuv2Rgb Bgra2Yuv420;

	public ref class Decoder{
	public:
		Decoder(String ^dllName);
		Decoder();

		bool Decode(unsigned char *frame, int length,bool noDelay, DecodingState% rc, H264Sharp::Yuv420p^% yuv);
		bool Decode(unsigned char *frame, int length, bool noDelay, DecodingState% rc, H264Sharp::RgbImage^% rgb);
		bool Decode(unsigned char* frame, int length, bool noDelay, DecodingState% rc, System::Drawing::Bitmap^% bitmap);

		bool Decode(IntPtr frame, int length, bool noDelay, DecodingState% rc, H264Sharp::Yuv420p^% yuv);
		bool Decode(IntPtr frame, int length, bool noDelay, DecodingState% rc, H264Sharp::RgbImage^% rgb);
		bool Decode(IntPtr frame, int length, bool noDelay, DecodingState% rc, System::Drawing::Bitmap^% bitmap);

		bool Decode(array<System::Byte>^ frame, int startIdx, int length, bool noDelay, DecodingState% rc, H264Sharp::Yuv420p^% yuv);
		bool Decode(array<System::Byte>^ frame, int startIdx, int length, bool noDelay, DecodingState% rc, H264Sharp::RgbImage^% rgb);
		bool Decode(array<System::Byte>^frame, int startIdx, int length, bool noDelay, DecodingState% rc, System::Drawing::Bitmap^% bitmap);

	private:
		unsigned char* innerBuffer = nullptr;
		int innerBufLen = 0;
		ISVCDecoder* decoder = nullptr;

		typedef int(__cdecl* WelsCreateDecoderFunc)(ISVCDecoder** ppDecoder);
		WelsCreateDecoderFunc CreateDecoderFunc;
		typedef void(__cdecl* WelsDestroyDecoderFunc)(ISVCDecoder* ppDecoder);
		WelsDestroyDecoderFunc DestroyDecoderFunc;

		void Create(const wchar_t* dllName);
		int Initialize();

		YuvNative Decoder::DecodeInternal(unsigned char* frame, int length, bool noDelay, DecodingState& rc, bool& success);
		byte* YUV420PtoRGB(byte* yplane, byte* uplane, byte* vplane, int width, int height, int stride, int stride2);
		static System::Drawing::Bitmap^ RGBtoBitmap(byte* rgb, int width, int height);

		~Decoder();
		!Decoder();

	};
}
