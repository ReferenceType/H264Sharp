#include "ImageTypes.h"


H264Sharp::RgbImage::RgbImage(unsigned char* imageBytes, int width, int height, int stride)
{
	ImageBytes = imageBytes;
	Width = width;
	Height = height;
	Stride = stride;
}


H264Sharp::Yuv420p::Yuv420p(unsigned char* YY, unsigned char* UU, unsigned char* VV, int width_, int height_, int stride_Y, int stride_U, int stride_V)
{
	Y = YY;
	U = UU;
	V = VV;
	width = width_;
	height = height_;
	strideY = stride_Y;
	strideU = stride_U;
	strideV = stride_V;
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


