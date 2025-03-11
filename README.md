# H264Sharp
Cisco's OpenH264 Native facade/wrapper for .Net with highly optimised SIMD color format conversion support. It is very suitable for realtime streaming over network. 

SIMD color format converters are up to 2.9x faster than OpenCV implementation.

- Cross Platform
- Plug&Play
- Tested on .NetFramework and Net(up to 8), Windows & Linux & Android (x86 and Arm).
- No memory leaks or GC pressure.
- Compatible with OpenCV.(i.e. OpenCVsharp)

Cisco Openh264 is chosen for its unbeatible performance compared to other available software encoders. A paper involving performance metrics can be found here:
<br>https://iopscience.iop.org/article/10.1088/1757-899X/1172/1/012036/pdf</br>

Library consist of native dll which acts as OpenH264 wrapper/facade and color format converter (YUV <-> RGB,BGR,RGBA,BGRA)
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

    for (int j = 0; j < 1; j++)
    {

        if (!encoder.Encode(rgbIn, out EncodedData[] ec))
        {
            Console.WriteLine("skipped");
            continue;
        }

        /* You can manupulate encoder settings on runtime. */
        //encoder.ForceIntraFrame();
        //encoder.SetMaxBitrate(2000000);
        //encoder.SetTargetFps(16.9f);

        foreach (var encoded in ec)
        {
            bool keyframe = encoded.FrameType == FrameType.I
                            || encoded.FrameType == FrameType.IDR;

            /* You can extract the bytes */
            //encoded.GetBytes();
            //encoded.CopyTo(buffer,offset);


            if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgbOut))
            {
                //Console.WriteLine($"F:{encoded.FrameType} size: {encoded.Length}");
                //var result = rgbOut.ToBitmap();
                //result.Save("OUT2.bmp");

            }

        }
    }   
}

 encoder.Dispose();
 decoder.Dispose();
```
Bitmaps are not included on library to keep it cross platform.
An extention library is provided for windows.
<br/>For the bitmaps and other image container types, an extention library is provided.
``` c#
 RgbImage rgb = new RgbImage(H264Sharp.ImageFormat.Rgb, w, h);
 Bitmap bmp = rgb.ToBitmap();
```
And to extract bitmap data:
```c#
 Bitmap bitmap;// some bitmap
 RgbImage rgb = bitmap.ToRgbImage();
```
# Info & Tips
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
Decoder can work with both frame by frame or merged.  I personally merge the encoded frames into single array and decode them on single shot.

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
### Tips
- Raw image bytes are large, avoid allocating new ones and try to reuse same RgbImage or YuvImage or pool them in something like concurrent bag.

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

## Converter

Color format conversion (RGB <-> YUV) has optional configuration where you can provide number of threads on parallelisation.
<br/>Using 1 thread consumes least cpu cycles(minimum context switch) and most efficient but it may be slower. 
<br/>Performance depends on image size, your system memory speed, core IPC, cache and many other factors, so your milage may vary.

You can configure on RGB to YUV conversion SIMD support. By default highest supported instruction set will be selected on runtime 
For example, if AVX2 is enabled it wont run SSE version
Neon is only active on arm and does nothing on x86 systems.

```c#
var config = Converter.GetCurrentConfig();
config.EnableSSE = 1;
config.EnableNeon = 1;
config.EnableAvx2 = 1;
config.NumThreads = 4;
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



