using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace H264PInvoke
{
    public enum  ImageType { Rgb,Bgr,Rgba,Bgra };
    public enum FrameType
    { 
        /// <summary>
        /// Invalid frame
        /// </summary>
        Invalid,
        /// <summary>
        /// Instant decoder refresh frame
        /// </summary>
        IDR,
        /// <summary>
        /// Intra frame
        /// </summary>
        I,
        /// <summary>
        /// Predicted frame
        /// </summary>
        P, 
        /// <summary>
        /// Skipped
        /// </summary>
        Skip,
        /// <summary>
        /// Mixed Intra-Predicted
        /// </summary>
        IPMixed 
    };


    [StructLayout(LayoutKind.Sequential)]
    internal unsafe ref struct UnsafeGenericImage
    {
        public ImageType ImgType;
        public int Width;
        public int Height;
        public int Stride;
        public byte* ImageBytes;
    };

    /// <summary>
    /// Defines an image pointer woth given pixel format.
    /// <br/> This struct is used for encoding.
    /// </summary>
    public ref struct GenericImage
    {
        public ImageType ImgType;
        public int Width;
        public int Height;
        public int Stride;
        public IntPtr ImageBytes;

    };

    [StructLayout(LayoutKind.Sequential)]
    public readonly ref struct RGBImage
    {
        public readonly int Width;
        public readonly int Height;
        public readonly int Stride;
        public readonly IntPtr ImageBytes;

    };

    [StructLayout(LayoutKind.Sequential)]
    public unsafe readonly ref struct YUVImage
    {
        public readonly byte* Y;
        public readonly byte* U;
        public readonly byte* V;
        public readonly int width;
        public readonly int height;
        public readonly int strideY;
        public readonly int strideU;
        public readonly int strideV;

    };

    [StructLayout(LayoutKind.Sequential)]
    internal unsafe readonly ref struct EncodedFrame
    {
        public readonly byte* Data;
        public readonly int Length;
        public readonly int LayerNum;
        public readonly int Type;
    };

   [StructLayout(LayoutKind.Sequential)]
    internal unsafe ref struct FrameContainer
    {
        public EncodedFrame* Frames;
        public readonly int Lenght;
    };

    /// <summary>
    /// holds h264 encoded data per layer
    /// </summary>
    public readonly struct EncodedData
    {
        public readonly IntPtr DataPointer;
        public readonly int Length;
        public readonly FrameType FrameType;
        public readonly int LayerNum;
        public EncodedData(IntPtr dataPointer, int size, FrameType frameType, int layerNum)
        {
            DataPointer = dataPointer;
            Length = size;
            FrameType = frameType;
            LayerNum = layerNum;
        }

        public byte[] GetBytes()
        {
            var b = new byte[Length];
            Marshal.Copy(DataPointer, b, 0, Length);
            return b;
        }

        public void WriteTo(Stream s)
        {
            var b = new byte[Length];
            Marshal.Copy(DataPointer, b, 0, Length);
            s.Write(b,0,b.Length);
        }

        public void CopyTo(byte[] buffer, int startIndex)
        {
            if (buffer.Length - startIndex < Length)
            {
                throw new InvalidOperationException("Not enough space in provided byte[] buffer");
            }
            Marshal.Copy(DataPointer, buffer, startIndex, Length);

        }

        public static int CopyTo(EncodedData[] datas, byte[] toBuffer, int startIndex)
        {
            int written = 0;
            for (int i = 0; i < datas.Length; i++)
            {
                datas[i].CopyTo(toBuffer, startIndex);
                written += datas[i].Length;
            }
            return written;
        }
    }
}
