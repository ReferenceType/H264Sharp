#pragma once
#include "pch.h"
#include <string>


namespace H264Sharp {

	
	enum class ConfigType { CameraBasic, ScreenCaptureBasic, CameraCaptureAdvanced, ScreenCaptureAdvanced };

	class Encoder
	{
		public:
			Encoder(const wchar_t* dllname);
			Encoder();
			~Encoder();

			int Initialize(int width, int height, int bps, float fps, ConfigType configNo);

			bool Encode(GenericImage img, FrameContainer& frame);
			bool Encode(unsigned char* i420, FrameContainer &frame);

			int ForceIntraFrame();
			void SetMaxBitrate(int target);
			void SetTargetFps(float target);

	private:
		int buffer_size = 0;
		unsigned char* innerBuffer = nullptr;
		int innerBufLen = 0;

		ISVCEncoder* encoder= nullptr;
		SSourcePicture* pic = nullptr;
		SFrameBSInfo* bsi = nullptr;

		typedef int(__cdecl* WelsCreateSVCEncoder)(ISVCEncoder** ppEncoder);
		WelsCreateSVCEncoder CreateEncoderFunc;
		typedef void(__cdecl* WelsDestroySVCEncoder)(ISVCEncoder* ppEncoder);
		WelsDestroySVCEncoder DestroyEncoderFunc;

		void Create(const wchar_t* dllName);
		int InitializeInternal(int width, int height, int bps, float fps, ConfigType configType);
		void EnsureCapacity(int capacity);
		void GetEncodedFrames(const SFrameBSInfo& info, FrameContainer &fc);

	};
}
