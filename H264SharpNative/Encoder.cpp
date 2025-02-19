#include "Encoder.h"

namespace H264Sharp {
	
#pragma region Setup
	int Encoder::EnableDebugLogs = 0;
	Encoder::Encoder()
	{
		const char* dllName = is64Bit() ? "openh264-2.4.1-win64.dll" : "openh264-2.4.1-win32.dll";
		Create(dllName);
	}

	Encoder::Encoder(const char* dllname)
	{
		Create(dllname);
	}

	void Encoder::Create(const char* dllName)
	{
		if(Encoder::EnableDebugLogs>0)
			std::cout << "Encoder [" << dllName << "] loading..\n";
		// Load dynamic library
#ifdef _WIN32
		int wcharCount = MultiByteToWideChar(CP_UTF8, 0, dllName, -1, nullptr, 0);
		wchar_t* wideDllName = new wchar_t[wcharCount];
		MultiByteToWideChar(CP_UTF8, 0, dllName, -1, wideDllName, wcharCount);

		HMODULE handle = DLL_LOAD_FUNCTION(wideDllName);
		delete[] wideDllName;

#else
		void* handle = DLL_LOAD_FUNCTION(dllName, RTLD_LAZY);
#endif
		if (handle == NULL)
		{
#ifdef _WIN32
			throw std::runtime_error("Failed to load library");
#else
			throw std::runtime_error(DLL_ERROR_CODE);
#endif
		}

		// Load Function
		CreateEncoderFunc = reinterpret_cast<WelsCreateSVCEncoder>(DLL_GET_FUNCTION(handle, "WelsCreateSVCEncoder"));
		if (CreateEncoderFunc == NULL)
		{
#ifdef _WIN32
			throw std::runtime_error("Failed to load [WelsCreateSVCEncoder] method");
#else
			throw std::runtime_error(DLL_ERROR_CODE);
#endif
		}
		DestroyEncoderFunc = reinterpret_cast<WelsDestroySVCEncoder>(DLL_GET_FUNCTION(handle, "WelsDestroySVCEncoder"));
		if (DestroyEncoderFunc == NULL)
		{
#ifdef _WIN32
			throw std::runtime_error("Failed to load [WelsDestroySVCEncoder] method");
#else
			throw std::runtime_error(DLL_ERROR_CODE);
#endif
		}

		ISVCEncoder* enc = nullptr;
		int rc = CreateEncoderFunc(&enc);
		encoder = enc;
		if (rc != 0) throw std::runtime_error("Failed to create encoder");
		if (Encoder::EnableDebugLogs > 0)
			std::cout << dllName << " loaded\n";
		dllName = nullptr;
	}


	int Encoder::Initialize(int width, int height, int bps, float fps, ConfigType configNo)
	{
		return InitializeInternal(width, height, bps, fps, configNo);
	}

	int Encoder::GetDefaultParams(SEncParamExt& params)
	{
		return encoder->GetDefaultParams(&params);
	}

	int Encoder::Initialize(SEncParamBase base)
	{

		auto rc = encoder->Initialize(&base);
		auto videoFormat = videoFormatI420;
		encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

		

		pic.iPicWidth = base.iPicWidth;
		pic.iPicHeight = base.iPicHeight;
		pic.iColorFormat = videoFormatI420;
		pic.iStride[0] = pic.iPicWidth;
		pic.iStride[1] = pic.iStride[2] = pic.iPicWidth >> 1;


		bool t = true;
		encoder->SetOption(ENCODER_OPTION_ENABLE_SSEI, &t);
		if (Encoder::EnableDebugLogs > 0)
			std::cout << "Encoder Set" << std::endl;
		return rc;

	}
	int Encoder::Initialize(SEncParamExt params)
	{
		//memcpy(dest_struct, source_struct, sizeof(*dest_struct));

		auto rc = encoder->InitializeExt(&params);
		auto videoFormat = videoFormatI420;
		encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

		memset(&bsi, 0, sizeof(SFrameBSInfo));
		memset(&pic, 0, sizeof(SSourcePicture));

		pic.iPicWidth = params.iPicWidth;
		pic.iPicHeight = params.iPicHeight;
		pic.iColorFormat = videoFormatI420;
		pic.iStride[0] = pic.iPicWidth;
		pic.iStride[1] = pic.iStride[2] = pic.iPicWidth >> 1;


		bool t = true;
		encoder->SetOption(ENCODER_OPTION_ENABLE_SSEI, &t);
		PrintParam(params);
		return rc;
	}


