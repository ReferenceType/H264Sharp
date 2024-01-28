#include "pch.h"
#include <chrono>
#include <iostream>
#include "Encoder.h"


namespace H264Sharp {

#pragma region Setup

	Encoder::Encoder()
	{
		const wchar_t* dllName = is64Bit() ? L"openh264-2.3.1-win64.dll" : L"openh264-2.3.1-win32.dll";
		Create(dllName);
	}
	Encoder::Encoder(const wchar_t* dllname)
	{
		/*auto s = std::wstring(dllName.begin(),dllName.end());
		const wchar_t* dllname = s.c_str();*/
		Create(dllname);
	}
	void Encoder::Create(const wchar_t* dllname)
	{
		std::wcout << dllname << " loading\n";
		// Load Open h264 DLL
		//GetFullPathName(dllname);
		HMODULE hDll = LoadLibrary(dllname);
		if (hDll == NULL)
		{
			throw new std::exception("Failed to load Dll ", GetLastError());
		}

		// Load Function
		CreateEncoderFunc = (WelsCreateSVCEncoder)GetProcAddress(hDll, "WelsCreateSVCEncoder");
		if (CreateEncoderFunc == NULL)
		{
			throw new std::exception("Failed to load[WelsCreateSVCEncoder] method", GetLastError());
		}
		DestroyEncoderFunc = (WelsDestroySVCEncoder)GetProcAddress(hDll, "WelsDestroySVCEncoder");
		if (DestroyEncoderFunc == NULL)
		{
			throw new std::exception("Failed to load[WelsDestroySVCEncoder] method", GetLastError());
		}


		ISVCEncoder* enc = nullptr;
		int rc = CreateEncoderFunc(&enc);
		encoder = enc;
		if (rc != 0) throw new std::exception("Failed to load[WelsCreateSVCEncoder] method", GetLastError());

		std::wcout << dllname << " loaded\n";
		dllname = nullptr;

	}
	EncodedFrame* ef1;
	EncodedFrame* ef2;
	int Encoder::Initialize(int width, int height, int bps, float fps, ConfigType configNo)
	{
		 ef1 = new EncodedFrame[1];
		 ef2 = new EncodedFrame[2];
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

		pic = new SSourcePicture();
		bsi = new SFrameBSInfo();

		pic->iPicWidth = width;
		pic->iPicHeight = height;
		pic->iColorFormat = videoFormatI420;
		pic->iStride[0] = pic->iPicWidth;
		pic->iStride[1] = pic->iStride[2] = pic->iPicWidth >> 1;
		

		bool t = true;
		encoder->SetOption(ENCODER_OPTION_ENABLE_SSEI, &t);
		std::cout << "Encoder Set" << std::endl;
		return 0;
	};

#pragma endregion

	bool Encoder::Encode(GenericImage img, FrameContainer& frame) 
	{
		int width = img.Width;
		int height = img.Height;
		int stride = img.Stride;
		
		EnsureCapacity(width * height * 3);
		//return false;
		switch (img.Type)
		{
			case ImageType::Rgb:
				RGBtoYUV420Planar(img.ImageBytes, innerBuffer, width, height, stride);
				break;
			case ImageType::Bgr:
				BGRtoYUV420Planar(img.ImageBytes, innerBuffer, width, height, stride);
				break;
			case ImageType::Rgba:
				RGBAtoYUV420Planar(img.ImageBytes, innerBuffer, width, height, stride);
				break;
			case ImageType::Bgra:
				BGRAtoYUV420Planar(img.ImageBytes, innerBuffer, width, height, stride);
				break;
			default:
				break;
		}
	
		auto res = Encode(innerBuffer, frame);
		
		return res;
	}
	

	bool Encoder::Encode(unsigned char* i420, FrameContainer &frame)
	{
		//memcpy(i420_buffer, i420, buffer_size);

		pic->pData[0] = i420;
		pic->pData[1] = pic->pData[0] + pic->iPicWidth * pic->iPicHeight;
		pic->pData[2] = pic->pData[1] + (pic->iPicWidth * pic->iPicHeight >> 2);


		int resultCode = encoder->EncodeFrame(pic, bsi);
		if (resultCode != 0) {
			return false;
		}

		if (bsi->eFrameType != videoFrameTypeSkip && bsi->eFrameType != videoFrameTypeInvalid) {
			GetEncodedFrames(*bsi, frame);
			return true;
		}
		return false;
	}
	
	void Encoder::GetEncodedFrames(const SFrameBSInfo& info, FrameContainer& fc)
	{
		fc.Lenght = info.iLayerNum;
		if (info.iLayerNum == 1)
			fc.Frames = ef1;
		else if(info.iLayerNum == 2)
			fc.Frames = ef2;
		else
			fc.Frames = new EncodedFrame[info.iLayerNum];

		for (int i = 0; i < info.iLayerNum; ++i)
		{
			const SLayerBSInfo& layerInfo = info.sLayerInfo[i];
			int layerSize = 0;
			for (int j = 0; j < layerInfo.iNalCount; ++j)
			{
				layerSize += layerInfo.pNalLengthInByte[j];
			}
			
			fc.Frames[i] = EncodedFrame(layerInfo.pBsBuf, layerSize, i, (FrameType)info.eFrameType);	
		}

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
		encoder->Uninitialize();
		DestroyEncoderFunc(encoder);

		delete pic;
		delete bsi;
		delete[] innerBuffer;
		delete ef1;
		delete ef2;
	}

	void Encoder::EnsureCapacity(int capacity)
	{
		//std::cout << "Ensuring";
		if (innerBufLen == 0 || innerBufLen < capacity)
		{
			if (innerBuffer != nullptr) {
				//std::cout << "deleting";
				delete[] innerBuffer;
			}
			//std::cout << "alloc";
			innerBuffer = new byte[capacity];
			innerBufLen = capacity;
		}
	}

}
