#ifndef IMAGE_TYPES
#define IMAGE_TYPES
#include "EncodedFrame.h"

namespace H264Sharp {

    enum class ImageType{Rgb,Bgr,Rgba,Bgra};
    enum class YUVType{YV12,NV12};

    struct GenericImage {
    public:
        ImageType Type;
        int Width;
        int Height;
        int Stride;
        unsigned char* ImageBytes;
        GenericImage(unsigned char* imageBytes, int width, int height, int stride, ImageType type) 
        {
            ImageBytes = imageBytes;
            Width = width;
            Height = height;
            Stride = stride;
        }
    };

    struct RgbImage{
    public:
        int Width;
        int Height;
        int Stride;
        unsigned char* ImageBytes;
        RgbImage(unsigned char* imageBytes, int width, int height, int stride);

    };
   
    struct YuvNative {
    public:
        unsigned char* Y = nullptr;
        unsigned char* U = nullptr;
        unsigned char* V = nullptr;
        int width =0;
        int height=0;
        int yStride=0;
        int uvStride=0;
        YUVType format = YUVType::YV12;
    };
   
    
}
#endif
