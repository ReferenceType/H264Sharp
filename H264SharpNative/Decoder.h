#pragma once
#include "pch.h"
#include <string>


namespace H264Sharp 
{
	 enum class DecodingState {
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

	

    class Decoder{
	public:
		Decoder(std::string dllName);
		Decoder(const wchar_t* dllname);
		Decoder();
		~Decoder();

		bool Decode(unsigned char *frame, int length,bool noDelay, DecodingState &rc, H264Sharp::Yuv420p &yuv);
		bool Decode(unsigned char *frame, int length, bool noDelay, DecodingState &rc, H264Sharp::RgbImage &rgb);

		
	private:
		unsigned char* innerBuffer = nullptr;
		int innerBufLen=0;
		ISVCDecoder* decoder= nullptr;

		typedef int(__cdecl* WelsCreateDecoderFunc)(ISVCDecoder** ppDecoder);
		WelsCreateDecoderFunc CreateDecoderFunc= nullptr;
		typedef void(__cdecl* WelsDestroyDecoderFunc)(ISVCDecoder* ppDecoder);
		WelsDestroyDecoderFunc DestroyDecoderFunc= nullptr;

		void Create(const wchar_t* dllName);
		int Initialize();

		YuvNative DecodeInternal(unsigned char* frame, int length, bool noDelay, DecodingState& rc, bool& success);
		byte* YUV420PtoRGB(byte* yplane, byte* uplane, byte* vplane, int width, int height, int stride, int stride2);


	};
}
