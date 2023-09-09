#include "pch.h"
#include <vcclr.h> // PtrToStringChars
#include <chrono>
#include <iostream>
#using <System.Drawing.dll>
#include "Decoder.h"

using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::Runtime::InteropServices;

namespace H264Sharp {
	

	Decoder::Decoder() 
	{
		const wchar_t* dllName = is64Bit() ? L"openh264-2.3.1-win64.dll" : L"openh264-2.3.1-win32.dll";
		Create(dllName);
	}
	Decoder::Decoder(String^ dllName)
	{
		pin_ptr<const wchar_t> dllname = PtrToStringChars(dllName);
		Create(dllname);
	}
	void Decoder::Create(const wchar_t* dllname)
	{
		// Try to load converter library, if cant find we use the C++/ClI version.
		const wchar_t* converterDll = is64Bit() ? L"Converter64.dll" : L"Converter32.dll";

		HMODULE lib = LoadLibrary(converterDll);
		if (lib != NULL)
		{
			Bgra2Yuv420 = (Yuv2Rgb)GetProcAddress(lib, "Yuv420P2RGB_");
			if (Bgra2Yuv420 == NULL)
				throw gcnew System::DllNotFoundException(String::Format("{0}", GetLastError()));
		}
		else {
			auto ss = is64Bit() ? "Converter64.dll" : "Converter32.dll";
			std::cout << "Unable to load " << ss << ", make sure to include it on your executable path. Falling back to CLI convertors" << std::endl;
			Bgra2Yuv420 = (Yuv2Rgb)Yuv420P2Rgb;
		}

		innerBuffer = 0;
		// Load Open h264 dll. We need to load create and destroy methods.
		//pin_ptr<const wchar_t> dllname = PtrToStringChars(dllName);
		HMODULE hDll = LoadLibrary(dllname);
		if (hDll == NULL) {
			bool _64 = is64Bit();
			auto err = GetLastError();
			throw gcnew System::DllNotFoundException(String::Format("Unable to load '{0}, ErrorCode: {1}'", (_64 ? "openh264-2.3.1-win64.dll" : "openh264-2.3.1-win64.dll"), err));
		}
		dllname = nullptr;

		CreateDecoderFunc = (WelsCreateDecoderFunc)GetProcAddress(hDll, "WelsCreateDecoder");
		if (CreateDecoderFunc == NULL)
		{
			auto err = GetLastError();
			throw gcnew System::DllNotFoundException(String::Format("Unable to load WelsCreateDecoder func, error code: {1}", err));
		}

		DestroyDecoderFunc = (WelsDestroyDecoderFunc)GetProcAddress(hDll, "WelsDestroyDecoder");
		if (DestroyDecoderFunc == NULL) throw gcnew System::DllNotFoundException(String::Format("Unable to load WelsDestroyDecoder func in '{0}'"));

		ISVCDecoder* dec = nullptr;
		int rc = CreateDecoderFunc(&dec);
		decoder = dec;
		if (rc != 0) throw gcnew System::DllNotFoundException(String::Format("Unable to call WelsCreateSVCDecoder func in '{0}'"));

		rc = Initialize();
		if (rc != 0) throw gcnew System::InvalidOperationException("Error occurred during initializing decoder.");


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

	bool Decoder::Decode(array<System::Byte>^ frame, int startIdx, int length, bool noDelay, [Out]DecodingState% rc, [Out]H264Sharp::Yuv420p^% yuv)
	{
		pin_ptr<System::Byte> p = &frame[startIdx];
		unsigned char* pby = p;
		return Decode(pby, length, noDelay, rc, yuv);
	}

	bool Decoder::Decode(array<System::Byte>^ frame, int startIdx, int length, bool noDelay, [Out]DecodingState% rc, [Out]H264Sharp::RgbImage^% rgb)
	{
		pin_ptr<System::Byte> p = &frame[startIdx];
		unsigned char* pby = p;
		return Decode(pby, length, noDelay, rc, rgb);
	}

	bool Decoder::Decode(array<System::Byte>^ frame, int startIdx, int length, bool noDelay, [Out]DecodingState% rc,[Out]Bitmap^% bmp)
	{
		pin_ptr<System::Byte> p = &frame[startIdx];
		unsigned char* pby = p;
		return Decode(pby, length, noDelay, rc, bmp);
	}
	bool Decoder::Decode(IntPtr frame, int length, bool noDelay, [Out]DecodingState% rc, [Out]H264Sharp::Yuv420p^% yuv)
	{
		void* ptr = frame.ToPointer();
		unsigned char* p = reinterpret_cast<unsigned char*>(ptr);
		return Decode(p, length, noDelay, rc, yuv);
	}
	bool Decoder::Decode(IntPtr frame, int length, bool noDelay, [Out]DecodingState% rc, [Out]Bitmap^% bmp)
	{
		void* ptr = frame.ToPointer();
		unsigned char* p = reinterpret_cast<unsigned char*>(ptr);
		return Decode(p, length, noDelay, rc, bmp);
	}
	bool Decoder::Decode(IntPtr frame, int length, bool noDelay, [Out]DecodingState% rc, [Out]H264Sharp::RgbImage^% rgb)
	{
		void* ptr = frame.ToPointer();
		unsigned char* p = reinterpret_cast<unsigned char*>(ptr);
		return Decode(p, length, noDelay, rc, rgb);
	}

	bool Decoder::Decode(unsigned char* frame, int length, bool noDelay, [Out]DecodingState% rc, [Out]H264Sharp::Yuv420p^% yuv)
	{
		DecodingState statusCode;
		bool success;
		YuvNative res = DecodeInternal(frame, length, noDelay, statusCode, success);
		if (success)
		{
			yuv = gcnew Yuv420p(res.Y, res.U, res.V,res.width, res.height, res.stride, res.stride2, res.stride2);
		}
		rc = statusCode;
		return success;
	}
	
	long avg = 0;
	bool Decoder::Decode(unsigned char* frame, int length, bool noDelay, [Out]DecodingState% rc, [Out]H264Sharp::RgbImage^% rgb)
	{
		DecodingState statusCode;
		bool success;
		YuvNative res = DecodeInternal(frame, length, noDelay, statusCode, success);
		if(success)
		{
			byte* rgbBytes = YUV420PtoRGB(res.Y, res.U, res.V, res.width, res.height, res.stride,res.stride2);
			rgb = gcnew RgbImage(rgbBytes, res.width, res.height, res.width*3);
			
		}
		rc = statusCode;
		return success;
	}

	bool Decoder::Decode(unsigned char* frame, int length, bool noDelay, [Out]DecodingState% rc, [Out]Bitmap^% bmp)
	{
		DecodingState statusCode;
		bool success;
		YuvNative res = DecodeInternal(frame, length, noDelay, statusCode, success);
		if (success)
		{
			byte* rgb = YUV420PtoRGB(res.Y, res.U, res.V, res.width, res.height, res.stride,res.stride2);
			bmp = RGBtoBitmap(rgb, res.width, res.height);
		}
		rc = statusCode;
		return success;
	}
	inline bool HasFlag(int value, int flag)
	{
		return (value & (int)flag) == (int)flag;
	}
	YuvNative Decoder::DecodeInternal(unsigned char* frame, int length, bool noDelay, DecodingState& rcc, bool& succes)
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
		if (rc!=0 ) return yuv;
		//if (HasFlag(rc, dsRefLost) ) return yuv;
		if (bufInfo.iBufferStatus != 1) return yuv;

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
		this->!Decoder();
	}

	
	Decoder::!Decoder()
	{
		delete innerBuffer;
		decoder->Uninitialize();
		DestroyDecoderFunc(decoder);
	}

