# H264Sharp
Cisco's OpenH264 C++/CLI wrapper in C# with optimised image format conversions. It is very suitable for realtime streaming.
- Offers managed and unmanaged API.
- Tested on .NetFramework and Net6.
- Compatible with OpenCV.(i.e. OpenCVsharp)
- Tested on WPF application with camera and screen capture (P2P Videocall).
- Simple console application example is provided on repo as an example.
  
### Setup
``` c#
  decoder = new H264Sharp.Decoder();
  
  encoder = new H264Sharp.Encoder();
  encoder.Initialize(width, height, bps: 3_000_000, fps: 30, H264Sharp.Encoder.ConfigType.CameraBasic);
```
- Empty Constructor will look for 32 or 64 bit openh264 dll automatically on executable directory(i.e. Debug/Release folder of your project).
- You can setup with a different dll name, constructor is overloaded.

### Encode 
```C#
  if(encoder.Encode(bitmap, out EncodedFrame[] frames))
  {
      foreach (var frame in frames)
      {
          //byte[] b = frame.ToByteArray();
          //frame.CopyTo(buffer, 0);
          Decode(frame.Data, frame.Length, frame.Type);
      }
  }
```
- You can encode rgb/rgba/bgr/bgra/yuv_i420 on as raw data format or System.Drawing.Bitmaps.
- You have to determine startIndex, width height and stride values for your raw data images.
- Raw data is compatible with OpenCV Mats.
- EncodedFrame represents h264 encoded bytes(NALs etc).
  
### Decode
```C#
  void Decode(IntPtr data, int length, FrameType type)
  {
      //if (decoder.Decode(data, length, noDelay:true, out DecodingState statusCode, out RgbImage rgb)) 
      //if (decoder.Decode(data, length, noDelay:true, out DecodingState statusCode, out Yuv420p yuv420)) 
      if (decoder.Decode(data, length, noDelay:true, out DecodingState statusCode, out Bitmap bmp)) 
      {
          // Do stuff..
          // bmp.Save("t.bmp");
      }
  }
```
- You can decode with pointers or managed byte array as input.
- You can decode into System.Drawing.Bitmaps or raw data format images (they are compatible with OpenCV Mats).
# Converter dll
A separate dll is provided for RGB <-> YUV conversions. Its compiled with clang LLVM and has AVX2 intrinsics.
</br>You can optionally include it on your executable path just like Openh264 dll.
</br>
</br>If wrapper could not find the Converter32/64 dll it will fall back to use C++Cli versions.
</br>External dll 2x+ faster than C++Cli convertors.

# TLDR how to install
- Go to my releases find lates version.
- Reference/Include H264Sharp dll on your project.
- Add `openh264-2.3.1-win32.dll` or `openh264-2.3.1-win64.dll` or both to your executable directory. (keep the original names if you want to use default constructor)
- Optionally Add Converter64/32 dlls to your executable directory.
# Remarks
.Net Core and .Net Framework releases are provided.
Use at least 2.3.1 version of openh264.(cisco has updated some data types, older versions might lead to stack buffer overflow).

- Download Cisco's [`openh264-2.3.1-win32.dll`](http://ciscobinary.openh264.org/openh264-2.3.1-win32.dll.bz2)
- Download Cisco's [`openh264-2.3.1-win64.dll`](http://ciscobinary.openh264.org/openh264-2.3.1-win64.dll.bz2).