	int Encoder::SetOption(ENCODER_OPTION option, void* value)
	{
		return encoder->SetOption(option, value);
	}

	int Encoder::GetOption(ENCODER_OPTION option, void* value)
	{
		return encoder->GetOption(option, value);
	}

	int Encoder::InitializeInternal(int width, int height, int bps, float fps, ConfigType configType)
	{
		int rc = 0;
		int videoFormat = 0;

		SEncParamBase base;
		memset(&base, 0, sizeof(SEncParamBase));

		SEncParamExt param;
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
			rc += encoder->Initialize(&base);
			break;

		case ConfigType::ScreenCaptureBasic:

			base.iPicWidth = width;
			base.iPicHeight = height;
			base.fMaxFrameRate = fps;
			base.iUsageType = SCREEN_CONTENT_REAL_TIME;
			base.iTargetBitrate = bps;
			base.iRCMode = RC_BITRATE_MODE;
			rc += encoder->Initialize(&base);
			break;

		case ConfigType::CameraCaptureAdvanced:

			encoder->GetDefaultParams(&param);

			param.iUsageType = CAMERA_VIDEO_REAL_TIME;
			param.iPicWidth = width;
			param.iPicHeight = height;
			param.iTargetBitrate = bps;
			param.iTemporalLayerNum = 1;
			param.iSpatialLayerNum = 1;
			param.iRCMode = RC_QUALITY_MODE;

			param.sSpatialLayers[0].iVideoWidth = 0;
			param.sSpatialLayers[0].iVideoWidth = 0;
			param.sSpatialLayers[0].fFrameRate = 60;
			param.sSpatialLayers[0].iSpatialBitrate = bps;
			param.sSpatialLayers[0].uiProfileIdc = PRO_HIGH;
			param.sSpatialLayers[0].uiLevelIdc = LEVEL_UNKNOWN;
			param.sSpatialLayers[0].iDLayerQp = 0;


			param.iComplexityMode = HIGH_COMPLEXITY;
			param.uiIntraPeriod = 0;
			param.iNumRefFrame = 0;
			param.eSpsPpsIdStrategy = SPS_LISTING_AND_PPS_INCREASING;
			param.bPrefixNalAddingCtrl = false;
			param.bEnableSSEI = true;
			param.bSimulcastAVC = false;
			param.iPaddingFlag = 0;
			param.iEntropyCodingModeFlag = 1;
			param.bEnableFrameSkip = false;
			param.iMaxBitrate = 0;
			param.iMinQp = 0;
			param.iMaxQp = 51;
			param.uiMaxNalSize = 0;
			param.bEnableLongTermReference = true;
			param.iLTRRefNum = 1;
			param.iLtrMarkPeriod = 180;
			param.iMultipleThreadIdc = 1;
			param.bUseLoadBalancing = true;

			param.bEnableDenoise = false;
			param.bEnableBackgroundDetection = true;
			param.bEnableAdaptiveQuant = true;
			param.bEnableSceneChangeDetect = true;
			param.bIsLosslessLink = false;
			param.bFixRCOverShoot = true;
			param.iIdrBitrateRatio = 400;
			param.fMaxFrameRate = fps;

			rc += encoder->InitializeExt(&param);
			videoFormat = videoFormatI420;
			rc += encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
			PrintParam(param);
			if (Encoder::EnableDebugLogs > 0)
				std::cout << "Advanced param Encoder Set" << std::endl;
			break;

		case ConfigType::ScreenCaptureAdvanced:
			encoder->GetDefaultParams(&param);
			param.iUsageType = SCREEN_CONTENT_REAL_TIME;
			param.iPicWidth = width;
			param.iPicHeight = height;
			param.iTargetBitrate = bps;
			param.iTemporalLayerNum = 1;
			param.iSpatialLayerNum = 1;
			param.iRCMode = RC_QUALITY_MODE;

			param.sSpatialLayers[0].iVideoWidth = 0;
			param.sSpatialLayers[0].iVideoWidth = 0;
			param.sSpatialLayers[0].fFrameRate = 60;
			param.sSpatialLayers[0].iSpatialBitrate = bps;
			param.sSpatialLayers[0].uiProfileIdc = PRO_HIGH;
			param.sSpatialLayers[0].uiLevelIdc = LEVEL_UNKNOWN;
			param.sSpatialLayers[0].iDLayerQp = 0;


			param.iComplexityMode = HIGH_COMPLEXITY;
			param.uiIntraPeriod = 0;
			param.iNumRefFrame = 0;
			param.eSpsPpsIdStrategy = INCREASING_ID;
			param.bPrefixNalAddingCtrl = false;
			param.bEnableSSEI = true;
			param.bSimulcastAVC = false;
			param.iPaddingFlag = 0;
			param.iEntropyCodingModeFlag = 1;
			param.bEnableFrameSkip = false;
			param.iMaxBitrate = 0;
			param.iMinQp = 0;
			param.iMaxQp = 51;
			param.uiMaxNalSize = 0;
			param.bEnableLongTermReference = true;
			param.iLTRRefNum = 1;
			param.iLtrMarkPeriod = 180;
			param.iMultipleThreadIdc = 0;
			param.bUseLoadBalancing = true;

			param.bEnableDenoise = false;
			param.bEnableBackgroundDetection = true;
			param.bEnableAdaptiveQuant = true;
			param.bEnableSceneChangeDetect = true;
			param.bIsLosslessLink = false;
			param.bFixRCOverShoot = true;
			param.iIdrBitrateRatio = 100;
			param.fMaxFrameRate = 30;

			rc += encoder->InitializeExt(&param);
			videoFormat = videoFormatI420;
			rc += encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
			if (Encoder::EnableDebugLogs > 0)
				std::cout << "Advanced param Encoder Set" << std::endl;
			PrintParam(param);
			break;

		default:

			base.iPicWidth = width;
			base.iPicHeight = height;
			base.fMaxFrameRate = fps;
			base.iUsageType = CAMERA_VIDEO_REAL_TIME;
			base.iTargetBitrate = bps;
			base.iRCMode = RC_BITRATE_MODE;
			rc += encoder->Initialize(&base);
			break;
		}