	byte* Decoder::YUV420PtoRGB(byte* yplane, byte* uplane, byte* vplane, int width, int height, int stride,int stride2)
	{

		// Caching the decode buffer.
		if (innerBufLen ==0 || innerBufLen != width * height * 3)
		{
			delete innerBuffer;
			innerBuffer = new byte[width * height * 3];
			innerBufLen = width * height * 3;
		}
		//auto t_start = std::chrono::high_resolution_clock::now();

		Bgra2Yuv420(innerBuffer, yplane, uplane, vplane, width, height, stride, stride2, width * 3, 0);

		/*auto t_end = std::chrono::high_resolution_clock::now();
		double elapsed_time_ms = std::chrono::duration<double, std::micro>(t_end - t_start).count();
		std::cout << elapsed_time_ms << std::endl;*/
		return innerBuffer;

		//byte* rgbBytes = innerBuffer;
		//byte* result =(byte*)innerBuffer;
		//byte* rgb = result;
		////
		//for (int y = 0; y < height; y++)
		//{
		//	int rowIdx = (stride * y);
		//	int uvpIdx = (stride / 2) * (y / 2);

		//	byte* pYp = yplane + rowIdx;
		//	byte* pUp = uplane + uvpIdx;
		//	byte* pVp = vplane + uvpIdx;

		//	for (int x = 0; x < width; x += 2)
		//	{
		//		int C1 = ((pYp[0] - 16) * 298) + 128;
		//		int C2 = ((pYp[1] - 16) * 298) + 128;

		//		const int D = *pUp - 128;
		//		int D1 = (D) * 100;
		//		int D5 = (D) * 516;

		//		const int E = *pVp - 128;
		//		int E4 = (E) * 409;
		//		int E2 = (E) * 208;
		//		int DE = -D1 - E2;
		//		//--

		//		int R1 = (C1 + E4 ) >> 8;
		//		int G1 = (C1 + DE ) >> 8;
		//		int B1 = (C1 + D5 ) >> 8;

		//		int R2 = (C2 + E4 ) >> 8;
		//		int G2 = (C2 + DE ) >> 8;
		//		int B2 = (C2 + D5 ) >> 8;

		//		rgb[0] = (byte)(B1 < 0 ? 0 : B1 > 255 ? 255 : B1);
		//		rgb[1] = (byte)(G1 < 0 ? 0 : G1 > 255 ? 255 : G1);
		//		rgb[2] = (byte)(R1 < 0 ? 0 : R1 > 255 ? 255 : R1);

		//		rgb[3] = (byte)(B2 < 0 ? 0 : B2 > 255 ? 255 : B2);
		//		rgb[4] = (byte)(G2 < 0 ? 0 : G2 > 255 ? 255 : G2);
		//		rgb[5] = (byte)(R2 < 0 ? 0 : R2 > 255 ? 255 : R2);

		//		rgb += 6;
		//		pYp += 2;
		//		pUp += 1;
		//		pVp += 1;
		//	}
		//	
		//}
		//
	
		//return result;
	}

	Bitmap^ Decoder::RGBtoBitmap(byte* rgb, int width, int height)
	{
		Bitmap^ bmp = gcnew Bitmap(width, height, width * 3 ,  System::Drawing::Imaging::PixelFormat::Format24bppRgb, System::IntPtr(rgb));
		return bmp;
		
	}
}
