#include "pch.h"
#include <vcclr.h>
#using <System.Drawing.dll>
#include <chrono>
#include <iostream>
#include "Encoder.h"

using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::Runtime::InteropServices;

namespace H264Sharp {

#pragma region Setup

	Encoder::Encoder()
	{
		const wchar_t* dllName = is64Bit() ? L"openh264-2.3.1-win64.dll" : L"openh264-2.3.1-win32.dll";
		Create(dllName);
	}
	Encoder::Encoder(String^ dllName)
	{
		pin_ptr<const wchar_t> dllname = PtrToStringChars(dllName);
		Create(dllname);
	}
	void Encoder::Create(const wchar_t* dllname)
	{
		const wchar_t* converterDll = is64Bit() ? L"Converter64.dll" : L"Converter32.dll";
		const DWORD Avx2 = 40;
		HMODULE lib = NULL;
		if (!IsProcessorFeaturePresent(Avx2)) 
		{
			std::cout << "Unable to use ConverterDLL because AVX2 instructions are not supported! Falling back to CLI convertors" << std::endl;
		}
		else 
		{
			lib = LoadLibrary(converterDll);
		}
		if (lib != NULL)
		{
			Bgra2Yuv420 = (Covert2Yuv420)GetProcAddress(lib, "BGRAtoYUV420Planar_");
			if (Bgra2Yuv420 == NULL)
				throw gcnew System::DllNotFoundException(String::Format("{0}", GetLastError()));

			Bgr2Yuv420 = (Covert2Yuv420)GetProcAddress(lib, "BGRtoYUV420Planar_");
			if (Bgr2Yuv420 == NULL)
				throw gcnew System::DllNotFoundException(String::Format("{0}", GetLastError()));

			Rgb2Yuv420 = (Covert2Yuv420)GetProcAddress(lib, "RGBtoYUV420Planar_");
			if (Rgb2Yuv420 == NULL)
				throw gcnew System::DllNotFoundException(String::Format("{0}", GetLastError()));

			Rgba2Yuv420 = (Covert2Yuv420)GetProcAddress(lib, "RGBAtoYUV420Planar_");
			if (Rgba2Yuv420 == NULL)
				throw gcnew System::DllNotFoundException(String::Format("{0}", GetLastError()));
		}
		else {
			auto ss = is64Bit() ? "Converter64.dll" : "Converter32.dll";
			std::cout << "Encoder: Unable to load " << ss << ", make sure to include it on your executable path. Falling back to CLI convertors" << std::endl;
			Bgra2Yuv420 = BGRAtoYUV420Planar;
			Bgr2Yuv420 = BGRtoYUV420Planar;
			Rgba2Yuv420 = RGBtoYUV420Planar;
			Rgba2Yuv420 = RGBtoYUV420Planar;
		}


		// Load Open h264 DLL
		HMODULE hDll = LoadLibrary(dllname);
		if (hDll == NULL)
		{
			auto err = GetLastError();
			bool _64 = is64Bit();

			throw gcnew System::DllNotFoundException(String::Format("Unable to load '{0}' Code: {1}", _64 ? "openh264-2.3.1-win64.dll" : "openh264-2.3.1-win64.dll", err));
		}
		dllname = nullptr;

		// Load Function
		CreateEncoderFunc = (WelsCreateSVCEncoder)GetProcAddress(hDll, "WelsCreateSVCEncoder");
		if (CreateEncoderFunc == NULL) 
		{
			auto err = GetLastError();
			throw gcnew System::DllNotFoundException(String::Format("Unable to load WelsCreateSVCEncoder func in WelsCreateSVCEncoderFunc Code: {0}'", err));
		}
		DestroyEncoderFunc = (WelsDestroySVCEncoder)GetProcAddress(hDll, "WelsDestroySVCEncoder");
		if (DestroyEncoderFunc == NULL) 
		{
			auto err = GetLastError();
			throw gcnew System::DllNotFoundException(String::Format("Unable to load WelsDestroySVCEncoder func in 'WelsDestroySVCEncoderFunc' Code{0}", err));
		}


		ISVCEncoder* enc = nullptr;
		int rc = CreateEncoderFunc(&enc);
		encoder = enc;
		if (rc != 0) throw gcnew System::DllNotFoundException(String::Format("Unable to call WelsCreateSVCEncoder func in '{0}'"));

	}