		if (rc != 0) return -1;
		memset(&pic, 0, sizeof(SSourcePicture));
		memset(&bsi, 0, sizeof(SFrameBSInfo));

		pic.iPicWidth = width;
		pic.iPicHeight = height;
		pic.iColorFormat = videoFormatI420;
		pic.iStride[0] = pic.iPicWidth;
		pic.iStride[1] = pic.iStride[2] = pic.iPicWidth >> 1;


		bool t = true;
		encoder->SetOption(ENCODER_OPTION_ENABLE_SSEI, &t);
		if (Encoder::EnableDebugLogs > 0)
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

		switch (img.Type)
		{
		case ImageType::Rgb:
			Converter::RGBXtoYUV420Planar<3,true>(img.ImageBytes, innerBuffer, width, height, stride);
			break;
		case ImageType::Bgr:
			Converter::RGBXtoYUV420Planar<3, false>(img.ImageBytes, innerBuffer, width, height, stride);
			break;
		case ImageType::Rgba:
			Converter::RGBXtoYUV420Planar<4, true>(img.ImageBytes, innerBuffer, width, height, stride);
			break;
		case ImageType::Bgra:
			Converter::RGBXtoYUV420Planar<4, false>(img.ImageBytes, innerBuffer, width, height, stride);
			break;
		default:
			break;
		}

		auto res = Encode(innerBuffer, frame);

