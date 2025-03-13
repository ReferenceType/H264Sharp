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
			uint8_t uiTemporalId;
			uint8_t uiSpatialId;
			uint8_t uiQualityId;
			uint8_t uiLayerType;
			int iSubSeqId;
			EncodedFrame(unsigned char* data, int lenght,int layerNum, const SFrameBSInfo& bsi)
			{
				Data = data;
				Length = lenght;
				LayerNum = layerNum;
				Type = (FrameType)bsi.eFrameType;
				uiTemporalId = bsi.sLayerInfo[layerNum].uiTemporalId;
				uiSpatialId = bsi.sLayerInfo[layerNum].uiSpatialId;
				uiQualityId = bsi.sLayerInfo[layerNum].uiQualityId;
				uiLayerType = bsi.sLayerInfo[layerNum].uiLayerType;
				iSubSeqId = bsi.sLayerInfo[layerNum].iSubSeqId;
			}
			EncodedFrame() {}
	};
	
	 struct FrameContainer {
	 public:
		 EncodedFrame* Frames;
		 int Lenght;
	 };
	
}
#endif


