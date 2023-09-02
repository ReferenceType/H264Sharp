# H264Sharp
Cisco's OpenH264 C++/CLI wrapper in C# comes with optimised image format conversion library.

- Compatible with OpenCV.
- Tested on WPF application with camera and screen capture (P2P Videocall).
- Very Simple console application example is provided on repo.
  
Setup
``` c#
            decoder = new H264Sharp.Decoder();

            encoder = new H264Sharp.Encoder();
            encoder.Initialize(w, h, bps: 200_000_000, fps: 30, H264Sharp.Encoder.ConfigType.CameraBasic);
```

Encode 

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

Decode
```C#
        void Decode(IntPtr data, int length, FrameType type)
        {
            if (decoder.Decode(data, length, noDelay:true, out DecodingState statusCode, out Bitmap bmp)) 
            {
                // Do stuff..
                // bmp.Save("t.bmp");
            }
        }
```
.Net Core and .Net Framework releases are provided.
Tested on .Net6 and .NetFramework 4.7.2.

Reference the wrapper DLL on your project.
Make sure Cisco`s DLL is on your executable path.

- Include Cisco's `openh264-2.1.1-win32` for 32 bit projects.
- include Cisco's `openh264-2.1.1-win64` for 64.
