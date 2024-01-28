#include "pch.h"


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

H264Sharp::EncodedFrame::EncodedFrame(unsigned char* data, int lenght, int layerNum, FrameType frameType)
{
	Data = data;
	Length = lenght;
	LayerNum = layerNum;
	Type = frameType;
}
H264Sharp::EncodedFrame::EncodedFrame()
{
	
}


