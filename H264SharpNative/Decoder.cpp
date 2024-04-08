#include "Decoder.h"

namespace H264Sharp {


	Decoder::Decoder()
	{
		const char* dllName = is64Bit() ? "openh264-2.3.1-win64.dll" : "openh264-2.3.1-win32.dll";
		Create(dllName);
	}
	Decoder::Decoder(const char* dllname)
	{
		Create(dllname);
	}
	void Decoder::Create(const char* dllName)
	{
		// Convert wchar_t* to char* for Linux compatibility
		
		// Load dynamic library
#ifdef _WIN32
		int wcharCount = MultiByteToWideChar(CP_UTF8, 0, dllName, -1, nullptr, 0);
		wchar_t* wideDllName = new wchar_t[wcharCount];
		MultiByteToWideChar(CP_UTF8, 0, dllName, -1, wideDllName, wcharCount);

		HMODULE hDll = DLL_LOAD_FUNCTION(wideDllName);
		delete[] wideDllName;
#else
		void* hDll = DLL_LOAD_FUNCTION(dllName, RTLD_LAZY);
#endif
		if (hDll == NULL) {
#ifdef _WIN32
			throw std::runtime_error("Failed to load library");
#else
			throw std::runtime_error(DLL_ERROR_CODE);
#endif
		}

		// Load Function
		CreateDecoderFunc = reinterpret_cast<WelsCreateDecoderFunc>(DLL_GET_FUNCTION(hDll, "WelsCreateDecoder"));
		if (CreateDecoderFunc == NULL) {
#ifdef _WIN32
			throw std::runtime_error("Failed to load [WelsCreateDecoder] method");
#else
			throw std::runtime_error(DLL_ERROR_CODE);
#endif
		}

		DestroyDecoderFunc = reinterpret_cast<WelsDestroyDecoderFunc>(DLL_GET_FUNCTION(hDll, "WelsDestroyDecoder"));
		if (DestroyDecoderFunc == NULL) {
#ifdef _WIN32
			throw std::runtime_error("Failed to load [WelsDestroyDecoder] method");
#else
			throw std::runtime_error(DLL_ERROR_CODE);
#endif
		}

		ISVCDecoder* dec = nullptr;
		int rc = CreateDecoderFunc(&dec);
		decoder = dec;
		if (rc != 0) {
			throw std::runtime_error("Failed to create decoder");
		}


	}

	int Decoder::Initialize()
	{
		SDecodingParam decParam;
		memset(&decParam, 0, sizeof(SDecodingParam));
		decParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
		decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_SVC;
		return decoder->Initialize(&decParam);
	}

	int Decoder::Initialize(SDecodingParam decParam)
	{
		return decoder->Initialize(&decParam);
	}



	bool Decoder::Decode(unsigned char* frame, int length, bool noDelay, DecodingState& rc, H264Sharp::Yuv420p& yuv)
	{
		DecodingState statusCode;
		bool success;
		YuvNative res = DecodeInternal(frame, length, noDelay, statusCode, success);
		if (success)
		{
			yuv = Yuv420p(res.Y, res.U, res.V, res.width, res.height, res.stride, res.stride2, res.stride2);
		}
		rc = statusCode;
		return success;
	}

	bool Decoder::Decode(unsigned char* frame, int length, bool noDelay, DecodingState& rc, H264Sharp::RgbImage& rgb)
	{
		DecodingState statusCode;
		bool success;
		YuvNative res = DecodeInternal(frame, length, noDelay, statusCode, success);
		if (success)
		{
			byte* rgbBytes = YUV420PtoRGB(res.Y, res.U, res.V, res.width, res.height, res.stride, res.stride2);
			rgb = RgbImage(rgbBytes, res.width, res.height, res.width * 3);

		}
		rc = statusCode;
		return success;
	}

	bool Decoder::DecodeExt(unsigned char* frame, int length, bool noDelay, DecodingState& rc, unsigned char* destRgb)
	{
		DecodingState statusCode;
		bool success;
		YuvNative res = DecodeInternal(frame, length, noDelay, statusCode, success);
		if (success)
		{
			YUV420PtoRGBExt(res.Y, res.U, res.V, res.width, res.height, res.stride, res.stride2, destRgb);
		}
		rc = statusCode;
		return success;
	}