	int Encoder::Initialize(int width, int height, int bps, float fps, ConfigType configNo)
	{
		return InitializeInternal(width, height, bps, fps, configNo);
	};

	int Encoder::InitializeInternal(int width, int height, int bps, float fps, ConfigType configType)
	{
		int rc = 0;
		SEncParamBase base;
		memset(&base, 0, sizeof(SEncParamBase));
		int videoFormat = 0;
		SliceModeEnum sliceMode = SM_FIXEDSLCNUM_SLICE;
		SEncParamExt param;
		SSpatialLayerConfig* spatial_config = nullptr;
		memset(&param, 0, sizeof(SEncParamExt));
		switch (configType)
		{
		case ConfigType::CameraBasic:

			base.iPicWidth = width;
			base.iPicHeight = height;
			base.fMaxFrameRate = fps;
			base.iUsageType = CAMERA_VIDEO_REAL_TIME;
			base.iTargetBitrate = bps;
			base.iRCMode = RC_BITRATE_MODE;
			rc = encoder->Initialize(&base);
			break;

		case ConfigType::ScreenCaptureBasic:

			base.iPicWidth = width;
			base.iPicHeight = height;
			base.fMaxFrameRate = fps;
			base.iUsageType = SCREEN_CONTENT_REAL_TIME;
			base.iTargetBitrate = bps;
			base.iRCMode = RC_BITRATE_MODE;
			rc = encoder->Initialize(&base);
			break;

		case ConfigType::CameraCaptureAdvanced:

			encoder->GetDefaultParams(&param);
			param.iUsageType = CAMERA_VIDEO_REAL_TIME;
			param.iRCMode = RC_BITRATE_MODE;
			param.iComplexityMode = LOW_COMPLEXITY;
			param.iMinQp = 1;
			param.iMaxQp = 51;
			param.bEnableFrameSkip = false;
			param.bPrefixNalAddingCtrl = true;
			param.bIsLosslessLink = false;
			param.bEnableLongTermReference = true;
			param.uiMaxNalSize = 1500;

			spatial_config = &param.sSpatialLayers[0];
			spatial_config->uiProfileIdc = PRO_BASELINE;
			param.iPicWidth = spatial_config->iVideoWidth = width;
			param.iPicHeight = spatial_config->iVideoHeight = height;
			param.fMaxFrameRate = spatial_config->fFrameRate = fps;
			param.iTargetBitrate = spatial_config->iSpatialBitrate = bps;
			param.iMaxBitrate = spatial_config->iMaxSpatialBitrate = bps;
			spatial_config->sSliceArgument.uiSliceMode = SM_FIXEDSLCNUM_SLICE;
			spatial_config->sSliceArgument.uiSliceNum = 0;

			param.iMultipleThreadIdc = 2;

			param.iNumRefFrame = -1;
			param.uiIntraPeriod = -1; // 12 - FFMPEG
			param.iLTRRefNum = 0;
			param.iLtrMarkPeriod = 30;
			rc = encoder->InitializeExt(&param);
			videoFormat = videoFormatI420;
			rc = encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
			break;

		case ConfigType::ScreenCaptureAdvanced:
			param;
			encoder->GetDefaultParams(&param);
			param.iUsageType = SCREEN_CONTENT_REAL_TIME;
			param.fMaxFrameRate = fps;
			param.iPicWidth = width;
			param.iPicHeight = height;
			param.iTargetBitrate = bps;
			param.bEnableDenoise = false;
			param.iSpatialLayerNum = 1;
			param.bEnableAdaptiveQuant = false;
			param.bEnableFrameSkip = false;
			param.bPrefixNalAddingCtrl = false;
			param.bIsLosslessLink = false;
			param.bFixRCOverShoot = true;
			param.bEnableLongTermReference = false;
			param.iMinQp = 1;
			param.iMaxQp = 51;

			sliceMode = SM_FIXEDSLCNUM_SLICE;
			//SM_SIZELIMITED_SLICE with multi-thread is still under testing
			/*if (sliceMode != SM_SINGLE_SLICE && sliceMode != SM_SIZELIMITED_SLICE)*/
			param.iMultipleThreadIdc = 2;
			for (int i = 0; i < param.iSpatialLayerNum; i++) {
				param.sSpatialLayers[i].iVideoWidth = width >> (param.iSpatialLayerNum - 1 - i);
				param.sSpatialLayers[i].iVideoHeight = height >> (param.iSpatialLayerNum - 1 - i);
				param.sSpatialLayers[i].fFrameRate = fps;
				param.sSpatialLayers[i].iSpatialBitrate = param.iTargetBitrate;
				param.sSpatialLayers[i].sSliceArgument.uiSliceNum = 16;
				param.sSpatialLayers[i].sSliceArgument.uiSliceMode = sliceMode;
				if (sliceMode == SM_SIZELIMITED_SLICE) {
					param.sSpatialLayers[i].sSliceArgument.uiSliceSizeConstraint = 600;
					param.uiMaxNalSize = 1500;
				}
			}
			param.iTargetBitrate *= param.iSpatialLayerNum;
			rc = encoder->InitializeExt(&param);
			videoFormat = videoFormatI420;
			rc = encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
			break;

		default:

			base.iPicWidth = width;
			base.iPicHeight = height;
			base.fMaxFrameRate = fps;
			base.iUsageType = CAMERA_VIDEO_REAL_TIME;
			base.iTargetBitrate = bps;
			base.iRCMode = RC_BITRATE_MODE;
			rc = encoder->Initialize(&base);
			break;
		}

		if (rc != 0) return -1;

		buffer_size = width * height * 3 / 2;
		i420_buffer = new unsigned char[buffer_size];
		pic = new SSourcePicture();
		pic->iPicWidth = width;
		pic->iPicHeight = height;
		pic->iColorFormat = videoFormatI420;
		pic->iStride[0] = pic->iPicWidth;
		pic->iStride[1] = pic->iStride[2] = pic->iPicWidth >> 1;
		pic->pData[0] = i420_buffer;
		pic->pData[1] = pic->pData[0] + width * height;
		pic->pData[2] = pic->pData[1] + (width * height >> 2);

		bsi = new SFrameBSInfo();
		bool t = true;
		encoder->SetOption(ENCODER_OPTION_ENABLE_SSEI, &t);
		std::cout << "Encoder Set" << std::endl;
		return 0;
	};

#pragma endregion


