using System;

namespace H264Sharp
{
    public class H264Decoder : IDisposable
    {
        private readonly IntPtr decoder;
        private bool disposedValue;
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
                H264NativeApi.SetParallelConverterDec(decoder, value);
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
                H264NativeApi.UseSSEConverterDec(decoder, value);
            }
        }

        /// <summary>
        /// Initialises new instance
        /// </summary>
        public H264Decoder()
        {
            decoder = H264NativeApi.GetDecoder();
        }

        /// <summary>
        /// Initialises new instance with custom cisco dll name
        /// </summary>
        /// <param name="ciscoDllName"></param>
        public H264Decoder(string ciscoDllName)
        {
            decoder = H264NativeApi.GetDecoder(ciscoDllName);
        }

        /// <summary>
        /// Initialises decoder with default parameters
        /// </summary>
        /// <returns>success if 0</returns>
        public int Initialize()
        {
            return H264NativeApi.InitializeDecoderDefault(decoder);
        }

        /// <summary>
        /// Initialises decoder with custom parameters
        /// </summary>
        /// <param name="param"></param>
        /// <returns>success if 0</returns>
        public int Initialize(TagSVCDecodingParam param)
        {
            return H264NativeApi.InitializeDecoder(decoder, param);
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
                        int r = H264NativeApi.GetOptionDecoder(decoder, option, new IntPtr(&toSet));
                        var success = (r == 0);
                        value = (T)(object)(toSet == 1 ? true:false );
                        return success;
                    }
                }
                fixed(T* V = &value)
                {
                    int r = H264NativeApi.GetOptionDecoder(decoder, option, new IntPtr(V));
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
                    int r = H264NativeApi.GetOptionDecoder(decoder, option, new IntPtr(V));
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
                    return H264NativeApi.SetOptionDecoder(decoder, option, new IntPtr(&toSet)) == 0;
                }

                return H264NativeApi.SetOptionDecoder(decoder, option, new IntPtr(&value)) == 0;
            }
        }

        /// <summary>
        /// Decodes an encoded data into Image with RGB pixel format.
        /// </summary>
        /// <param name="encoded">Data buffer</param>
        /// <param name="offset">Data buffer offset</param>
        /// <param name="count">Data count</param>
        /// <param name="noDelay">Specifies whether to decode immediately.<br/> This is a Cisco feature and its recommended to be set to true</param>
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
                    bool success = H264NativeApi.DecodeAsRGB(decoder, ref P[offset], count, noDelay, ref state_, ref img);
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
                    bool success = H264NativeApi.DecodeRgbInto(decoder, ref P[offset], count, noDelay, ref state_, img.ImageBytes);
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
                bool success = H264NativeApi.DecodeRgbInto(decoder, ref p[0], data.Length, noDelay, ref state_, img.ImageBytes);
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
        /// <param name="noDelay">Specifies whether to decode immediately.<br/> This is a Cisco feature and its recommended to be set to true</param>
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
                    bool success = H264NativeApi.DecodeAsYUV(decoder, ref P[offset], count, noDelay, ref state_, ref img);
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
        ///// <param name="noDelay">Specifies whether to decode immediately.<br/> This is a Cisco feature and its recommended to be set to true</param>
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
                bool success = H264NativeApi.DecodeAsYUV(decoder, ref p[0], data.Length, noDelay, ref state_, ref img);
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
                bool success = H264NativeApi.DecodeAsRGB(decoder, ref p[0], data.Length, noDelay, ref state_, ref img);
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
                H264NativeApi.FreeDecoder(decoder);
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