	bool HasFlag(int value, int flag)
	{
		return (value & (int)flag) == (int)flag;
	}

	// Fucking clang skips if statements inside wnen optimised...
	[[clang::optnone]] YuvNative Decoder::DecodeInternal(unsigned char* frame, int length, bool noDelay, DecodingState& rcc, bool& succes)
	{
		succes = false;
		YuvNative yuv;

		unsigned char* buffer[3];

		SBufferInfo bufInfo; memset(&bufInfo, 0x00, sizeof(bufInfo));
		int rc = -1;

		if (noDelay)
			rc = decoder->DecodeFrameNoDelay(frame, length, buffer, &bufInfo);
		else
			rc = decoder->DecodeFrame2(frame, length, buffer, &bufInfo);

		rcc = (DecodingState)rc;
		//if (rc!=0 ) return yuv;

		//if (HasFlag(rc, dsNoParamSets)) {
		//	//std::cout << "No Param";
		//	return yuv;
		//}

		if (bufInfo.iBufferStatus != 1) return yuv;// clang skips this

		memset(&yuv, 0x00, sizeof(yuv));

		yuv.width = bufInfo.UsrData.sSystemBuffer.iWidth;
		yuv.height = bufInfo.UsrData.sSystemBuffer.iHeight;
		yuv.stride = bufInfo.UsrData.sSystemBuffer.iStride[0];
		yuv.Y = bufInfo.pDst[0];
		yuv.U = bufInfo.pDst[1];
		yuv.V = bufInfo.pDst[2];
		yuv.stride2 = bufInfo.UsrData.sSystemBuffer.iStride[1];
		succes = true;
		return yuv;
	}

	Decoder::~Decoder()
	{
		delete[] innerBuffer;
		decoder->Uninitialize();
		DestroyDecoderFunc(decoder);
	}

	int Decoder::SetOption(DECODER_OPTION option, void* value)
	{
		return decoder->SetOption(option, value);
	}

	int Decoder::GetOption(DECODER_OPTION option, void* value)
	{
		return decoder->GetOption(option, value);

	}

	int Decoder::DecodeParser(const unsigned char* pSrc, const int iSrcLen, SParserBsInfo* pDstInfo)
	{
		return decoder->DecodeParser(pSrc, iSrcLen, pDstInfo);
	}

	byte* Decoder::YUV420PtoRGB(byte* yplane, byte* uplane, byte* vplane, int width, int height, int stride, int stride2)
	{

		// Caching the decode buffer.
		if (innerBufLen == 0 || innerBufLen != width * height * 3)
		{
			delete[] innerBuffer;
			innerBuffer = new byte[width * height * 3];
			innerBufLen = width * height * 3;
		}
		//auto t_start = std::chrono::high_resolution_clock::now();

		Yuv420P2RGB(innerBuffer, yplane, uplane, vplane, width, height, stride, stride2, width * 3, useSSEConverter, threadCount);

		/*	auto t_end = std::chrono::high_resolution_clock::now();
			double elapsed_time_ms = std::chrono::duration<double, std::micro>(t_end - t_start).count();
			std::cout<<"decoded " << elapsed_time_ms << std::endl;*/
		return innerBuffer;


	}
	byte* Decoder::YUV420PtoRGBExt(byte* yplane, byte* uplane, byte* vplane, int width, int height, int stride, int stride2, unsigned char* destBuff)
	{


		//auto t_start = std::chrono::high_resolution_clock::now();

		Yuv420P2RGB(destBuff, yplane, uplane, vplane, width, height, stride, stride2, width * 3, useSSEConverter, threadCount);

		/*	auto t_end = std::chrono::high_resolution_clock::now();
			double elapsed_time_ms = std::chrono::duration<double, std::micro>(t_end - t_start).count();
			std::cout<<"decoded " << elapsed_time_ms << std::endl;*/


		return destBuff;
	}


}
