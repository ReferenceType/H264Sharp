#include "Decoder.h"

namespace H264Sharp {

	int Decoder::EnableDebugLogs = 0;

	Decoder::Decoder()
	{
		
	}
	

	enum DecoderErrorCode {
		SUCCESS = 0,
		LIBRARY_LOAD_FAILED = 1,
		CREATE_FUNC_LOAD_FAILED = 2,
		DESTROY_FUNC_LOAD_FAILED = 3,
		DECODER_CREATION_FAILED = 4
	};

	int Decoder::LoadCisco(const char* dllName)
	{
		if (Decoder::EnableDebugLogs > 0)
			logger << "Decoder [" << dllName << "] loading..\n";

#ifdef _WIN32
		// Convert UTF-8 to wide characters for Windows
		int wcharCount = MultiByteToWideChar(CP_UTF8, 0, dllName, -1, nullptr, 0);
		wchar_t* wideDllName = new wchar_t[wcharCount];
		MultiByteToWideChar(CP_UTF8, 0, dllName, -1, wideDllName, wcharCount);

		HMODULE hDll = LoadLibraryW(wideDllName);
		delete[] wideDllName;

		if (hDll == NULL) {
			if (Decoder::EnableDebugLogs > 0)
				logger << "Failed to load library\n";
			return LIBRARY_LOAD_FAILED;
		}

		// Load decoder functions
		CreateDecoderFunc = reinterpret_cast<WelsCreateDecoderFunc>(GetProcAddress(hDll, "WelsCreateDecoder"));
		if (CreateDecoderFunc == NULL) {
			if (Decoder::EnableDebugLogs > 0)
				logger << "Failed to load [WelsCreateDecoder] method\n";
			FreeLibrary(hDll);
			return CREATE_FUNC_LOAD_FAILED;
		}

		DestroyDecoderFunc = reinterpret_cast<WelsDestroyDecoderFunc>(GetProcAddress(hDll, "WelsDestroyDecoder"));
		if (DestroyDecoderFunc == NULL) {
			if (Decoder::EnableDebugLogs > 0)
				logger << "Failed to load [WelsDestroyDecoder] method\n";
			FreeLibrary(hDll);
			return DESTROY_FUNC_LOAD_FAILED;
		}
#else
		void* hDll = dlopen(dllName, RTLD_LAZY);
		if (hDll == NULL) {
			if (Decoder::EnableDebugLogs > 0)
				logger << "Failed to load library: " << dlerror() << "\n";
			return LIBRARY_LOAD_FAILED;
		}

		CreateDecoderFunc = reinterpret_cast<WelsCreateDecoderFunc>(dlsym(hDll, "WelsCreateDecoder"));
		if (CreateDecoderFunc == NULL) {
			if (Decoder::EnableDebugLogs > 0)
				logger << "Failed to load [WelsCreateDecoder] method: " << dlerror() << "\n";
			dlclose(hDll);
			return CREATE_FUNC_LOAD_FAILED;
		}

		DestroyDecoderFunc = reinterpret_cast<WelsDestroyDecoderFunc>(dlsym(hDll, "WelsDestroyDecoder"));
		if (DestroyDecoderFunc == NULL) {
			if (Decoder::EnableDebugLogs > 0)
				logger << "Failed to load [WelsDestroyDecoder] method: " << dlerror() << "\n";
			dlclose(hDll);
			return DESTROY_FUNC_LOAD_FAILED;
		}
#endif

		ISVCDecoder* dec = nullptr;
		int rc = CreateDecoderFunc(&dec);
		if (rc != 0) {
			if (Decoder::EnableDebugLogs > 0)
				logger << "Failed to create decoder with return code: " << rc << "\n";
#ifdef _WIN32
			FreeLibrary(hDll);
#else
			dlclose(hDll);
#endif
			return DECODER_CREATION_FAILED;
		}

		decoder = dec;
		libraryHandle = hDll;  

		if (Decoder::EnableDebugLogs > 0)
			logger << dllName << " loaded\n";

		return SUCCESS;
	}

//	void Decoder::Create(const char* dllName)
//	{
//		if(Decoder::EnableDebugLogs >0)
//			logger <<"Decoder [" << dllName << "] loading..\n";
//		
//		// Load dynamic library
//#ifdef _WIN32
//		int wcharCount = MultiByteToWideChar(CP_UTF8, 0, dllName, -1, nullptr, 0);
//		wchar_t* wideDllName = new wchar_t[wcharCount];
//		MultiByteToWideChar(CP_UTF8, 0, dllName, -1, wideDllName, wcharCount);
//
//		HMODULE hDll = DLL_LOAD_FUNCTION(wideDllName);
//		delete[] wideDllName;
//#else
//		void* hDll = DLL_LOAD_FUNCTION(dllName, RTLD_LAZY);
//#endif
//		if (hDll == NULL) {
//#ifdef _WIN32
//			throw std::runtime_error("Failed to load library");
//#else
//			throw std::runtime_error(DLL_ERROR_CODE);
//#endif
//		}
//
//		// Load Function
//		CreateDecoderFunc = reinterpret_cast<WelsCreateDecoderFunc>(DLL_GET_FUNCTION(hDll, "WelsCreateDecoder"));
//		if (CreateDecoderFunc == NULL) {
//#ifdef _WIN32
//			throw std::runtime_error("Failed to load [WelsCreateDecoder] method");
//#else
//			throw std::runtime_error(DLL_ERROR_CODE);
//#endif
//		}
//
//		DestroyDecoderFunc = reinterpret_cast<WelsDestroyDecoderFunc>(DLL_GET_FUNCTION(hDll, "WelsDestroyDecoder"));
//		if (DestroyDecoderFunc == NULL) {
//#ifdef _WIN32
//			throw std::runtime_error("Failed to load [WelsDestroyDecoder] method");
//#else
//			throw std::runtime_error(DLL_ERROR_CODE);
//#endif
//		}
//
//		ISVCDecoder* dec = nullptr;
//		int rc = CreateDecoderFunc(&dec);
//		decoder = dec;
//		if (rc != 0) {
//			throw std::runtime_error("Failed to create decoder");
//		}
//		if (Decoder::EnableDebugLogs > 0)
//			logger << dllName << " loaded\n";
//
//	}

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

