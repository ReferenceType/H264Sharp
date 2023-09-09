# H264Sharp
Cisco's OpenH264 C++/CLI wrapper in C# with optimised image format conversions. It is very suitable for realtime streaming over network.
- Offers managed and unmanaged intuitive API.
- Tested on .NetFramework and NetCore(upto 7).
- Compatible with OpenCV.(i.e. OpenCVsharp)
- Tested on WPF application with camera and screen capture (P2P Videocall).
- No memory leaks or GC pressure with bitmaps.
- Simple console application example is provided on repo as an example.
  
### Setup
- Default Constructor will look for `openh264-2.3.1-win32.dll` or `openh264-2.3.1-win64.dll` automatically on executable directory depending on process type.
- You can setup with a different dll name, constructor is overloaded.
``` c#
  decoder = new H264Sharp.Decoder();
  
  encoder = new H264Sharp.Encoder();
  encoder.Initialize(width, height, bps: 3_000_000, fps: 30, H264Sharp.Encoder.ConfigType.CameraBasic);
```

### Encode 
- You can encode from rgb/rgba/bgr/bgra/yuv_i420 on as raw data format, or System.Drawing.Bitmaps.
- Raw data is compatible with OpenCV Mats or any other standard image container.
- EncodedFrame represents h264 encoded bytes(NALs etc).
```C#
  if(encoder.Encode(bitmap, out EncodedFrame[] frames))
  {
      foreach (var frame in frames)
      {
          //hints.. 
          //byte[] b = frame.ToByteArray();
          //frame.CopyTo(buffer, 0);
          Decode(frame.Data, frame.Length, frame.Type);
      }
  }
```

  
### Decode
- You can decode with pointers or managed byte array as input.
- You can decode into System.Drawing.Bitmaps or raw data format images (they are compatible with OpenCV Mats and any other standard image containers.).
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

# Converter dll
A separate dll is provided for RGB <-> YUV conversions. Its compiled with clang LLVM and has AVX2 intrinsics.
</br>You can optionally include it on your executable path just like Openh264 dll.
</br>
</br>If wrapper cannot find the Converter32/64 dll it will fall back to use C++/Cli versions.
</br>External dll 2x+ faster than C++/Cli versions.

# TLDR how to install
- Go to my releases find lates version.
- Reference H264Sharp dll on your project.
- Add `openh264-2.3.1-win32.dll` or `openh264-2.3.1-win64.dll` or both to your executable directory(Or include on your project and ckeck copy to output-> copy if newer).
- Keep the original names if you want to use default constructors.
- Optionally Add Converter64/32 dlls to your executable directory same way as openh264 dll.
- Enjoy
# Remarks
- Decode callbacks with raw image formats use cached backed buffer, if you wont consume them immediately, make a copy or sync your system.
- Encoder output "EncodedFrame" uses cached back buffer if you wont consume them immediately, make a copy or sync your system.
- .Net Core and .Net Framework releases are provided.
- Use at least 2.3.1 version of openh264.(cisco has updated some data types, older versions might lead to stack buffer overflow).

- Download Cisco's [`openh264-2.3.1-win32.dll`](http://ciscobinary.openh264.org/openh264-2.3.1-win32.dll.bz2)
- Download Cisco's [`openh264-2.3.1-win64.dll`](http://ciscobinary.openh264.org/openh264-2.3.1-win64.dll.bz2).
