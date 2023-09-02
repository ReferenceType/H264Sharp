#include "pch.h"
using namespace System;

H264Sharp::BgrImage::BgrImage(unsigned char* imageBytes, int width, int height, int stride)
{
	ImageBytes = imageBytes;
	Width = width;
	Height = height;
	Stride = stride;

}

H264Sharp::BgrImage::BgrImage(array<byte>^ sourceArray, int startIndex, int width, int height, int stride)
{
	pin_ptr<byte> ptr = &sourceArray[startIndex];
	ImageBytes = ptr;
	Width = width;
	Height = height;
	Stride = stride;
}

H264Sharp::RgbImage::RgbImage(unsigned char* imageBytes, int width, int height, int stride)
{
	ImageBytes = imageBytes;
	Width = width;
	Height = height;
	Stride = stride;

}

H264Sharp::RgbImage::RgbImage(array<byte>^ sourceArray, int startIndex, int width, int height, int stride)
{
	pin_ptr<byte> ptr = &sourceArray[startIndex];
	ImageBytes = ptr;
	Width = width;
	Height = height;
	Stride = stride;
}

H264Sharp::BgraImage::BgraImage(unsigned char* imageBytes, int width, int height, int stride)
{
	ImageBytes = imageBytes;
	Width = width;
	Height = height;
	Stride = stride;
}
H264Sharp::BgraImage::BgraImage(array<byte>^ sourceArray, int startIndex, int width, int height, int stride)
{
	pin_ptr<byte> ptr = &sourceArray[startIndex];
	ImageBytes = ptr;
	Width = width;
	Height = height;
	Stride = stride;
}
H264Sharp::RgbaImage::RgbaImage(unsigned char* imageBytes, int width, int height, int stride)
{
	ImageBytes = imageBytes;
	Width = width;
	Height = height;
	Stride = stride;

}
H264Sharp::RgbaImage::RgbaImage(array<byte>^ sourceArray, int startIndex, int width, int height, int stride)
{
	pin_ptr<byte> ptr = &sourceArray[startIndex];
	ImageBytes = ptr;
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
	DataPointer = data;
	Data = (System::IntPtr)data;
	Length = lenght;
	LayerNum = layerNum;
	Type = frameType;
}
array<System::Byte>^ H264Sharp::EncodedFrame::ToByteArray()
{
	array<System::Byte>^ data = gcnew array<Byte>(Length);
	System::Runtime::InteropServices::Marshal::Copy(Data, data, 0, Length);
	return data;
}

void H264Sharp::EncodedFrame::CopyTo(array<Byte>^ destination, int startIndex)
{
	pin_ptr<Byte> ptr = &destination[startIndex];
	memcpy(ptr, DataPointer, Length);
	ptr = nullptr;
}