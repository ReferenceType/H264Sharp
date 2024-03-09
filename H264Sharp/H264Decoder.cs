//using System.Drawing.Imaging;
using System.Drawing;
using System.Runtime.InteropServices;
using System;
using System.Text;
using static System.Net.Mime.MediaTypeNames;
using System.Reflection.Emit;
namespace H264Sharp
{
    public class H264Decoder : IDisposable
    {
        #region Dll import
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr GetDecoderx64([MarshalAs(UnmanagedType.LPWStr)] string s);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeDecoderDefaultx64(IntPtr dec);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeDecoderx64(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool DecodeAsRGBx64(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref RGBImagePointer decoded);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool DecodeAsYUVx64(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);
        //DecodeRgbInto
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        private static unsafe extern bool DecodeRgbIntox64(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern void FreeDecoderx64(IntPtr decoder);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetParallelConverterDecx64(IntPtr decoder, int isParallel);
        
        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "UseSSEYUVConverter", CallingConvention = CallingConvention.Cdecl)]
        private static extern void UseSSEConverterDecx64(IntPtr decoder, bool isSSE);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetOptionDecoderx64(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDllName64bit, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetOptionDecoderx64(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        //32
        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "GetDecoder", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr GetDecoderx86([MarshalAs(UnmanagedType.LPWStr)] string s);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "InitializeDecoderDefault", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeDecoderDefaultx86(IntPtr dec);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "InitializeDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int InitializeDecoderx86(IntPtr dec, TagSVCDecodingParam param);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DecodeAsRGB", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool DecodeAsRGBx86(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref RGBImagePointer decoded);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DecodeAsYUV", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool DecodeAsYUVx86(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, ref YUVImagePointer decoded);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "DecodeAsRGBInto", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool DecodeRgbIntox86(IntPtr decoder, ref byte frame, int lenght, bool noDelay, ref int state, IntPtr buffer);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "FreeDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern void FreeDecoderx86(IntPtr decoder);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "SetParallelConverterDec", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetParallelConverterDecx86(IntPtr decoder, int isParallel);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "UseSSEYUVConverter", CallingConvention = CallingConvention.Cdecl)]
        private static extern void UseSSEConverterDecx86(IntPtr decoder, bool isSSE);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "GetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetOptionDecoderx86(IntPtr decoder, DECODER_OPTION option, IntPtr value);

        [DllImport(Defines.WrapperDllName32bit, EntryPoint = "SetOptionDecoder", CallingConvention = CallingConvention.Cdecl)]
        private static extern int SetOptionDecoderx86(IntPtr decoder, DECODER_OPTION option, IntPtr value);
        #endregion

        private readonly IntPtr decoder;
        private bool disposedValue;
        private readonly bool x64;

        private int converterNumberOfThreads;
        private bool enableSSEYUVConversion;

        /// <summary>
        /// Number of threads to use on YUV420P to RGB conversion on decoder
        /// Default is 4.
        /// </summary>
        public int ConverterNumberOfThreads
        {
            get => converterNumberOfThreads;
            set {
                converterNumberOfThreads = value;

                if (x64)
                    SetParallelConverterDecx64(decoder, value);
                else
                    SetParallelConverterDecx86(decoder, value);
            }
        }

        /// <summary>
        /// Enables SSE intrinsics on YUV420P to RGB conversion on decoder
        /// otherwise will use table base converter.
        /// </summary>
        public bool EnableSSEYUVConversion
        {
            get => enableSSEYUVConversion;
            set
            {
                enableSSEYUVConversion = value;

                if (x64)
                    UseSSEConverterDecx64(decoder, value);
                else
                    UseSSEConverterDecx86(decoder, value);
            }
        }

        /// <summary>
        /// Initialises new instance. You can change the cisco dll name with <see cref="Defines"></see> class before initialisation
        /// </summary>
        public H264Decoder()
        {
            x64 = Environment.Is64BitProcess;
            decoder = x64 ? GetDecoderx64(Defines.CiscoDllName64bit) :
                           GetDecoderx86(Defines.CiscoDllName32bit);
        }

        /// <summary>
        /// Initialises decodcer with default parameters
        /// </summary>
        /// <returns>success if 0</returns>
        public int Initialize()
        {
            if (x64)
                return InitializeDecoderDefaultx64(decoder);
            else
                return InitializeDecoderDefaultx86(decoder);

        }
        /// <summary>
        /// Initialises decodcer with custom parameters
        /// </summary>
        /// <param name="param"></param>
        /// <returns>success if 0</returns>
        public int Initialize(TagSVCDecodingParam param)
        {
            if (x64)
                return InitializeDecoderx64(decoder, param);
            else
                return InitializeDecoderx86(decoder, param);

        }