		return res;
	}

	bool Encoder::Encode(YuvNative* yuv, FrameContainer& frame)
	{
		//TODO here be converter in case its nv12!
		SSourcePicture pic_;
		memset(&pic_, 0, sizeof(SSourcePicture));

		pic_.pData[0] = yuv->V;
		pic_.pData[1] = yuv->U;
		pic_.pData[2] = yuv->V;
		pic_.iStride[0] = yuv->yStride;
		pic_.iStride[1] = yuv->uvStride;
		pic_.iStride[2] = yuv->uvStride;
		pic_.iPicWidth = yuv->width;
		pic_.iPicHeight = yuv->height;
		pic_.iColorFormat = videoFormatI420;

		int resultCode = encoder->EncodeFrame(&pic_, &bsi);
		if (resultCode != 0) {
			return false;
		}

		if (bsi.eFrameType != videoFrameTypeSkip && bsi.eFrameType != videoFrameTypeInvalid) {
			GetEncodedFrames(frame);
			return true;
		}

		return false;
	}

	bool Encoder::Encode(unsigned char* i420, FrameContainer& frame)
	{
		//memcpy(i420_buffer, i420, buffer_size);


		pic.pData[0] = i420;
		pic.pData[1] = pic.pData[0] + pic.iPicWidth * pic.iPicHeight;
		pic.pData[2] = pic.pData[1] + (pic.iPicWidth * pic.iPicHeight >> 2);// /4


		int resultCode = encoder->EncodeFrame(&pic, &bsi);
		if (resultCode != 0) {
			return false;
		}

		if (bsi.eFrameType != videoFrameTypeSkip && bsi.eFrameType != videoFrameTypeInvalid) {
			GetEncodedFrames( frame);
			return true;
		}

		return false;
	}


	void Encoder::GetEncodedFrames( FrameContainer& fc)
	{
		fc.Lenght = bsi.iLayerNum;

		auto it = efm.find(bsi.iLayerNum);
		if (it == efm.end())
		{
			it = efm.emplace(bsi.iLayerNum, new EncodedFrame[bsi.iLayerNum]).first;
		}

		fc.Frames = it->second;
		
		for (int i = 0; i < bsi.iLayerNum; ++i)
		{
			const SLayerBSInfo& layerInfo = bsi.sLayerInfo[i];
			int layerSize = 0;
			//std::cout << "NAL CNT" << layerInfo.iNalCount << "\n";

			for (int j = 0; j < layerInfo.iNalCount; ++j)
			{
				layerSize += layerInfo.pNalLengthInByte[j];
				//std::cout << "NAL Len" << layerInfo.pNalLengthInByte[j] << "\n";
			}

			fc.Frames[i] = EncodedFrame(layerInfo.pBsBuf, layerSize, i, bsi);
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

		FreeAllignAlloc(innerBuffer);
		for (auto& it : efm)
		{
			delete[] it.second;
		}
	}

	void Encoder::EnsureCapacity(int capacity)
	{
		if ( innerBufLen < capacity)
		{
			if (innerBuffer != nullptr) 
			{
				FreeAllignAlloc(innerBuffer);
			}
			innerBuffer = (unsigned char*)AllignAlloc(capacity);
			innerBufLen = capacity;
		}
	}

	void Encoder::PrintParam(const TagEncParamExt& param)
	{
		if (Encoder::EnableDebugLogs > 0) {

			std::cout << "iUsageType" << " " << param.iUsageType << "\n";
			std::cout << "iPicWidth" << " " << param.iPicWidth << "\n";
			std::cout << "iPicHeight" << " " << param.iPicHeight << "\n";
			std::cout << "iTargetBitrate" << " " << param.iTargetBitrate << "\n";
			std::cout << "iRCMode" << " " << param.iRCMode << "\n";
			std::cout << "fMaxFrameRate" << " " << param.fMaxFrameRate << "\n";
			std::cout << "iTemporalLayerNum" << " " << param.iTemporalLayerNum << "\n";
			std::cout << "iSpatialLayerNum" << " " << param.iSpatialLayerNum << "\n";
			std::cout << "iSpatialLayerNum" << " " << param.iSpatialLayerNum << "\n";
			// struct 4
			for (size_t i = 0; i < 4; i++)
			{
				std::cout << "- SpatialLayer " << i << " : " << "iVideoWidth" << " " << param.sSpatialLayers[i].iVideoWidth << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "iVideoHeight" << " " << param.sSpatialLayers[i].iVideoHeight << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "fFrameRate" << " " << param.sSpatialLayers[i].fFrameRate << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "iSpatialBitrate" << " " << param.sSpatialLayers[i].iSpatialBitrate << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "iMaxSpatialBitrate" << " " << param.sSpatialLayers[i].iMaxSpatialBitrate << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "uiProfileIdc" << " " << param.sSpatialLayers[i].uiProfileIdc << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "uiLevelIdc" << " " << param.sSpatialLayers[i].uiLevelIdc << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "iDLayerQp" << " " << param.sSpatialLayers[i].iDLayerQp << "\n";

				for (size_t j = 0; j < 35; j++)
				{
					std::cout << "- SpatialLayer/SliceArg " << i << " : " << "uiSliceMbNum" << j << " " << param.sSpatialLayers[i].sSliceArgument.uiSliceMbNum[j] << "\n";
				}

				std::cout << "- SpatialLayer/SliceArg " << i << " : " << "uiSliceMode" << " " << param.sSpatialLayers[i].sSliceArgument.uiSliceMode << "\n";
				std::cout << "- SpatialLayer/SliceArg " << i << " : " << "uiSliceNum" << " " << param.sSpatialLayers[i].sSliceArgument.uiSliceNum << "\n";
				std::cout << "- SpatialLayer/SliceArg " << i << " : " << "uiSliceSizeConstraint" << " " << param.sSpatialLayers[i].sSliceArgument.uiSliceSizeConstraint << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "bVideoSignalTypePresent" << " " << param.sSpatialLayers[i].bVideoSignalTypePresent << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "uiVideoFormat" << " " << param.sSpatialLayers[i].uiVideoFormat << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "bFullRange" << " " << param.sSpatialLayers[i].bFullRange << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "bColorDescriptionPresent" << " " << param.sSpatialLayers[i].bColorDescriptionPresent << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "uiColorPrimaries" << " " << param.sSpatialLayers[i].uiColorPrimaries << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "uiTransferCharacteristics" << " " << param.sSpatialLayers[i].uiTransferCharacteristics << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "uiColorMatrix" << " " << param.sSpatialLayers[i].uiColorMatrix << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "bAspectRatioPresent" << " " << param.sSpatialLayers[i].bAspectRatioPresent << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "eAspectRatio" << " " << param.sSpatialLayers[i].eAspectRatio << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "sAspectRatioExtWidth" << " " << param.sSpatialLayers[i].sAspectRatioExtWidth << "\n";
				std::cout << "- SpatialLayer " << i << " : " << "sAspectRatioExtHeight" << " " << param.sSpatialLayers[i].sAspectRatioExtHeight << "\n";

			}

			//
			std::cout << "iComplexityMode" << " " << param.iComplexityMode << "\n";
			std::cout << "uiIntraPeriod" << " " << param.uiIntraPeriod << "\n";
			std::cout << "iNumRefFrame" << " " << param.iNumRefFrame << "\n";
			std::cout << "eSpsPpsIdStrategy" << " " << param.eSpsPpsIdStrategy << "\n";
			std::cout << "bPrefixNalAddingCtrl" << " " << param.bPrefixNalAddingCtrl << "\n";
			std::cout << "bEnableSSEI" << " " << param.bEnableSSEI << "\n";
			std::cout << "bSimulcastAVC" << " " << param.bSimulcastAVC << "\n";
			std::cout << "iPaddingFlag" << " " << param.iPaddingFlag << "\n";
			std::cout << "iEntropyCodingModeFlag" << " " << param.iEntropyCodingModeFlag << "\n";
			std::cout << "bEnableFrameSkip" << " " << param.bEnableFrameSkip << "\n";
			std::cout << "iMaxBitrate" << " " << param.iMaxBitrate << "\n";
			std::cout << "iMaxQp" << " " << param.iMaxQp << "\n";
			std::cout << "iMinQp" << " " << param.iMinQp << "\n";
			std::cout << "uiMaxNalSize" << " " << param.uiMaxNalSize << "\n";
			std::cout << "bEnableLongTermReference" << " " << param.bEnableLongTermReference << "\n";
			std::cout << "iLTRRefNum" << " " << param.iLTRRefNum << "\n";
			std::cout << "iLtrMarkPeriod" << " " << param.iLtrMarkPeriod << "\n";
			std::cout << "iMultipleThreadIdc" << " " << param.iMultipleThreadIdc << "\n";
			std::cout << "bUseLoadBalancing" << " " << param.bUseLoadBalancing << "\n";
			std::cout << "iLoopFilterDisableIdc" << " " << param.iLoopFilterDisableIdc << "\n";
			std::cout << "iLoopFilterAlphaC0Offset" << " " << param.iLoopFilterAlphaC0Offset << "\n";
			std::cout << "iLoopFilterBetaOffset" << " " << param.iLoopFilterBetaOffset << "\n";
			std::cout << "bEnableDenoise" << " " << param.bEnableDenoise << "\n";
			std::cout << "bEnableBackgroundDetection" << " " << param.bEnableBackgroundDetection << "\n";
			std::cout << "bEnableAdaptiveQuant" << " " << param.bEnableAdaptiveQuant << "\n";
			std::cout << "bEnableFrameCroppingFlag" << " " << param.bEnableFrameCroppingFlag << "\n";
			std::cout << "bEnableSceneChangeDetect" << " " << param.bEnableSceneChangeDetect << "\n";
			std::cout << "bIsLosslessLink" << " " << param.bIsLosslessLink << "\n";
			std::cout << "bFixRCOverShoot" << " " << param.bFixRCOverShoot << "\n";
			std::cout << "iIdrBitrateRatio" << " " << param.iIdrBitrateRatio << "\n";
		}
	}

}
