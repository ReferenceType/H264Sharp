# H264Sharp

Cisco's OpenH264 Native facade/wrapper for .Net with highly optimised SIMD color format conversion support. It is very suitable for realtime streaming over network. 
This library is designed to give access to full control on Encoding and Decoding process instead of a blackbox approach, yet offers simplified API for quick setup and go.

SIMD color format converters are up to 2.9x faster than OpenCV implementation.

- Cross Platform
- Plug&Play
- Tested on .NetFramework and Net(up to 8), Windows & Linux & Android (x86 and Arm).
- No memory leaks or GC pressure.
- Compatible with WPF and OpenCV.(i.e. OpenCVsharp)

Cisco Openh264 is chosen for its unbeatible performance compared to other available software encoders. A paper involving performance metrics can be found here:
<br>https://iopscience.iop.org/article/10.1088/1757-899X/1172/1/012036/pdf</br>


Library consist of native dll which acts as OpenH264 wrapper/facade and color format converter (YUV <-> RGB,BGR,RGBA,BGRA)<br/>
C# library is .Net standard wrapper library for this dll and performs PInvoke to handle transcoding.


## NuGet


Install the NuGet package. All native dependencies should be automatically installed and resolved(from version v1.6.0+).
- Tested on Windows(x86,x64), Linux(x86,x64,arm32,arm64), Android MAUI app(x64 on emulator, arm on Pixel phone).

