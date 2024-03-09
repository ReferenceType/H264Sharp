#pragma once
#include "pch.h"
namespace H264Sharp {
	
	public ref class EncodedFrame
	{
		public:
			unsigned char* Data;
			IntPtr DataPointer;
			int Length;
			int LayerNum;
			FrameType Type;
			EncodedFrame(unsigned char* data, int lenght,int layerNum, FrameType frameType);
			array<Byte>^ ToByteArray();
			void CopyTo(array<Byte>^ destination, int startIndex);
	};
	
}