        /// <summary>
        /// Gets decoder option
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public bool GetOption<T>(DECODER_OPTION option, out T value) where T: struct
        {
            unsafe
            {
                value = new T();
                if(value is bool)
                {
                    bool v = (bool)((object)value);
                    byte toSet = v ? (byte)1 : (byte)0;
                    {
                        int r = x64 ? GetOptionDecoderx64(decoder, option, new IntPtr(&toSet)) :
                                GetOptionDecoderx86(decoder, option, new IntPtr(&toSet));
                        var success = (r == 0);
                        value = (T)(object)(toSet == 1 ? true:false );
                        return success;
                    }
                }
                fixed(T* V = &value)
                {
                    int r = x64 ? GetOptionDecoderx64(decoder, option, new IntPtr(V)) :
                            GetOptionDecoderx86(decoder, option, new IntPtr(V));
                    var success = (r == 0);
                    value = *V;
                    return success;
                }
            }

        }

        /// <summary>
        /// Gets decoder option, allows reuse of the value
        /// </summary>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public bool GetOptionRef<T>(DECODER_OPTION option, ref T value) where T : struct
        {
            unsafe
            {
                fixed (T* V = &value)
                {
                    int r = x64 ? GetOptionDecoderx64(decoder, option, new IntPtr(V)) :
                            GetOptionDecoderx86(decoder, option, new IntPtr(V));
                    var success = (r == 0);
                    value = *V;
                    return success;
                }
            }

        }

        /// <summary>
        /// Sets a decoder option
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="option"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public bool SetOption<T>(DECODER_OPTION option, T value) where T : struct
        {
            unsafe
            {
                if (value is bool)
                {
                    bool v = (bool)((object)value);
                    byte toSet = v ? (byte)1 : (byte)0;
                    if (x64)
                        return SetOptionDecoderx64(decoder, option, new IntPtr(&toSet)) == 0;
                    else
                        return SetOptionDecoderx86(decoder, option, new IntPtr(&toSet)) == 0;
                }

                if (x64)
                    return SetOptionDecoderx64(decoder, option, new IntPtr(&value)) == 0;
                else
                    return SetOptionDecoderx86(decoder , option, new IntPtr(&value)) == 0;
            }


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
        public bool Decode(byte[] encoded, int offset, int count, bool noDelay, out DecodingState state, out RGBImagePointer img)
        {
            state = 0;
            img = new RGBImagePointer();
            int state_ = 0;
            unsafe
            {
                fixed (byte* P = &encoded[offset])
                {
                    bool success = x64 ? DecodeAsRGBx64(decoder, ref P[offset], count, noDelay, ref state_, ref img) :
                                         DecodeAsRGBx86(decoder, ref P[offset], count, noDelay, ref state_, ref img);
                    state = (DecodingState)state_;
                    return success;
                }
            }

        }

       /// <summary>
       /// Decodes the data into provided rgb image container
       /// </summary>
       /// <param name="encoded"></param>
       /// <param name="offset"></param>
       /// <param name="count"></param>
       /// <param name="noDelay"></param>
       /// <param name="state"></param>
       /// <param name="img"></param>
       /// <returns></returns>
        public bool Decode(byte[] encoded, int offset, int count, bool noDelay, out DecodingState state, ref RgbImage img)
        {
            state = 0;
            int state_ = 0;
            unsafe
            {
                fixed (byte* P = &encoded[offset])
                {
                    bool success = x64 ? DecodeRgbIntox64(decoder, ref P[offset], count, noDelay, ref state_, img.ImageBytes) :
                                         DecodeRgbIntox86(decoder, ref P[offset], count, noDelay, ref state_, img.ImageBytes);
                    state = (DecodingState)state_;
                    return success;
                }
            }

        }

        /// <summary>
        /// Decodes the data into provided rgb image container
        /// </summary>
        /// <param name="data"></param>
        /// <param name="noDelay"></param>
        /// <param name="state"></param>
        /// <param name="img"></param>
        /// <returns></returns>
        public bool Decode(EncodedData data, bool noDelay, out DecodingState state, ref RgbImage img)
        {
            state = 0;
            int state_ = 0;
            unsafe
            {
              
                var p = (byte*)data.DataPointer;
                bool success = x64 ? DecodeRgbIntox64(decoder, ref p[0], data.Length, noDelay, ref state_, img.ImageBytes) :
                                    DecodeRgbIntox86(decoder, ref p[0], data.Length, noDelay, ref state_, img.ImageBytes);
                state = (DecodingState)state_;
                return success;
                
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
        public bool Decode(byte[] encoded, int offset, int count, bool noDelay, out DecodingState state, out YUVImagePointer img)
        {
            state = 0;
            img = new YUVImagePointer();
            int state_ = 0;
            unsafe
            {
                fixed (byte* P = &encoded[offset])
                {
                    bool success = x64 ? DecodeAsYUVx64(decoder, ref P[offset], count, noDelay, ref state_, ref img) :
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
        public bool Decode(EncodedData data, bool noDelay, out DecodingState state, out YUVImagePointer img)
        {
            state = 0;
            img = new YUVImagePointer();
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
        public bool Decode(EncodedData data, bool noDelay, out DecodingState state, out RGBImagePointer img)
        {
            state = 0;
            img = new RGBImagePointer();
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

        ~H264Decoder()
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
    [Flags]
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
