#pragma once
#include "pch.h"
namespace H264Sharp {
	
	struct EncodedFrame
	{
			unsigned char* Data;
			int Length;
			int LayerNum;
			FrameType Type;
			EncodedFrame(unsigned char* data, int lenght,int layerNum, FrameType frameType);
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


