# H264Sharp
Cisco's OpenH264 Native wrapper for .Net with optimised color format conversion support. It is very suitable for realtime streaming over network.
This is the only open source .Net library with full feature wrapper, supported for windows and linux. 

SIMD color format converters are faster than OpenCV implementation.

- Cross Platform
- Plug&Play
- Tested on .NetFramework and Net(up to 8), Windows & Linux (x86 and Arm).
- Compatible with OpenCV.(i.e. OpenCVsharp)
- Tested on WPF application with camera and screen capture.
- No memory leaks or GC pressure.
- Simple console application example and WPF application is provided as an example.

Cisco Openh264 is chosen for its unbeatible performance compared to other available encoders. A paper involving performance metrics can be found here:
<br>https://iopscience.iop.org/article/10.1088/1757-899X/1172/1/012036/pdf</br>

Library consist of native dll which acts as OpenH264 wrapper/facade and color format converter (YUV420p <-> RGB,BGR,RGBA,BGRA)
<br/>Converters are vectorised(AVX2 or SSE for x86, Neon for Arm) and can be configured for parallelisation.

C# library is .Net standard wrapper library for this dll and performs PInvoke to handle transcoding.
## Nuget
Install the nuget package and its ready to go. All native dependencies are automatically installed and will apepear on your executable directory.
Binaries also provided on release section.

