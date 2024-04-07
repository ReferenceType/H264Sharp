#ifndef ENCODED_FRAME
#define ENCODED_FRAME
#include "pch.h"
namespace H264Sharp {
	enum class FrameType { Invalid, IDR, I, P, Skip, IPMixed };
	struct EncodedFrame
	{
			unsigned char* Data;
			int Length;
			int LayerNum;
			FrameType Type;
			byte uiTemporalId;
			byte uiSpatialId;
			byte uiQualityId;
			byte uiLayerType;
			int iSubSeqId;
			EncodedFrame(unsigned char* data, int lenght,int layerNum, const SFrameBSInfo& bsi);
			EncodedFrame();
	};

	 struct FrameContainer {
	 public:
		 EncodedFrame* Frames;
		 int Lenght;
	 };

	 struct Array {
	 public:

		 unsigned char* data;
		 int lenght;
	 };
	
}
#endif


