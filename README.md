# H264Sharp
Cisco's OpenH264 Native wrapper for .Net with optimised image format conversion support. It is very suitable for realtime streaming over network.
This is the only open source C# library with full feature wrapper. 
Image format converters are faster than OpenCV implementation.
- Plug&Play
- Tested on .NetFramework and Net(up to 8).
- Compatible with OpenCV.(i.e. OpenCVsharp)
- Tested on WPF application with camera and screen capture.
- No memory leaks or GC pressure.
- Simple console application example and WPF application is provided as an example.

Library consist of native dll which acts as OpenH264 wrapper and image format converter (YUV420p <-> RGB,BGR,RGBA,BGRA)
<br/>Converters are vectorised(AVX2 and SSE) and can be configured for parallelisation for high performance.

C# library is .Net standard wrapper library for this dll and performs PInvoke to handle transcoding.
## Nuget
Install the nuget package and its ready to go. All native dependencies are automatically installed and will apepear on your executable directory.

[![NuGet](https://img.shields.io/nuget/v/H264Sharp)](https://www.nuget.org/packages/H264Sharp/1.2.0)

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
    RgbImage rgbb = new RgbImage(w, h);

    for (int j = 0; j < 10; j++)
    {
       
        encoder.Encode(data, out EncodedData[] ec)
       
        foreach (var encoded in ec)
        {
            bool keyframe = encoded.FrameType == FrameType.I || encoded.FrameType == FrameType.IDR;
            //encoded.GetBytes();
            //encoded.CopyTo(buffer,offset);

            if (decoder.Decode(encoded, noDelay: true, out DecodingState ds, ref rgbb))
            {
                Console.WriteLine($"F:{encoded.FrameType} size: {encoded.Length}");
               // Bitmap result = RgbToBitmap(rgbb);
               // result.Save("Ok1.bmp");
            }
        }
    }
  
    encoder.Dispose();
    decoder.Dispose();
}
```
Bitmaps are not included on library to keep it cross platform.
<br/>For the bitmaps and other image container types i will provide extention libraries.
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

# Advanced Configuration & Features
## Advanced Setup
If you want to initialise your encoder and able to control everything, you can use provided API which is identical to Ciso C++.
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
    decParam.eEcActiveIdc = ERROR_CON_IDC.ERROR_CON_SLICE_MV_COPY_CROSS_IDR;
    decParam.eEcActiveIdc = ERROR_CON_IDC.ERROR_CON_FRAME_COPY_CROSS_IDR;
    decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_TYPE.VIDEO_BITSTREAM_SVC;
    decoder.Initialize(decParam);
```

Image format conversion (RGB <-> YUV420) has optional configuration where you can provide number of threads on parallelisation.
<br/>Using 1 thread gives consumes least cpu cycles and most efficient but it takes more time. 
<br/>Beyond 4 threads you start to get diminishing returns.
<br/>Fastest performance is achieved when threadcount is same as your phyical threads on your machine.
<br/>Larger the image more effective is the parallelisation.
<br/>Default count is 4.
```c#
    encoder.ConverterNumberOfThreads = Environment.ProcessorCount;
    decoder.ConverterNumberOfThreads = 4;
```

You can configure on RGB to YUV conversion wether to use SSE or table based converter. SSE is faster and is default configuration.

```c#
    decoder.EnableSSEYUVConversion= true;
```

## Options
You can get and set options to decoder and encoder on runtime. All options API is implemented 1-1 with cisco api.

```c#
      encoder.SetOption(ENCODER_OPTION.ENCODER_OPTION_IDR_INTERVAL, 600);
      encoder.GetOption(ENCODER_OPTION.ENCODER_OPTION_IDR_INTERVAL, out int idrPeriod);
      decoder.GetOption(DECODER_OPTION.DECODER_OPTION_FRAME_NUM, out tempInt);
        ...
```

There are many possible options and they are commented on the enum fields as well as required types. If you want more detail search as general H264 options ad params not cisco spesifically.
<br/>Because you wont find any documentation on cisco side.

If you want to reuse your option structs for efficiency, you can use this method:
```c#
 SEncoderStatistics ss;
 SDecoderStatistics ss1;
 encoder.GetOptionRef(ENCODER_OPTION.ENCODER_OPTION_GET_STATISTICS, ref ss);
 decoder.GetOptionRef(DECODER_OPTION.DECODER_OPTION_GET_STATISTICS, ref ss1);
```
# Example App
A simple example WPF application is provided. here you can explore:
- Advanced Setup and their effects.
- Using LTR references and loss recovery.
- Recording audio and video.
<img src="https://github.com/ReferenceType/OpenH264Wrapper/assets/109621184/37cf09ee-7599-41ae-a062-22c480074ac4" width=50% height=50%>






# Legacy C++/CLI(deprecated)
### C++Cli wrapper is deprecated due to platform limitations and other issues. Plase use native Pinvoke version which is also distrbuted with Nuget.
Cisco's OpenH264 C++/CLI wrapper with optimised image format conversion support. It is very suitable for realtime streaming over network.
- Offers managed and unmanaged API.
- Tested on .NetFramework and NetCore(up to 8).
- Compatible with OpenCV.(i.e. OpenCVsharp)
- Tested on WPF application with camera and screen capture (P2P Videocall).
- No memory leaks or GC pressure with bitmaps.
- Simple console application example is provided as an example.
  
### Setup(deprecated)
- Default Constructor will look for `openh264-2.3.1-win32.dll` or `openh264-2.3.1-win64.dll` automatically on executable directory depending on process type(64/32 bit).
- You can setup with a different dll name, constructor is overloaded.
``` c#
  decoder = new H264Sharp.Decoder();
  
  encoder = new H264Sharp.Encoder();
  encoder.Initialize(width,
                     height,
                     bps: 3_000_000,
                     fps: 30,
                     H264Sharp.Encoder.ConfigType.CameraBasic);
```

### Encode (deprecated)
- You can encode from rgb/rgba/bgr/bgra/yuv_i420 on as raw data format, or System.Drawing.Bitmaps.
- Raw data is compatible with OpenCV Mats or any other standard image container.
- EncodedFrame represents h264 encoded bytes(NALs etc).
```C#
  if(encoder.Encode(bitmap, out EncodedFrame[] frames))
  {
      foreach (var frame in frames)
      {
          //You can convert to managed array
          //byte[] b = frame.ToByteArray();

          // You can copy to Managed array
          //frame.CopyTo(buffer, 0);

          Decode(frame.Data, frame.Length, frame.Type);
      }
  }
```

  
### Decode(deprecated)
- You can decode with pointers or with managed byte array as input.
- You can decode into System.Drawing.Bitmaps or raw data format images (they are compatible with OpenCV Mats and any other standard image containers.).
```C#
  void Decode(IntPtr data, int length, FrameType type)
  {
      if (decoder.Decode(data, length, noDelay:true, out DecodingState statusCode, out Bitmap bmp)) 
      {
          // Do stuff..
          // bmp.Save("t.bmp");
      }
  }
// You can use other formats as:
 decoder.Decode(data, length, noDelay:true, out DecodingState statusCode, out RgbImage rgb)
 decoder.Decode(data, length, noDelay:true, out DecodingState statusCode, out Yuv420p yuv420)
 ...
```

## Converter dll(deprecated)
A separate dll is provided for RGB <-> YUV conversions. Its compiled with clang LLVM with AVX2 intrinsics.
</br>You can optionally include it on your executable path just like Openh264 dll.
</br>
</br>If wrapper cannot find the Converter32/64 dll or if your machine does not support AVX2 it will fall back to use default C++/Cli versions.
</br>External dll 2x+ faster than C++/Cli versions.

## TLDR how to install(deprecated)
- Go to my releases find latest version.
- Reference H264Sharp dll on your C# project.
- Add `openh264-2.3.1-win32.dll` or `openh264-2.3.1-win64.dll` or both to your executable directory(Or include on your project and ckeck copy to output-> copy if newer).
- Keep the original names if you want to use default constructors.
- Optionally Add Converter64/32 dlls to your executable directory same way as openh264 dll.
- Enjoy

## Remarks(deprecated)
- Decode callbacks with raw image formats use cached back buffer, if you wont consume them immediately, make a copy or sync your system.
- Encoder output "EncodedFrame" uses cached back buffer, if you wont consume them immediately, make a copy or sync your system.
- .Net Core and .Net Framework releases are provided.
- Use at least 2.3.1 version of openh264.(cisco has updated some data types, older versions might lead to stack buffer overflow).

- Download Cisco's [`openh264-2.3.1-win32.dll`](http://ciscobinary.openh264.org/openh264-2.3.1-win32.dll.bz2)
- Download Cisco's [`openh264-2.3.1-win64.dll`](http://ciscobinary.openh264.org/openh264-2.3.1-win64.dll.bz2).