H264Sharp
<br>[![NuGet](https://img.shields.io/nuget/v/H264Sharp)](https://www.nuget.org/packages/H264Sharp)

H264SharpBitmapExtentions
<br>[![NuGet](https://img.shields.io/nuget/v/H264SharpBitmapExtentions)](https://www.nuget.org/packages/H264SharpBitmapExtentions)

For usage in Unity, You have to specify the absolute path for openh264 dll. (i.e. StreamingAssets)
``` c#
Defines.CiscoDllName64bit = "{YourPath}/openh264-2.4.0-win64.dll";
```

## Example
Examples can be found on examples directroy.

Following code shows encoder and decoder in action, commented lines are for hints.
``` c#

static void Main(string[] args)
{
    H264Encoder encoder = new H264Encoder();
    H264Decoder decoder = new H264Decoder();
    
    var img = System.Drawing.Image.FromFile("ocean1080.jpg");
    int w = img.Width; 
    int h = img.Height;
    var bmp = new Bitmap(img);

    encoder.Initialize(w, h, bps:10_000_000, fps:30, ConfigType.CameraBasic);
    decoder.Initialize();

    var data = BitmapToImageData(bmp);
    RgbImage rgb = new RgbImage(w, h);

    for (int j = 0; j < 10; j++)
    {
       
        encoder.Encode(data, out EncodedData[] ec)
       
        foreach (var encoded in ec)
        {
            bool keyframe = encoded.FrameType == FrameType.I || encoded.FrameType == FrameType.IDR;
            //encoded.GetBytes();
            //encoded.CopyTo(buffer,offset);

            if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgb))
            {
                Console.WriteLine($"F:{encoded.FrameType} size: {encoded.Length}");
               // Bitmap result = RgbToBitmap(rgb);
               // result.Save("Ok1.bmp");
            }
        }
    }
  
    encoder.Dispose();
    decoder.Dispose();
}
```
Bitmaps are not included on library to keep it cross platform.
An extention library is provided for windows.
<br/>For the bitmaps and other image container types, an extention library is provided.
``` c#
private static Bitmap RgbToBitmap(RgbImage img)
{
    Bitmap bmp = new Bitmap(img.Width,
                            img.Height,
                            img.Width * 3,
                            PixelFormat.Format24bppRgb,
                            img.ImageBytes);
    return bmp;
}
```
And to extract bitmap data:
```c#
/*
 * Pixel data is ARGB, 1 byte for alpha, 1 for red, 1 for green, 1 for blue. 
 * Alpha is the most significant byte, blue is the least significant.
 * On a little-endian machine, like yours and many others,
 * the little end is stored first, so the byte order is b g r a.
 */
    private static ImageData BitmapToImageData(Bitmap bmp)
    {
        int width = bmp.Width;
        int height = bmp.Height;
        BitmapData bmpData = bmp.LockBits(new Rectangle(0, 0, width, height),
                                          ImageLockMode.ReadOnly,
                                          PixelFormat.Format32bppArgb);
        var bmpScan = bmpData.Scan0;

        //PixelFormat.Format32bppArgb is default
        ImageType type = ImageType.Rgb;
        switch (bmp.PixelFormat)
        {
            case PixelFormat.Format32bppArgb:
                type = ImageType.Bgra; //endianness
                break;
            case PixelFormat.Format32bppRgb:
                type = ImageType.Bgra;
                break;
            case PixelFormat.Format24bppRgb:
                type = ImageType.Bgr;
                break;
            default:
                throw new NotSupportedException($"Format {bmp.PixelFormat} is not supported");

        }

        var img = new H264Sharp.ImageData(type, width, height, bmpData.Stride, bmpScan);

        bmp.UnlockBits(bmpData);
        return img;
    }
}
```
# Info & Tips

Decoder has two ways of decoding:

- First is with the ``` out RGBImagePointer rgb``` which is a ref struct on purpose. On unmanaged side, It writes the decoded bytes into Cached array, and it reuses same array in consecutive decodes, so what you get is only a reference to that memory. So not intended for storage(hence the ref struct), but handling decoded image ephemerally.

- Second is with ``` ref RgbImage img``` which receives the memory externally from managed side and directly decodes there without copy. When you create new instance of RgbImage it allocates the raw image array(Unmanaged, to be interopped). This object is storable, and reusable. If you are going to use this you should either reuse same reference or pool these objects(i.e. ConcurrentBag). One should avoid making new on each decode.

Yuv<->Rgb converter does not allocate any memory its either uses the memory provided by managed side, or the cached array in the unmanaged side depending on which method is called, as I explained above.

Similarly Encoder also gives a reference object to unmanaged cached memory which is again cycled through each encode operation.
It doesn't allocate any memory unless you call ``` .GetBytes()``` method of the Encoded data. Here you should use ``` .CopyTo(buffer,offset) ``` if you can.

- A tip, Encoded data which consists of arrays of byte arrays can be stitched(copy to) into single contiguous array and fed into decoder as single input, If you are only using single layer(standard use case).
 
# Advanced Configuration & Features
## Advanced Setup
If you want to initialise your encoder and able to control everything, you can use provided API which is identical to Ciso C++ Release.
```c#
 encoder = new H264Encoder();
 var param = encoder.GetDefaultParameters();

 param.iUsageType = EUsageType.CAMERA_VIDEO_REAL_TIME;
 param.iPicWidth = w; 
 param.iPicHeight = h;
 param.iTargetBitrate = 1000000;
 param.iTemporalLayerNum = 1;
 param.iSpatialLayerNum = 1;
 param.iRCMode = RC_MODES.RC_BITRATE_MODE;

 param.sSpatialLayers[0].iVideoWidth = 0;
 param.sSpatialLayers[0].iVideoWidth = 0;
 param.sSpatialLayers[0].fFrameRate = 60;
 param.sSpatialLayers[0].iSpatialBitrate = 1000000;
 param.sSpatialLayers[0].uiProfileIdc = EProfileIdc.PRO_HIGH;
 param.sSpatialLayers[0].uiLevelIdc = 0;
 param.sSpatialLayers[0].iDLayerQp = 0;


 param.iComplexityMode = ECOMPLEXITY_MODE.HIGH_COMPLEXITY;
 param.uiIntraPeriod = 300;
 param.iNumRefFrame = 0;
 param.eSpsPpsIdStrategy = EParameterSetStrategy.SPS_LISTING_AND_PPS_INCREASING;
 param.bPrefixNalAddingCtrl = false;
 param.bEnableSSEI = true;
 param.bSimulcastAVC = false;
 param.iPaddingFlag = 0;
 param.iEntropyCodingModeFlag = 1;
 param.bEnableFrameSkip = false;
 param.iMaxBitrate =0;
 param.iMinQp = 0;
 param.iMaxQp = 51;
 param.uiMaxNalSize = 0;
 param.bEnableLongTermReference = true;
 param.iLTRRefNum = 1;
 param.iLtrMarkPeriod = 180;
 param.iMultipleThreadIdc = 1;
 param.bUseLoadBalancing = true;
 
 param.bEnableDenoise = false;
 param.bEnableBackgroundDetection = true;
 param.bEnableAdaptiveQuant = true;
 param.bEnableSceneChangeDetect = true;
 param.bIsLosslessLink = false;
 param.bFixRCOverShoot = true;
 param.iIdrBitrateRatio = 400;
 param.fMaxFrameRate = 30;

 encoder.Initialize(param);
```

Similarly for decoder
```c#
    decoder = new H264Decoder();
    TagSVCDecodingParam decParam = new TagSVCDecodingParam();
    decParam.uiTargetDqLayer = 0xff;
    decParam.eEcActiveIdc = ERROR_CON_IDC.ERROR_CON_FRAME_COPY_CROSS_IDR;
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_TYPE.VIDEO_BITSTREAM_SVC;
    decoder.Initialize(decParam);
```


Color format conversion (RGB <-> YUV420) has optional configuration where you can provide number of threads on parallelisation.
<br/>Using 1 thread consumes least cpu cycles(minimum context switch) and most efficient but it takes more time. 
<br/>Beyond 4 threads you start to get diminishing returns on practical setups. Default count is 4.
<br/>Behaviour depends on image size, your system memory speed, core IPC, cache and many other factors so your milage can vary.

You can configure on RGB to YUV conversion SIMD support. For example, if AVX2 is enabled it wont run SSE version
Neon is only active on arm and does nothing on x86 systems.

```c#
var config = ConverterConfig.Default;
config.EnableSSE = 1;
config.EnableNeon = 1;
config.EnableAvx2 = 1;
config.NumThreads = 4;
config.EnableCustomThreadPool = 0;
Converter.SetConfig(config);
```

## Options
You can get and set options to decoder and encoder on runtime. All options API is implemented 1-1 with cisco api.

```c#
    encoder.SetOption(ENCODER_OPTION.ENCODER_OPTION_IDR_INTERVAL, 600);
    encoder.GetOption(ENCODER_OPTION.ENCODER_OPTION_IDR_INTERVAL, out int idrPeriod);
    decoder.GetOption(DECODER_OPTION.DECODER_OPTION_FRAME_NUM, out tempInt);
        ...
```

There are many possible options and they are commented on the enum fields as well as required types. If you want more detail, search as general H264 options.
<br/>Because you wont find any documentation on cisco side RTFC(Read the F. code) pinciple.

If you want to reuse your option structs for efficiency, you can use this method:
```c#
 SEncoderStatistics ss;
 SDecoderStatistics ss1;
 encoder.GetOptionRef(ENCODER_OPTION.ENCODER_OPTION_GET_STATISTICS, ref ss);
 decoder.GetOptionRef(DECODER_OPTION.DECODER_OPTION_GET_STATISTICS, ref ss1);
```
# Example App
A simple example WPF application(quick & dirty) is provided. This app emulates advanced use cases for the lossy transfers. 
here you can explore:
- Advanced Setup and their effects.
- Using LTR references and loss recovery.
- Recording audio and video.
<img src="https://github.com/ReferenceType/H264Sharp/assets/109621184/e530be0b-30df-4937-b5e5-6a5e970c81ba" width=50% height=50%>



