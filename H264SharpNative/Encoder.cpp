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
	Encoder::Encoder(std::string dllName)
	{
		auto s = std::wstring(dllName.begin(), dllName.end());
		const wchar_t* dllname = s.c_str();
		Create(dllname);
	}
	Encoder::Encoder(const wchar_t* dllname)
	{
		/*auto s = std::wstring(dllName.begin(),dllName.end());
		const wchar_t* dllname = s.c_str();*/
		Create(dllname);
	}
	EncodedFrame* ef1;
	EncodedFrame* ef2;
	EncodedFrame* ef3;
	EncodedFrame* ef4;
	EncodedFrame* ef5;
	void Encoder::Create(const wchar_t* dllname)
	{
		std::cout << "New Version " << " loading\n";

		std::cout << dllname << " loading\n";

		// Load dynamic library
#ifdef _WIN32
		HMODULE handle = DLL_LOAD_FUNCTION(dllname);
#else
		void* handle = DLL_LOAD_FUNCTION(dllname, RTLD_LAZY);
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

		std::cout << dllname << " loaded\n";

		// Close library handle
#ifdef _WIN32
		//DLL_CLOSE_FUNCTION(handle);
#else
// No need to close on Linux
#endif
		std::wcout << dllname << " loaded\n";
		dllname = nullptr;

		ef1 = new EncodedFrame[1];
		ef2 = new EncodedFrame[2];
		ef3 = new EncodedFrame[3];
		ef4 = new EncodedFrame[4];
		ef5 = new EncodedFrame[5];
	}
	//void Encoder::Create(const wchar_t* dllname)
	//{
	//	std::wcout << dllname << " loading\n";
	//	// Load Open h264 DLL
	//	//GetFullPathName(dllname);
	//	HMODULE hDll = LoadLibrary(dllname);
	//	if (hDll == NULL)
	//	{
	//		throw new std::exception("Failed to load Dll ", GetLastError());
	//	}

	//	// Load Function
	//	CreateEncoderFunc = (WelsCreateSVCEncoder)GetProcAddress(hDll, "WelsCreateSVCEncoder");
	//	if (CreateEncoderFunc == NULL)
	//	{
	//		throw new std::exception("Failed to load[WelsCreateSVCEncoder] method", GetLastError());
	//	}
	//	DestroyEncoderFunc = (WelsDestroySVCEncoder)GetProcAddress(hDll, "WelsDestroySVCEncoder");
	//	if (DestroyEncoderFunc == NULL)
	//	{
	//		throw new std::exception("Failed to load[WelsDestroySVCEncoder] method", GetLastError());
	//	}


	//	ISVCEncoder* enc = nullptr;
	//	int rc = CreateEncoderFunc(&enc);
	//	encoder = enc;
	//	if (rc != 0) throw new std::exception("Failed to load[WelsCreateSVCEncoder] method", GetLastError());

	//	std::wcout << dllname << " loaded\n";
	//	dllname = nullptr;

	//	ef1 = new EncodedFrame[1];
	//	ef2 = new EncodedFrame[2];
	//	ef3 = new EncodedFrame[3];
	//	ef4 = new EncodedFrame[4];
	//	ef5 = new EncodedFrame[5];
	//}
	
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

		pic = new SSourcePicture();
		bsi = new SFrameBSInfo();

		pic->iPicWidth = base.iPicWidth;
		pic->iPicHeight = base.iPicHeight;
		pic->iColorFormat = videoFormatI420;
		pic->iStride[0] = pic->iPicWidth;
		pic->iStride[1] = pic->iStride[2] = pic->iPicWidth >> 1;


		bool t = true;
		encoder->SetOption(ENCODER_OPTION_ENABLE_SSEI, &t);
		std::cout << "Encoder Set" << std::endl;
		return rc;

	}
	int Encoder::Initialize(SEncParamExt params)
	{
		
		//memcpy(dest_struct, source_struct, sizeof(*dest_struct));
		
		auto rc= encoder->InitializeExt(&params);
		auto videoFormat = videoFormatI420;
		 encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

		 pic = new SSourcePicture();
		 bsi = new SFrameBSInfo();

		 pic->iPicWidth = params.iPicWidth;
		 pic->iPicHeight = params.iPicHeight;
		 pic->iColorFormat = videoFormatI420;
		 pic->iStride[0] = pic->iPicWidth;
		 pic->iStride[1] = pic->iStride[2] = pic->iPicWidth >> 1;


		 bool t = true;
		 encoder->SetOption(ENCODER_OPTION_ENABLE_SSEI, &t);
		 TagEncParamExt param = params;



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
				 std::cout << "- SpatialLayer/SliceArg " << i << " : " << "uiSliceMbNum"<<j << " " << param.sSpatialLayers[i].sSliceArgument.uiSliceMbNum[j] << "\n";

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
			param.iPicWidth = width;
			param.iPicHeight = height;
			param.iTargetBitrate = bps;
			param.iTemporalLayerNum = 1;
			param.iSpatialLayerNum = 1;
			param.iRCMode = RC_BITRATE_MODE;

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

			//param.iUsageType = CAMERA_VIDEO_REAL_TIME;
			//param.iRCMode = RC_BITRATE_MODE;
			//param.iComplexityMode = LOW_COMPLEXITY;
			//param.iMinQp = 1;
			//param.iMaxQp = 51;
			//param.bEnableFrameSkip = true;
			//param.bPrefixNalAddingCtrl = true;
			//param.bIsLosslessLink = false;
			//param.bEnableLongTermReference = true;
			//param.uiMaxNalSize = 1500;

			//spatial_config = &param.sSpatialLayers[0];
			//spatial_config->uiProfileIdc = PRO_BASELINE;
			//param.iPicWidth = spatial_config->iVideoWidth = width;
			//param.iPicHeight = spatial_config->iVideoHeight = height;
			//param.fMaxFrameRate = spatial_config->fFrameRate = fps;
			//param.iTargetBitrate = spatial_config->iSpatialBitrate = bps;
			//param.iMaxBitrate = spatial_config->iMaxSpatialBitrate = bps;
			//spatial_config->sSliceArgument.uiSliceMode = SM_FIXEDSLCNUM_SLICE;
			//spatial_config->sSliceArgument.uiSliceNum = 0;

			//param.iMultipleThreadIdc = 0;

			//param.iNumRefFrame = -1;
			//param.uiIntraPeriod = -1; // 12 - FFMPEG
			//param.iLTRRefNum = 0;
			//param.iLtrMarkPeriod = 30;

			rc = encoder->InitializeExt(&param);
			videoFormat = videoFormatI420;
			rc &= encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
			std::cout << "ADV param Encoder Set" << std::endl;
			break;

		case ConfigType::ScreenCaptureAdvanced:
			encoder->GetDefaultParams(&param);
			param.iUsageType = SCREEN_CONTENT_REAL_TIME;
			param.iPicWidth = width;
			param.iPicHeight = height;
			param.iTargetBitrate = bps;
			param.iTemporalLayerNum = 1;
			param.iSpatialLayerNum = 1;
			param.iRCMode = RC_BITRATE_MODE;

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
		//auto t_start = std::chrono::high_resolution_clock::now();

		switch (img.Type)
		{
			case ImageType::Rgb:
				RGBtoYUV420Planar(img.ImageBytes, innerBuffer, width, height, stride, threadCount);
				break;
			case ImageType::Bgr:
				BGRtoYUV420Planar(img.ImageBytes, innerBuffer, width, height, stride, threadCount);
				break;
			case ImageType::Rgba:
				RGBAtoYUV420Planar(img.ImageBytes, innerBuffer, width, height, stride, threadCount);
				break;
			case ImageType::Bgra:
				BGRAtoYUV420Planar(img.ImageBytes, innerBuffer, width, height, stride, threadCount);
				break;
			default:
				break;
		}
		/*auto t_end = std::chrono::high_resolution_clock::now();
		double elapsed_time_ms = std::chrono::duration<double, std::micro>(t_end - t_start).count();
		std::cout <<"converted " << elapsed_time_ms << std::endl;*/

		auto res = Encode(innerBuffer, frame);
		
		return res;
	}

	

	bool Encoder::Encode(unsigned char* i420, FrameContainer &frame)
	{
		//memcpy(i420_buffer, i420, buffer_size);

		pic->pData[0] = i420;
		pic->pData[1] = pic->pData[0] + pic->iPicWidth * pic->iPicHeight;
		pic->pData[2] = pic->pData[1] + (pic->iPicWidth * pic->iPicHeight >> 2);// /2


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
	
	
	void Encoder::GetEncodedFrames(const SFrameBSInfo& bsi, FrameContainer& fc)
	{
		fc.Lenght = bsi.iLayerNum;
		
		switch (bsi.iLayerNum)
		{
		case 1:
			fc.Frames = ef1;
			break;
		case 2:
			fc.Frames = ef2;
			break;
		case 3:
			fc.Frames = ef3;
			break;
		case 4:
			fc.Frames = ef4;
			break;
		case 5:
			fc.Frames = ef5;
			break;
		default:
			fc.Frames = new EncodedFrame[bsi.iLayerNum];
			break;
		}
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

		delete pic;
		delete bsi;
		delete[] innerBuffer;
		delete ef1;
		delete ef2;
		delete ef3;
		delete ef4;
		delete ef5;
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