	int Decoder::Decode(unsigned char* frame, int length, bool noDelay, DecodingState& rc, H264Sharp::YuvNative& res)
	{
		DecodingState statusCode;
		int success = 1;
		res = DecodeInternal(frame, length, noDelay, statusCode, success);
		
		rc = statusCode;
		return success;
	}

	inline void Compact(const YuvNative& src, YuvNative& dst);

	int Decoder::DecodeExt(unsigned char* frame, int length, bool noDelay, DecodingState& rc, H264Sharp::YuvNative& to)
	{
		// wont let me give it
		//SBufferInfo bufInfo;
		//memset(&bufInfo, 0x00, sizeof(bufInfo));

		//uint8_t* buffer[3];
		//buffer[0] = to.Y;
		//buffer[1] = to.U;
		//buffer[2] = to.V;

		//bufInfo.UsrData.sSystemBuffer.iStride[0] = to.width;   
		//bufInfo.UsrData.sSystemBuffer.iStride[1] = to.width / 2; 
		//bufInfo.UsrData.sSystemBuffer.iStride[2] = to.width / 2; 
		//bufInfo.pDst[0] = to.Y;
		//bufInfo.pDst[1] = to.U;
		//bufInfo.pDst[2] = to.V;


		//int rci = -1;

		//if (noDelay)
		//	rci = decoder->DecodeFrameNoDelay(frame, length, buffer, &bufInfo);
		//else
		//	rci = decoder->DecodeFrame2(frame, length, buffer, &bufInfo);

		//rc = (DecodingState)rci;

		//if (bufInfo.iBufferStatus < 1)
		//	return true;// clang skips this when cond is !=1
		//return false;

		
		DecodingState statusCode;
		int success = 1;
		YuvNative src = DecodeInternal(frame, length, noDelay, statusCode, success);

		if(success==0)
		{
			if (src.width > to.width || src.height > to.height) 
			{
				success = 1;
				statusCode = DecodingState::dsDstBufNeedExpan;
			}
			else 
			{
				Compact(src, to);
			}
		}
			
		rc = statusCode;
		return success;
	}

	inline void Compact(const YuvNative& src, YuvNative& dst) {
		
		int uvHeight = src.height / 2;
		int uvWidth = src.width / 2;

		for (int i = 0; i < src.height; ++i) {
			std::memcpy(dst.Y + (i * src.width), src.Y + (i * src.yStride), src.width);
		}

		for (int i = 0; i < uvHeight; ++i) {
			std::memcpy(dst.U + i * uvWidth, src.U + i * src.uvStride, uvWidth);
		}

		for (int i = 0; i < uvHeight; ++i) {
			std::memcpy(dst.V + i * uvWidth, src.V + i * src.uvStride, uvWidth);
		}

		dst.yStride = dst.width;
		dst.uvStride = uvWidth;
	}

	/*[[clang::optnone]]*/YuvNative Decoder::DecodeInternal(unsigned char* frame, int length, bool noDelay, DecodingState& ds, int& succes)
	{
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
		yuv.uvStride = bufInfo.UsrData.sSystemBuffer.iStride[1];
		yuv.Y = bufInfo.pDst[0];
		yuv.U = bufInfo.pDst[1];
		yuv.V = bufInfo.pDst[2];
		succes = 0;
		return yuv;
	}

	

	uint8_t* Decoder::YUV420PtoRGB(YuvNative& yuv)
	{
		// Caching the decode buffer.
		if (innerBufLen == 0 || innerBufLen != yuv.width * yuv.height * 3)
		{
			delete[] innerBuffer;
			innerBuffer = new uint8_t[yuv.width * yuv.height * 3];
			innerBufLen = yuv.width * yuv.height * 3;
		}
		Converter::Yuv420PtoRGB<3,true>(innerBuffer, yuv.Y, yuv.U, yuv.V, yuv.width, yuv.height, yuv.yStride, yuv.uvStride, yuv.width * 3);

		return innerBuffer;


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

		if(decoder !=nullptr)
		{
			decoder->Uninitialize();
			DestroyDecoderFunc(decoder);
		}
		

		if (libraryHandle != nullptr) {
#ifdef _WIN32
			FreeLibrary(libraryHandle);
#else
			dlclose(libraryHandle);
#endif
		}
	}

}
