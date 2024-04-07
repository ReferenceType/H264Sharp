#ifndef IMAGE_TYPES
#define IMAGE_TYPES
#include "EncodedFrame.h"
namespace H264Sharp {

    enum class ImageType{Rgb,Bgr,Rgba,Bgra};

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

    struct RgbImage
    {
    public:
        int Width;
        int Height;
        int Stride;
        unsigned char* ImageBytes;
        RgbImage(unsigned char* imageBytes, int width, int height, int stride);

    };
   
    typedef struct YuvNative {
        unsigned char* Y;
        unsigned char* U;
        unsigned char* V;
        int width;
        int height;
        int stride;
        int stride2;
    };
    class Yuv420p
    {
    public:
        unsigned char* Y;
        unsigned char* U;
        unsigned char* V;
        int width;
        int height;
        int strideY;
        int strideU;
        int strideV;
        Yuv420p(unsigned char* Y, unsigned char* U, unsigned char* V, int width, int height, int strideY, int strideuu, int strideV);

    };
    
}
#endif