	bool Encoder::Encode(Bitmap^ bmp, [Out]array<EncodedFrame^>^% frame)
	{
		if (pic->iPicWidth != bmp->Width || pic->iPicHeight != bmp->Height) throw gcnew System::ArgumentException("Image width and height must be same.");
		int width = bmp->Width;
		int height = bmp->Height;
		BitmapData^ bmpDate = bmp->LockBits(System::Drawing::Rectangle(0, 0, width, height), ImageLockMode::ReadOnly, System::Drawing::Imaging::PixelFormat::Format32bppArgb);
		byte* bmpScan = (byte*)bmpDate->Scan0.ToPointer();

		EnsureCapacity(width * height * 3);

		//Both 32bpp and 24 bpp encodes the pixels in 4 bytes.. Yea..
		if (bmp->PixelFormat == PixelFormat::Format32bppArgb || bmp->PixelFormat == PixelFormat::Format24bppRgb)
		{
			//auto t_start = std::chrono::high_resolution_clock::now();

			Bgra2Yuv420(bmpScan, innerBuffer, width, height, bmpDate->Stride);

		/*	auto t_end = std::chrono::high_resolution_clock::now();
			double elapsed_time_ms = std::chrono::duration<double, std::micro>(t_end - t_start).count();
			std::cout << elapsed_time_ms << std::endl;*/

			bmp->UnlockBits(bmpDate);
			return Encode(innerBuffer, frame);
		}
		else {
			throw gcnew System::NotImplementedException("Bmp Pixel format is not supported");
		}


	}


	bool Encoder::Encode(RgbImage^ rgb, [Out]array<EncodedFrame^>^% frame)
	{
		int width = rgb->Width;
		int height = rgb->Height;
		int stride = rgb->Stride;
		EnsureCapacity(width * height * 3);
		Rgb2Yuv420(rgb->ImageBytes, innerBuffer, width, height, stride);
		return Encode(innerBuffer, frame);

	}
	bool Encoder::Encode(RgbaImage^ rgb, [Out]array<EncodedFrame^>^% frame)
	{
		int width = rgb->Width;
		int height = rgb->Height;
		int stride = rgb->Stride;
		EnsureCapacity(width * height * 3);

		Rgba2Yuv420(rgb->ImageBytes, innerBuffer, width, height, stride);
		return Encode(innerBuffer, frame);

	}
	bool Encoder::Encode(BgrImage^ rgb, [Out]array<EncodedFrame^>^% frame)
	{
		int width = rgb->Width;
		int height = rgb->Height;
		int stride = rgb->Stride;
		EnsureCapacity(width * height * 3);

		//auto t_start = std::chrono::high_resolution_clock::now();

		Bgr2Yuv420(rgb->ImageBytes, innerBuffer, width, height, stride);

		//auto t_end = std::chrono::high_resolution_clock::now();
		//double elapsed_time_ms = std::chrono::duration<double, std::micro>(t_end - t_start).count();
		//std::cout << elapsed_time_ms << std::endl;
		return Encode(innerBuffer, frame);

	}

