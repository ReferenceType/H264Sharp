#ifndef DECODER
#define DECODER
#include "pch.h"
#include <chrono>
#include "Decoder.h"
#include <stdexcept>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "ImageTypes.h"
#include "Converter.h"



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
		Decoder();
		~Decoder();
		int LoadCisco(const char* dllName);
		int Initialize();
		int Initialize(SDecodingParam param);

		int SetOption(DECODER_OPTION option, void* value);
		int GetOption(DECODER_OPTION option, void* value);

		int DecodeParser(const unsigned char* pSrc,const int iSrcLen, SParserBsInfo* pDstInfo) ;

		//internal buffer
		int Decode(unsigned char* frame, int length,bool noDelay, DecodingState &rc, H264Sharp::YuvNative &yuv);

		//external source
		int DecodeExt(unsigned char* frame, int length, bool noDelay, DecodingState& rc, H264Sharp::YuvNative& to);


		static int EnableDebugLogs;

		
	private:

		unsigned char* innerBuffer = nullptr;
		int innerBufLen=0;
		ISVCDecoder* decoder= nullptr;

		typedef int(*WelsCreateDecoderFunc)(ISVCDecoder** ppDecoder);
		WelsCreateDecoderFunc CreateDecoderFunc= nullptr;
		typedef void(*WelsDestroyDecoderFunc)(ISVCDecoder* ppDecoder);
		WelsDestroyDecoderFunc DestroyDecoderFunc= nullptr;

#ifdef _WIN32
		HMODULE libraryHandle = nullptr;
#else
		void* libraryHandle = nullptr;
#endif

		YuvNative DecodeInternal(unsigned char* frame, int length, bool noDelay, DecodingState& rc, int& success);
		uint8_t* YUV420PtoRGB(YuvNative& yuv);

		void EnsureCapacity(int capacity);



	};
}
#endif
