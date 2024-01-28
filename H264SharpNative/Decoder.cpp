#include "pch.h"
#include <chrono>
#include <iostream>
#include "Decoder.h"

namespace H264Sharp {


	Decoder::Decoder()
	{
		const wchar_t* dllName = is64Bit() ? L"openh264-2.3.1-win64.dll" : L"openh264-2.3.1-win32.dll";
		Create(dllName);
	}

	Decoder::Decoder(std::string dllName)
	{
		auto s = std::wstring(dllName.begin(), dllName.end());
		const wchar_t* dllname = s.c_str();
		Create(dllname);
	}

	Decoder::Decoder(const wchar_t* dllname)
	{
		Create(dllname);
	}

	void Decoder::Create(const wchar_t* dllname)
	{
		// Load Open h264 dll. We need to load create and destroy methods.
		//pin_ptr<const wchar_t> dllname = PtrToStringChars(dllName);
		HMODULE hDll = LoadLibrary(dllname);
		if (hDll == NULL) {
			throw new std::exception("Failed to load Dll ", GetLastError());
		}
		

		CreateDecoderFunc = (WelsCreateDecoderFunc)GetProcAddress(hDll, "WelsCreateDecoder");
		if (CreateDecoderFunc == NULL)
		{
			throw new std::exception("Failed to load Dll ", GetLastError());
		}

		DestroyDecoderFunc = (WelsDestroyDecoderFunc)GetProcAddress(hDll, "WelsDestroyDecoder");
		if (DestroyDecoderFunc == NULL) 
			throw new std::exception("Failed to load Dll ", GetLastError());


		ISVCDecoder* dec = nullptr;
		int rc = CreateDecoderFunc(&dec);
		decoder = dec;
		if (rc != 0) 
			throw new std::exception("Failed to load Dll ", GetLastError());


		rc = Initialize();
		if (rc != 0) 
			throw new std::exception("Unable to initialize ", GetLastError());
		
		std::wcout << dllname << " loaded\n";
		dllname = nullptr;
	}

	int Decoder::Initialize()
	{
		SDecodingParam decParam;
		memset(&decParam, 0, sizeof(SDecodingParam));
		decParam.uiTargetDqLayer = UCHAR_MAX;
		decParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
		decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_SVC;
		int rc = decoder->Initialize(&decParam);
		if (rc != 0) return -1;

		return 0;
	}


	bool Decoder::Decode(unsigned char* frame, int length, bool noDelay, DecodingState &rc, H264Sharp::Yuv420p &yuv)
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

	bool Decoder::Decode(unsigned char* frame, int length, bool noDelay, DecodingState &rc, H264Sharp::RgbImage &rgb)
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
		delete [] innerBuffer;
		decoder->Uninitialize();
		DestroyDecoderFunc(decoder);
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

		Yuv420P2RGB(innerBuffer, yplane, uplane, vplane, width, height, stride, stride2, width * 3, 0);

		/*auto t_end = std::chrono::high_resolution_clock::now();
		double elapsed_time_ms = std::chrono::duration<double, std::micro>(t_end - t_start).count();
		std::cout << elapsed_time_ms << std::endl;*/
		return innerBuffer;

		
	}

	
}
