#include "ImageTypes.h"


H264Sharp::RgbImage::RgbImage(unsigned char* imageBytes, int width, int height, int stride)
{
	ImageBytes = imageBytes;
	Width = width;
	Height = height;
	Stride = stride;
}




H264Sharp::EncodedFrame::EncodedFrame(unsigned char* data, int lenght, int layerNum, const SFrameBSInfo& bsi)
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
H264Sharp::EncodedFrame::EncodedFrame()
{
	
}


