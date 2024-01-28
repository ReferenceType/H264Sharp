//using System.Drawing.Imaging;
using System.Drawing;
using System.Runtime.InteropServices;
using System;
namespace H264PInvoke
{
    public class Decoder:IDisposable
    {
        #region Dll import
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr GetDecoderx64(string s);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.StdCall)]
        private static extern bool DecodeAsRGBx64(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref RGBImage decoded);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.StdCall)]
        private static extern bool DecodeAsYUVx64(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImage decoded);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern void FreeDecoderx64(IntPtr decoder);

        //32
        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr GetDecoderx86(string s);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.StdCall)]
        private static extern bool DecodeAsRGBx86(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref RGBImage decoded);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.StdCall)]
        private static extern bool DecodeAsYUVx86(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImage decoded);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern void FreeDecoderx86(IntPtr decoder);
        #endregion

        private readonly IntPtr decoder;
        private bool disposedValue;
        private readonly bool x64;
        /// <summary>
        /// Initialises new instance. You can change the cisco dll name with <see cref="Defines"></see> class before initialisation
        /// </summary>
        public Decoder()
        {
            x64 = Environment.Is64BitProcess;
            decoder = x64? GetDecoderx64(Defines.CiscoDllName64bit):
                           GetDecoderx86(Defines.CiscoDllName32bit);
        }
       
        /// <summary>
        /// Decodes an encoded data into Image with RGB pixel format.
        /// </summary>
        /// <param name="encoded">Data buffer</param>
        /// <param name="offset">Data buffer offset</param>
        /// <param name="count">Data count</param>
        /// <param name="noDelay">Specifies wether to decode immediately.<br/> This is a Cisco feature and its reccomended to be set to true</param>
        /// <param name="state">Decoding state determines the state of the operation and decoder</param>
        /// <param name="img">Rgb image container</param>
        /// <returns>true if an image is available</returns>
        public bool Decode(byte[] encoded,int offset, int count, bool noDelay, out DecodingState state, out RGBImage img)
        {
            state = 0;
            img = new RGBImage();
            int state_ = 0;
            unsafe
            {
                fixed(byte* P = &encoded[offset])
                {
                    bool success = x64 ? DecodeAsRGBx64(decoder, ref P[offset], count, noDelay, ref state_, ref img):
                                         DecodeAsRGBx86(decoder, ref P[offset], count, noDelay, ref state_, ref img);
                    state = (DecodingState)state_;
                    return success;
                }
            }
           
        }
        /// <summary>
        /// Decodes an encoded data into YUV420Planar Image.
        /// </summary>
        /// <param name="encoded">Data buffer</param>
        /// <param name="offset">Data buffer offset</param>
        /// <param name="count">Data count</param>
        /// <param name="noDelay">Specifies wether to decode immediately.<br/> This is a Cisco feature and its reccomended to be set to true</param>
        /// <param name="state">Decoding state determines the state of the operation and decoder</param>
        /// <param name="img">YUV420Planar Image</param>
        /// <returns>true if an image is available</returns>
        public bool Decode(byte[] encoded, int offset, int count, bool noDelay, out DecodingState state, out YUVImage img)
        {
            state = 0;
            img = new YUVImage();
            int state_ = 0;
            unsafe
            {
                fixed (byte* P = &encoded[offset])
                {
                    bool success = x64 ? DecodeAsYUVx64(decoder, ref P[offset], count, noDelay, ref state_, ref img):
                                         DecodeAsYUVx86(decoder, ref P[offset], count, noDelay, ref state_, ref img);
                    state = (DecodingState)state_;
                    return success;
                }
            }
           
        }
        ///// <summary>
        ///// Decodes an encoded data into Bitmap Image with PixelFormat.Format24bppRgb.
        ///// </summary>
        ///// <param name="encoded">Data buffer</param>
        ///// <param name="offset">Data buffer offset</param>
        ///// <param name="count">Data count</param>
        ///// <param name="noDelay">Specifies wether to decode immediately.<br/> This is a Cisco feature and its reccomended to be set to true</param>
        ///// <param name="state">Decoding state determines the state of the operation and decoder</param>
        ///// <param name="img"></param>
        ///// <returns></returns>
        //public bool Decode(byte[] encoded, int offset, int count, bool noDelay, out DecodingState state, out Bitmap img)
        //{
        //    state = 0;
        //    var img_ = new RGBImage();
        //    int state_ = 0;
        //    img = null;
        //    unsafe
        //    {
        //        fixed (byte* P = &encoded[offset])
        //        {
        //            bool success = x64 ? DecodeAsRGBx64(decoder, ref P[0], count, noDelay, ref state_, ref img_) :
        //                                 DecodeAsRGBx86(decoder, ref P[0], count, noDelay, ref state_, ref img_);
        //            state = (DecodingState)state_;
        //            if (success)
        //                img = RgbToBitmap(img_);
        //            return success;
        //        }
        //    }

        //}



        //public bool Decode(EncodedData data, bool noDelay, out DecodingState state,out Bitmap img)
        //{
        //    unsafe
        //    {
        //        state = 0;
        //        var img_ = new RGBImage();
        //        int state_ = 0;
        //        img = null;

        //        var p= (byte*)data.DataPointer;
        //        bool success = x64 ? DecodeAsRGBx64(decoder, ref p[0], data.Length, noDelay, ref state_, ref img_) :
        //                            DecodeAsRGBx86(decoder, ref p[0], data.Length, noDelay, ref state_, ref img_);
        //        state = (DecodingState)state_;
        //        if (success)
        //            img = RgbToBitmap(img_);
        //        return success;
        //    }

        //}

        /// <summary>
        /// Decodes an encoded data into YUV420Planar Image.
        /// </summary>
        /// <param name="data"></param>
        /// <param name="noDelay"></param>
        /// <param name="state"></param>
        /// <param name="img"></param>
        /// <returns> true if an image is available</returns>
        public bool Decode(EncodedData data, bool noDelay, out DecodingState state, out YUVImage img)
        {
            state = 0;
            img = new YUVImage();
            int state_ = 0;
            unsafe
            {
                var p = (byte*)data.DataPointer;
                bool success = x64 ? DecodeAsYUVx64(decoder, ref p[0], data.Length, noDelay, ref state_, ref img) :
                                     DecodeAsYUVx86(decoder, ref p[0], data.Length, noDelay, ref state_, ref img);
                state = (DecodingState)state_;
                return success;
                
            }

        }
        /// <summary>
        /// Decodes an encoded data into YUV420Planar Image.
        /// </summary>
        /// <param name="data"></param>
        /// <param name="noDelay"></param>
        /// <param name="state"></param>
        /// <param name="img"></param>
        /// <returns> true if an image is available</returns>
        public bool Decode(EncodedData data, bool noDelay, out DecodingState state, out RGBImage img)
        {
            state = 0;
            img = new RGBImage();
            int state_ = 0;
            unsafe
            {
                var p = (byte*)data.DataPointer;
                bool success = x64 ? DecodeAsRGBx64(decoder, ref p[0], data.Length, noDelay, ref state_, ref img) :
                                    DecodeAsRGBx86(decoder, ref p[0], data.Length, noDelay, ref state_, ref img);
                state = (DecodingState)state_;
                return success;

            }

        }

        //private Bitmap RgbToBitmap(RGBImage img)
        //{
        //    Bitmap bmp = new Bitmap(img.Width, img.Height, img.Width * 3, PixelFormat.Format24bppRgb, img.ImageBytes);
        //    return bmp;
        //}

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                FreeDecoderx64(decoder);
                disposedValue = true;
            }
        }

        ~Decoder()
        {
            Dispose(disposing: false);
        }

        public void Dispose()
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }

    /// <summary>
    /// Decoders state flags
    /// </summary>
    public enum DecodingState
    {
        /**
		* Errors derived from bitstream parsing
		*/

        /// <summary>
        /// bit stream error-free
        /// </summary>
        dsErrorFree = 0x00,
        /// <summary>
        /// need more throughput to generate a frame output
        /// </summary>
		dsFramePending = 0x01,
        /// <summary>
        /// layer lost at reference frame with temporal id 0
        /// </summary>
        dsRefLost = 0x02,
        /// <summary>
        /// error bitstreams(maybe broken internal frame) the decoder cared
        /// </summary>
        dsBitstreamError = 0x04,
        /// <summary>
        /// dependented layer is ever lost
        /// </summary>
		dsDepLayerLost = 0x08,
        /// <summary>
        /// no parameter set NALs involved
        /// </summary>
		dsNoParamSets = 0x10,
        /// <summary>
        /// current data error concealed specified
        /// </summary>
        dsDataErrorConcealed = 0x20,
        /// <summary>
        /// picure list contains null ptrs within uiRefCount range
        /// </summary>
		dsRefListNullPtrs = 0x40,

        /**
		* Errors derived from logic level
		*/

        /// <summary>
        /// invalid argument specified
        /// </summary>
        dsInvalidArgument = 0x1000,
        /// <summary>
        /// initializing operation is expected
        /// </summary>
        dsInitialOptExpected = 0x2000,
        /// <summary>
        /// out of memory due to new request
        /// </summary>
		dsOutOfMemory = 0x4000,
        /// <summary>
        /// actual picture size exceeds size of dst pBuffer feed in decoder, so need expand its size
        /// </summary>
        dsDstBufNeedExpan = 0x8000

    };
}
