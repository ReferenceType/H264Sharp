#ifndef DECODER
#define DECODER
#include "pch.h"
#include <chrono>
#include "Decoder.h"
#include <stdexcept>
#include <string.h>
#include <stdlib.h>
#include "ImageTypes.h"
#include "ConverterLocal.h"



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
		int Initialize();
		int Initialize(SDecodingParam param);

		int SetOption(DECODER_OPTION option, void* value);
		int GetOption(DECODER_OPTION option, void* value);

		int DecodeParser(const unsigned char* pSrc,const int iSrcLen, SParserBsInfo* pDstInfo) ;

		bool Decode(unsigned char *frame, int length,bool noDelay, DecodingState &rc, H264Sharp::Yuv420p &yuv);
		bool Decode(unsigned char *frame, int length, bool noDelay, DecodingState &rc, H264Sharp::RgbImage &rgb);
		bool DecodeExt(unsigned char *frame, int length, bool noDelay, DecodingState &rc, unsigned char* destRgb);
		void UseSSEConverter(bool isSSE)
		{
			useSSEConverter = isSSE;
		};
		int threadCount = 4;

		
	private:
		unsigned char* innerBuffer = nullptr;
		int innerBufLen=0;
		ISVCDecoder* decoder= nullptr;
		bool useSSEConverter = true;

		typedef int(*WelsCreateDecoderFunc)(ISVCDecoder** ppDecoder);
		WelsCreateDecoderFunc CreateDecoderFunc= nullptr;
		typedef void(*WelsDestroyDecoderFunc)(ISVCDecoder* ppDecoder);
		WelsDestroyDecoderFunc DestroyDecoderFunc= nullptr;

		void Create(const wchar_t* dllName);

		YuvNative DecodeInternal(unsigned char* frame, int length, bool noDelay, DecodingState& rc, bool& success);
		byte* YUV420PtoRGB(byte* yplane, byte* uplane, byte* vplane, int width, int height, int stride, int stride2);
		byte* YUV420PtoRGBExt(byte* yplane, byte* uplane, byte* vplane, int width, int height, int stride, int stride2, unsigned char* destBuff);


	};
}
#endif