Binaries also provided on [Relases](https://github.com/ReferenceType/H264Sharp/releases).

H264Sharp
<br>[![NuGet](https://img.shields.io/nuget/v/H264Sharp)](https://www.nuget.org/packages/H264Sharp)


H264SharpBitmapExtentions
<br>[![NuGet](https://img.shields.io/nuget/v/H264SharpBitmapExtentions)](https://www.nuget.org/packages/H264SharpBitmapExtentions)

For usage in Unity, manually place the dlls from releases. H264SharpNative will be resolved automatically. You have to specify the absolute path for openh264 dll(Cisco). (i.e. StreamingAssets)
``` c#
Defines.CiscoDllName64bit = "{YourPath}/openh264-2.4.0-win64.dll";
```
<ins>**[Help Wanted]: Only MacOS and IOS binaries are left to complete this project. I dont own a Mac or an Iphone, and I want to test before shipping instead of just compiling.
If you own this devices, it would be greatly appreciated to contribute this binaries.**<inst/>

## Example
Examples codes can be found on examples directory on the repository.<br/>
For detailed information and documentation please check out [Wiki](https://github.com/ReferenceType/H264Sharp/wiki) page

Following code shows minimalist example of encoder and decoder in action.
``` csharp
static void Main(string[] args)
{
    var img = System.Drawing.Image.FromFile("ocean 1920x1080.jpg");
    int w = img.Width;
    int h = img.Height;
    var bitmap = new Bitmap(img);

    H264Encoder encoder = new H264Encoder();
    H264Decoder decoder = new H264Decoder();

    decoder.Initialize();
    encoder.Initialize(w, h, 200_000_000, 30, ConfigType.CameraCaptureAdvanced);

    RgbImage rgbIn = bitmap.ToRgbImage();
    RgbImage rgbOut = new RgbImage(H264Sharp.ImageFormat.Rgb, w, h);

    for (int j = 0; j < 100; j++)
    {
        // Encode
        if (!encoder.Encode(rgbIn, out var encodedFrames))
            continue;//skipped

        // Decode
        foreach (var encoded in encodedFrames)
        {
            if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgbOut))
            {
               // Process rgbOut
            }
        }
    }
}
```
Bitmaps are not included on library to keep it cross platform.

For the bitmaps and other image container types, an extention library is provided.

``` c#
 RgbImage rgb = new RgbImage(H264Sharp.ImageFormat.Rgb, w, h);
 Bitmap bmp = rgb.ToBitmap();
```
And to extract bitmap data:
```c#
 Bitmap bitmap;// some bitmap
 RgbImage rgb = bitmap.ToRgbImage();
```
## Quick Tips
For detailed information and documentation please check out [Wiki](https://github.com/ReferenceType/H264Sharp/wiki) page
### Data
Data classes can use existing memory or allocate one for you
```c#
// Will allocate native memory and manage disposal
public YuvImage(int width, int height)
public RgbImage(ImageFormat format, int width, int height)

// Will refer to existing memory
public YuvImage(IntPtr data, int width, int height);
public RgbImage(ImageFormat format, int width, int height, IntPtr imageBytes)
public RgbImage(ImageFormat format, int width, int height, byte[] data)
```
### Encoder
Encoder has following Encode API:

```c#
public bool Encode(RgbImage im, out EncodedData[] ed)
public bool Encode(YuvImage yuv, out EncodedData[] ed)

public bool Encode(YUVImagePointer yuv, out EncodedData[] ed)
public bool Encode(YUVNV12ImagePointer yuv, out EncodedData[] ed)
```
Where
 - ```YuvImage``` and ```RgbImage``` may or may not own the memory depending on how its constructed.<br/>
 - ```YUVImagePointer``` and ```YUVNV12ImagePointer``` are ref structs and only points to an existing memory.

```EncodedData[]``` are the data frames of the encoder and refers to a ephemeral native memory and will be overwrtitten on next encode.
You can get the native frames one by one, or all merged, into contigious managed memory:
 ```c#

..out EncodedData[] ec

// You can extract the bytes one by one.
byte[] encodedbytes = ec[0].GetBytes();
ec[0].CopyTo(buffer,offset);

// or merge into single array
byte[] encodedbytes = ec.GetAllBytes();
ec.CopyAllTo(buffer,offset);

```
On single layer(standard use case) for IDR frames you get more than one EncodedData, the first frame is a metadata and it will produce neither an image nor an error when decoded.<br/>
Decoder can work with both frame by frame or merged.

### Decoder
Decoder has the API:
```c#
 public bool Decode(byte[] encoded, int offset, int count, bool noDelay, out DecodingState state, out YUVImagePointer yuv)
 public bool Decode(byte[] encoded, int offset, int count, bool noDelay, out DecodingState state, ref YuvImage yuv)
 public bool Decode(byte[] encoded, int offset, int count, bool noDelay, out DecodingState state, ref RgbImage img)
```
Cisco decoder only supports YUV I420 Planar output

API Where:<br/>
 - ```YUVImagePointer``` directly refers to decoder's native buffer, Cisco adds 64 byte padding.<br/>
 - ```YuvImage``` Copies the native YUV image into provided container.<br/>
 - ```RgbImage``` Output will convert native YUV into RGB container(RGB,BGR,RGBA or BGRA) provided. <br/>

If methods return false means there is no image and you need to check DecodingState. 
Otherwise there is an image but, on lossy link you still need to check DecodingState for error and perform necessary action(i.e. perform IDR refresh request to encoder).
#### Tips
- Raw image bytes are large, avoid allocating new ones and try to reuse same RgbImage or YuvImage or pool them in something like concurrent bag.

For more information refer to [Tutorial](https://github.com/ReferenceType/H264Sharp/wiki/Tutorial)
## Advanced Configuration & Features
### Advanced Setup
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

## Converter
Significant development effort has been spent here to deliver best possible performance. It was impactful to improve format conversions as much as possible especially for mobile platforms.

Color format conversion (RGB <-> YUV) has optional configuration where you can provide number of threads on parallelisation.
<br/>Using 1 thread consumes least cpu cycles(minimum context switch) and most efficient but it may be slower. 
<br/>Performance of parallel proccesing depends on image size, memory speed, core IPC, cache size and many other factors, so your milage may vary.

You can configure on RGB to YUV conversion SIMD support. By default highest supported instruction set will be selected on runtime 
For example, if AVX2 is enabled it wont run SSE version
Neon is only active on arm and does nothing on x86 systems.

```c#
var config = Converter.GetCurrentConfig();

config.EnableSSE = 1;
config.EnableNeon = 1;
config.EnableAvx2 = 1;
config.NumThreads = 4;// default is 1

config.EnableCustomThreadPool = 1;
Converter.SetConfig(config);

// Or like..
Converter.SetOption(ConverterOption.NumThreads, 8);

```
#### Converter Bechmarks
H264Sharp conversion operations are up to 2.9x faster than OpenCV implementations.

1080p 5000 Iterations of RGB -> YUV and YUV -> RGB, CustomThreadPool
AMD Ryzen 7 3700X Desktop CPU 
| #Threads  | OpenCV <sub>(ms)</sub> | H264Sharp <sub>(ms)</sub> |
|---|---|---|
|1|11919 |4899|
|2|6205 |2479|
|4|3807 |1303|
|8|2543 |822|
|16|2462|824 |

Intel i7 10600U Laptop CPU 
1080p 5000 Iterations of RGB -> YUV and YUV -> RGB, CustomThreadPool
| #Threads  | OpenCV <sub>(ms)</sub> | H264Sharp <sub>(ms)</sub> |
|---|---|---|
|1|11719 |6010|
|2|6600 |3210|
|4|4304 |2803|
|8|3560 |1839|

## Options
You can get and set options to decoder and encoder on runtime. All options API is implemented 1-1 with cisco api.

```csharp
    encoder.SetOption(ENCODER_OPTION.ENCODER_OPTION_IDR_INTERVAL, 600);
    encoder.GetOption(ENCODER_OPTION.ENCODER_OPTION_IDR_INTERVAL, out int idrPeriod);
    decoder.GetOption(DECODER_OPTION.DECODER_OPTION_FRAME_NUM, out tempInt);

    // If you want to reuse your option structs for efficiency:
    SEncoderStatistics ss;
    SDecoderStatistics ss1;
    encoder.GetOptionRef(ENCODER_OPTION.ENCODER_OPTION_GET_STATISTICS, ref ss);
    decoder.GetOptionRef(DECODER_OPTION.DECODER_OPTION_GET_STATISTICS, ref ss1);
```

There are many possible options and they are commented on the enum fields as well as required types. If you want more detail, search as general H264 options.
<br/>Because you wont find any documentation on cisco side.


# Example App

A simple example WPF application is provided. This app emulates advanced use cases for the lossy transfers(loss&jitter) leveraging LTR references. 

here you can explore:
- Advanced Setup and their effects.
- Using LTR references and loss recovery.
<img src="https://github.com/ReferenceType/H264Sharp/assets/109621184/e530be0b-30df-4937-b5e5-6a5e970c81ba" width=50% height=50%>



