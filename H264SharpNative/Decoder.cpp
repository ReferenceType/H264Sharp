#include "Decoder.h"

namespace H264Sharp {

	int Decoder::EnableDebugLogs = 0;

	Decoder::Decoder()
	{
		const char* dllName = is64Bit() ? "openh264-2.4.1-win64.dll" : "openh264-2.4.1-win32.dll";
		Create(dllName);
	}
	Decoder::Decoder(const char* dllname)
	{
		Create(dllname);
	}
	void Decoder::Create(const char* dllName)
	{
		if(Decoder::EnableDebugLogs >0)
			std::cout <<"Decoder [" << dllName << "] loading..\n";
		
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
		if (Decoder::EnableDebugLogs > 0)
			std::cout << dllName << " loaded\n";

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



	bool Decoder::Decode(unsigned char* frame, int length, bool noDelay, DecodingState& rc, H264Sharp::YuvNative& res)
	{
		DecodingState statusCode;
		bool success;
		res = DecodeInternal(frame, length, noDelay, statusCode, success);
		
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
			uint8_t* rgbBytes = YUV420PtoRGB(res);
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
			YUV420PtoRGBExt(res, destRgb);
		}
		rc = statusCode;
		return success;
	}


	bool HasFlag(int value, int flag)
	{
		return (value & (int)flag) == (int)flag;
	}

	/*[[clang::optnone]]*/ YuvNative Decoder::DecodeInternal(unsigned char* frame, int length, bool noDelay, DecodingState& ds, bool& succes)
	{
		succes = false;
		YuvNative yuv;

		unsigned char* buffer[3]{0,0,0};

		SBufferInfo bufInfo;
		memset(&bufInfo, 0x00, sizeof(bufInfo));

		int rc = -1;

		if (noDelay)
			rc = decoder->DecodeFrameNoDelay(frame, length, buffer, &bufInfo);
		else
			rc = decoder->DecodeFrame2(frame, length, buffer, &bufInfo);

		ds = (DecodingState)rc;

		if (bufInfo.iBufferStatus < 1) 
			return yuv;// clang skips this when cond is !=1


		yuv.width = bufInfo.UsrData.sSystemBuffer.iWidth;
		yuv.height = bufInfo.UsrData.sSystemBuffer.iHeight;
		yuv.yStride = bufInfo.UsrData.sSystemBuffer.iStride[0];
		yuv.Y = bufInfo.pDst[0];
		yuv.U = bufInfo.pDst[1];
		yuv.V = bufInfo.pDst[2];
		yuv.uvStride = bufInfo.UsrData.sSystemBuffer.iStride[1];
		yuv.format = YUVType::YV12;
		succes = true;
		return yuv;
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

	
	uint8_t* Decoder::YUV420PtoRGB(YuvNative& yuv)
	{
		EnsureCapacity((yuv.width * yuv.height) + (yuv.width * yuv.height) / 2);
		
		//Converter::Yuv420PtoRGB(yuv,innerBuffer);
		Converter::Yuv420PtoRGB<3, true>(innerBuffer, yuv.Y, yuv.U, yuv.V, yuv.width, yuv.height, yuv.yStride, yuv.uvStride, yuv.width * 3);

		return innerBuffer;
	}

	uint8_t* Decoder::YUV420PtoRGBExt(YuvNative& yuv, unsigned char* destBuff)
	{
		Converter::Yuv420PtoRGB<3, true>(destBuff, yuv.Y, yuv.U, yuv.V, yuv.width, yuv.height, yuv.yStride, yuv.uvStride, yuv.width * 3);

		//Converter::Yuv420PtoRGB(yuv, destBuff);
		return destBuff;
	}

	inline void Decoder::EnsureCapacity(int capacity)
	{

		if (innerBufLen < capacity)
		{
			if (innerBuffer != nullptr)
			{
				FreeAllignAlloc(innerBuffer);
			}
			innerBuffer = (unsigned char*)AllignAlloc(capacity);
			innerBufLen = capacity;
		}

	}
	Decoder::~Decoder()
	{
		FreeAllignAlloc(innerBuffer);
		decoder->Uninitialize();
		DestroyDecoderFunc(decoder);
	}

}
