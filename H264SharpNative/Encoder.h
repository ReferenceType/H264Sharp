#ifndef ENCODER
#define ENCODER
#include "pch.h"
#include <chrono>
#include <iostream>
#include "Encoder.h"
#include "string.h"
#include "EncodedFrame.h"
#include "ImageTypes.h"
#include "ConverterLocal.h"


namespace H264Sharp {

	
	enum class ConfigType { CameraBasic, ScreenCaptureBasic, CameraCaptureAdvanced, ScreenCaptureAdvanced };

	class Encoder
	{
		public:
			Encoder(const wchar_t* dllname);
			Encoder(std::string dllName);

			Encoder();
			~Encoder();

			int Initialize(int width, int height, int bps, float fps, ConfigType configNo);
			int Initialize(SEncParamBase base);
			int GetDefaultParams(SEncParamExt &params);
			int Initialize(SEncParamExt params);

			int SetOption(ENCODER_OPTION option, void* value);
			int GetOption(ENCODER_OPTION option, void* value);

			bool Encode(GenericImage img, FrameContainer& frame);
			bool Encode(unsigned char* i420, FrameContainer &frame);


			int ForceIntraFrame();
			void SetMaxBitrate(int target);
			void SetTargetFps(float target);
			int threadCount = 4;


	private:
		int buffer_size = 0;
		unsigned char* innerBuffer = nullptr;
		int innerBufLen = 0;

		ISVCEncoder* encoder= nullptr;
		SSourcePicture* pic = nullptr;
		SFrameBSInfo* bsi = nullptr;

		typedef int(* WelsCreateSVCEncoder)(ISVCEncoder** ppEncoder);
		WelsCreateSVCEncoder CreateEncoderFunc;
		typedef void(* WelsDestroySVCEncoder)(ISVCEncoder* ppEncoder);
		WelsDestroySVCEncoder DestroyEncoderFunc;

		void Create(const wchar_t* dllName);
		int InitializeInternal(int width, int height, int bps, float fps, ConfigType configType);
		void EnsureCapacity(int capacity);
		void GetEncodedFrames(const SFrameBSInfo& info, FrameContainer &fc);

	};
}
#endif