	bool Encoder::Encode(BgraImage^ rgb, [Out]array<EncodedFrame^>^% frame)
	{
		int width = rgb->Width;
		int height = rgb->Height;
		int stride = rgb->Stride;
		EnsureCapacity(width * height * 3);

		Bgra2Yuv420(rgb->ImageBytes, innerBuffer, width, height, stride);

		return Encode(innerBuffer, frame);

	}

	bool Encoder::Encode(array<Byte>^ i420, int startIndex, [Out]array<EncodedFrame^>^% frame)
	{
		pin_ptr<Byte> ptr = &i420[startIndex];
		bool res = Encode(innerBuffer, frame);

		ptr = nullptr; // unpin
		return res;
	}
	bool Encoder::Encode(IntPtr^ i420, array<EncodedFrame^>^% frame) {
		void* ptr = i420->ToPointer();
		unsigned char* p = reinterpret_cast<unsigned char*>(ptr);
		return Encode(p, frame);
	}

	bool Encoder::Encode(unsigned char* i420, [Out]array<EncodedFrame^>^% frame)
	{
		//memcpy(i420_buffer, i420, buffer_size);

		pic->pData[0] = i420;
		pic->pData[1] = pic->pData[0] + pic->iPicWidth * pic->iPicHeight;
		pic->pData[2] = pic->pData[1] + (pic->iPicWidth * pic->iPicHeight >> 2);

		bsi = new SFrameBSInfo();

		int resultCode = encoder->EncodeFrame(pic, bsi);
		if (resultCode != 0) {
			return false;
		}

		if (bsi->eFrameType != videoFrameTypeSkip) {
			frame = GetEncodedFrames(dynamic_cast<SFrameBSInfo%>(*bsi));
			return true;
		}
		return false;
	}

	array<EncodedFrame^>^ Encoder::GetEncodedFrames(const SFrameBSInfo% info)
	{
		array<EncodedFrame^>^ nals = gcnew array<EncodedFrame^>(info.iLayerNum);
		for (int i = 0; i < info.iLayerNum; ++i)
		{
			const SLayerBSInfo& layerInfo = info.sLayerInfo[i];
			int layerSize = 0;
			for (int j = 0; j < layerInfo.iNalCount; ++j)
			{
				layerSize += layerInfo.pNalLengthInByte[j];
			}

			nals[i] = gcnew EncodedFrame(layerInfo.pBsBuf, layerSize, i, (FrameType)info.eFrameType);
		}
		return nals;
	}

	int Encoder::ForceIntraFrame()
	{
		return encoder->ForceIntraFrame(true);
	}

	void H264Sharp::Encoder::SetMaxBitrate(int target)
	{
		SBitrateInfo param;

		memset(&param, 0, sizeof(SBitrateInfo));
		param.iBitrate = target;
		param.iLayer = SPATIAL_LAYER_ALL;
		encoder->SetOption(ENCODER_OPTION_MAX_BITRATE, &param);
		encoder->SetOption(ENCODER_OPTION_BITRATE, &param);
	}
	void H264Sharp::Encoder::SetTargetFps(float target)
	{
		
		encoder->SetOption(ENCODER_OPTION_FRAME_RATE, &target);
	}


	Encoder::~Encoder()
	{
		this->!Encoder();
	}

	Encoder::!Encoder()
	{
		encoder->Uninitialize();
		DestroyEncoderFunc(encoder);

		delete i420_buffer;
		delete pic;
		delete bsi;
		delete innerBuffer;
	}
	void Encoder::EnsureCapacity(int capacity)
	{
		if (innerBufLen == 0 || innerBufLen < capacity)
		{
			delete innerBuffer;
			innerBuffer = new byte[capacity];
			innerBufLen = capacity;
		}
	}

	//#pragma managed(pop)
}
