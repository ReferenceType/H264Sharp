using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace H264Sharp
{
    public class Converter
    {
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void RGBtoYUVx64(ref RGBImagePointer rgb, ref YUVImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.Cdecl)]
        private static extern void RGBtoYUVx86( ref RGBImagePointer rgb, ref YUVImagePointer yuv,int numThreads);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void YUV2RGBx64(ref YUVImagePointer rgb, ref RGBImagePointer yuv, int numThreads);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.Cdecl)]
        private static extern void YUV2RGBx86(ref YUVImagePointer rgb, ref RGBImagePointer yuv, int numThreads);


        //todo
    }
}



