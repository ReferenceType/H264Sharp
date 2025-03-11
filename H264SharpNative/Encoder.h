#ifndef ENCODER
#define ENCODER
#include "pch.h"
#include <chrono>
#include <iostream>
#include "string.h"
#include "EncodedFrame.h"
#include "ImageTypes.h"
#include "Converter.h"
#include <unordered_map>



namespace H264Sharp {

	enum class ConfigType { CameraBasic, ScreenCaptureBasic, CameraCaptureAdvanced, ScreenCaptureAdvanced,
		CameraCaptureAdvancedHP, ScreenCaptureAdvancedHP};

	class Encoder
	{
		public:
			Encoder(const char* dllname);

			Encoder();
			~Encoder();

			int Initialize(int width, int height, int bps, float fps, ConfigType configNo);
			int Initialize(SEncParamBase base);
			int GetDefaultParams(SEncParamExt &params);
			int Initialize(SEncParamExt params);

			int SetOption(ENCODER_OPTION option, void* value);
			int GetOption(ENCODER_OPTION option, void* value);

			int Encode(GenericImage img, FrameContainer& frame);
			int Encode(unsigned char* i420, FrameContainer &frame);
			int Encode(YuvNative* i420, FrameContainer &frame);
			int Encode(YuvNV12Native* i420, FrameContainer &frame);


			int ForceIntraFrame();
			void SetMaxBitrate(int target);
			void SetTargetFps(float target);
			static int EnableDebugLogs;


	private:
		int buffer_size = 0;
		unsigned char* innerBuffer = nullptr;
		int innerBufLen = 0;

		ISVCEncoder* encoder = nullptr;
		SSourcePicture pic;
		SFrameBSInfo bsi;
		std::unordered_map<int, EncodedFrame*> efm;

		typedef int(* WelsCreateSVCEncoder)(ISVCEncoder** ppEncoder);
		WelsCreateSVCEncoder CreateEncoderFunc;
		typedef void(* WelsDestroySVCEncoder)(ISVCEncoder* ppEncoder);
		WelsDestroySVCEncoder DestroyEncoderFunc;

		void Create(const char* dllName);
		int InitializeInternal(int width, int height, int bps, float fps, ConfigType configType);
		void EnsureCapacity(int capacity);
		void GetEncodedFrames( FrameContainer &fc);
		void PrintParam(const TagEncParamExt& param);

		
	};
}
#endif
