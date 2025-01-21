using System;
using System.IO;
using System.Runtime.InteropServices;

namespace H264Sharp
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
    /// Represents an image. Source can come from managed bytes or unmanaged.
    /// in case of unmanaged, no copy is created.
    /// </summary>
    public class ImageData
    {
        public readonly ImageType ImgType;
        public readonly int Width;
        public readonly int Height;
        /// <summary>
        /// stride is the width of one line of rgb/rgba.
        /// Typically its (width*height*3) for rgb and (width*height*4) for rgba
        /// </summary>
        public readonly int Stride;

        internal IntPtr imageData;
        internal byte[] data;
        internal int dataOffset;
        internal int dataLength;
        internal bool isManaged;

        /// <summary>
        /// Creates an instance with managed bytes.
        /// </summary>
        /// <param name="imgType"></param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="stride"></param>
        /// <param name="data"></param>
        public ImageData(ImageType imgType, int width, int height, int stride, byte[] data)
        {
            ImgType = imgType;
            Width = width;
            Height = height;
            Stride = stride;
            this.data = data;
            this.dataLength= data.Length;
            isManaged = true;
        }

        /// <summary>
        /// Creates an instance with managed bytes.
        /// </summary>
        /// <param name="imgType"></param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="stride"></param>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="count"></param>
        public ImageData(ImageType imgType, int width, int height, int stride, byte[] data, int offset, int count)
        {
            ImgType = imgType;
            Width = width;
            Height = height;
            Stride = stride;
            this.data = data;
            this.dataOffset= offset;
            this.dataLength = count;
            isManaged = true;
        }

        /// <summary>
        /// Creates an instance with unmanaged img pointer.
        /// Does not copy!
        /// </summary>
        /// <param name="imgType"></param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="stride"></param>
        /// <param name="imageBytes"></param>
        public ImageData(ImageType imgType, int width, int height, int stride, IntPtr imageBytes)
        {
            ImgType = imgType;
            Width = width;
            Height = height;
            Stride = stride;
            imageData= imageBytes;
            isManaged = false;
        }

       
    }

    /// <summary>
    /// Represents an RGB image stored on inner buffer of decoder.
    /// Next decode will overwite this data
    /// use with caution.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public readonly ref struct RGBImagePointer
    {
        public readonly int Width;
        public readonly int Height;
        public readonly int Stride;
        public readonly IntPtr ImageBytes;

        internal RGBImagePointer(int width, int height, int stride, IntPtr imageBytes)
        {
            Width = width;
            Height = height;
            Stride = stride;
            ImageBytes = imageBytes;
        }

        public byte[] GetBytes()
        {
            int Length = Stride * Height;
            var b = new byte[Length];
            unsafe
            {
                fixed (void* ptr = b)
                {
                    Buffer.MemoryCopy(ImageBytes.ToPointer(), ptr, Length, Length);
                }
            }
            //Marshal.Copy(ImageBytes, b, 0, Length);
            return b;
        }

        public void WriteTo(Stream s)
        {
            int Length = Stride * Height;

            var b = new byte[Length];
            
            Marshal.Copy(ImageBytes, b, 0, Length);
            s.Write(b, 0, b.Length);
        }

        public void CopyTo(byte[] buffer, int startIndex)
        {
            int Length = Stride * Height;

            if (buffer.Length - startIndex < Length)
            {
                throw new InvalidOperationException("Not enough space in provided byte[] buffer");
            }
           
            Marshal.Copy(ImageBytes, buffer, startIndex, Length);

        }

        
    };

    /// <summary>
    /// Unsafe YUV420P pointer struct
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public unsafe readonly ref struct YUVImagePointer
    {
        public readonly byte* Y;
        public readonly byte* U;
        public readonly byte* V;
        public readonly int width;
        public readonly int height;
        public readonly int strideY;
        public readonly int strideUV;

        public YUVImagePointer(byte* y, byte* u, byte* v, int width, int height, int strideY, int strideUV)
        {
            Y = y;
            U = u;
            V = v;
            this.width = width;
            this.height = height;
            this.strideY = strideY;
            this.strideUV = strideUV;
        }
    };

    /// <summary>
    /// Represent YUV420 Planar image. 
    /// This class owns unmanaged raw image bytes upon creation
    /// </summary>
    public class YuvImage
    {
        public readonly int Width;
        public readonly int Height;
        public readonly int strideY;
        public readonly int strideUV;
        public readonly IntPtr ImageBytes;
        private bool disposedValue;

        public YuvImage(int width, int height)
        {
            Width = width;
            Height = height;
            strideY = width;
            strideUV =  width/2;
            this.ImageBytes = Marshal.AllocHGlobal((width * height)+(width*height)/2);
        }

        internal unsafe YUVImagePointer ToYUVImagePointer()
        {
            return new YUVImagePointer(
                (byte*)ImageBytes.ToPointer(),
                (byte*)IntPtr.Add(ImageBytes, Width * Height),
                (byte*)IntPtr.Add(ImageBytes, Width * Height + (Width*Height)/4),
                Width, Height, strideY, strideUV);
            
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                Marshal.FreeHGlobal(ImageBytes);
                disposedValue = true;
            }
        }

        ~YuvImage()
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
    /// Represents storable and reusable rgb image container
    /// </summary>
    public class RgbImage:IDisposable
    {
        public IntPtr ImageBytes;
        public int offset;
        public int Width;
        public int Height;
        public int Stride;
        private bool disposedValue;

        /// <summary>
        /// Initialises new instance and allocates unmanaged memory(w*h*3) to be used in decoder.
        /// </summary>
        /// <param name="width"></param>
        /// <param name="height"></param>
        public RgbImage(int width, int height)
        {
           
            this.Width = width;
            this.Height = height;
            this.offset = 0;
            this.Stride = width*3;
            this.ImageBytes = Marshal.AllocHGlobal(width * height*3);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                Marshal.FreeHGlobal(ImageBytes);
                disposedValue = true;
            }
        }

        ~RgbImage()
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
    /// Represents storable bgr image
    /// you can pool images and reuse them.
    /// </summary>
    public class BgrImage : IDisposable
    {
        public IntPtr ImageBytes;
        public int offset;
        public int Width;
        public int Height;
        public int Stride;
        private bool disposedValue;

        public BgrImage(int width, int height)
        {
            this.Width = width;
            this.Height = height;
            this.offset = 0;
            this.Stride = width * 3;
            this.ImageBytes = Marshal.AllocHGlobal(width * height * 3);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                Marshal.FreeHGlobal(ImageBytes);
                disposedValue = true;
            }
        }

        ~BgrImage()
        {
            Dispose(disposing: false);
        }

        public void Dispose()
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }


    [StructLayout(LayoutKind.Sequential)]
    internal unsafe readonly ref struct EncodedFrame
    {
        public readonly byte* Data;
        public readonly int Length;
        public readonly int LayerNum;
        public readonly int Type;
        public readonly byte uiTemporalId;
        public readonly byte uiSpatialId;
        public readonly byte uiQualityId;
        public readonly byte uiLayerType;
        public readonly int iSubSeqId;

    };

   [StructLayout(LayoutKind.Sequential)]
    internal unsafe ref struct FrameContainer
    {
        public EncodedFrame* Frames;
        public readonly int FrameCount;
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
        public readonly byte TemporalId;
        public readonly byte SpatialId;
        public readonly byte QualityId;
        public readonly byte LayerType;
        public readonly int SubSeqId;
       
        internal unsafe EncodedData(EncodedFrame ef)
        {

            DataPointer = new IntPtr(ef.Data);
            Length = ef.Length;
            FrameType = (FrameType)ef.Type;
            LayerNum = ef.LayerNum;
            TemporalId = ef.uiTemporalId;
            SpatialId = ef.uiSpatialId;
            QualityId = ef.uiQualityId;  
            LayerType = ef.uiLayerType;
            SubSeqId = ef.iSubSeqId;
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

        public int CopyTo(byte[] buffer, int startIndex)
        {
            if (buffer.Length - startIndex < Length)
            {
                throw new InvalidOperationException("Not enough space in provided byte[] buffer");
            }
           
            Marshal.Copy(DataPointer, buffer, startIndex, Length);
            return Length;
        }

        public static int CopyTo(EncodedData[] datas, byte[] toBuffer, int startIndex)
        {
            int written = 0;
            for (int i = 0; i < datas.Length; i++)
            {
                datas[i].CopyTo(toBuffer, startIndex+written);
                written += datas[i].Length;
            }
            return written;
        }
    }


    #region Native API Data
    //------------------------
    [StructLayout(LayoutKind.Sequential)]
    public struct TagEncParamBase
    {
        /// <summary>
        /// application type; please refer to the definition of EUsageType
        /// </summary>
        public EUsageType iUsageType;

        /// <summary>
        /// width of picture in luminance samples (the maximum of all layers if multiple spatial layers presents)
        /// </summary>
        public int iPicWidth;
        /// <summary>
        /// height of picture in luminance samples((the maximum of all layers if multiple spatial layers presents)
        /// </summary>
        public int iPicHeight;
        /// <summary>
        /// target bitrate desired, in unit of bps
        /// </summary>
        public int iTargetBitrate;
        /// <summary>
        /// rate control mode
        /// </summary>
        public RC_MODES iRCMode;
        /// <summary>
        ///  maximal input frame rate
        /// </summary>
        public float fMaxFrameRate;

    }

    
    [StructLayout(LayoutKind.Sequential)]
    public struct TagEncParamExt
    {
        public EUsageType iUsageType;
        /// <summary>
        /// Width of picture in luminance samples (the maximum of all layers if multiple spatial layers presents)
        /// </summary>
        public int iPicWidth;
        /// <summary>
        /// Height of picture in luminance samples((the maximum of all layers if multiple spatial layers presents)
        /// </summary>
        public int iPicHeight;
        /// <summary>
        /// target bitrate desired, in unit of bps
        /// </summary>
        public int iTargetBitrate;    
        /// <summary>
        /// Rate control mode
        /// </summary>
        public RC_MODES iRCMode;   
        /// <summary>
        /// Max framerate. This affects rate control, try to match it with actual incoming frame rate
        /// </summary>
        public float fMaxFrameRate;
        /// <summary>
        ///temporal layer number, max temporal layer = 4
        /// </summary>
        public int iTemporalLayerNum;
        /// <summary>
        /// spatial layer number,1<= iSpatialLayerNum <= MAX_SPATIAL_LAYER_NUM, MAX_SPATIAL_LAYER_NUM = 4
        /// </summary>
        public int iSpatialLayerNum;      
        /// <summary>
        /// Up to 4 spacial layer config
        /// </summary>
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public SSpatialLayerConfig[] sSpatialLayers;

        public ECOMPLEXITY_MODE iComplexityMode;
        /// <summary>
        /// Period of Intra frame(every n'th frame)
        /// </summary>
        public uint uiIntraPeriod;
        /// <summary>
        /// number of reference frame used
        /// </summary>
        public int iNumRefFrame;
        /// <summary>
        /// Different stategy in adjust ID in SPS/PPS: 0- constant ID, 1-additional ID, 6-mapping and additional
        /// </summary>
        public EParameterSetStrategy eSpsPpsIdStrategy;
        /// <summary>
        /// false:not use Prefix NAL; true: use Prefix NAL
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bPrefixNalAddingCtrl;
        /// <summary>
        /// false:not use SSEI; true: use SSEI
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bEnableSSEI;

        /// <summary>
        /// (when encoding more than 1 spatial layer) false: use SVC syntax for higher layers; true: use Simulcast AVC
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bSimulcastAVC;
        /// <summary>
        ///  0:disable padding;1:padding
        /// </summary>
        public int iPaddingFlag;
        /// <summary>
        /// 0:CAVLC  1:CABAC.
        /// CABAC is better but more expensive
        /// </summary>
        public int iEntropyCodingModeFlag;

        /* rc control */
        /// <summary>
        /// False: don't skip frame even if VBV buffer overflow.True: allow skipping frames to keep the bitrate within limits
        /// without skips rate control can overshoot.
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bEnableFrameSkip;
        /// <summary>
        /// the maximum bitrate, in unit of bps, set it to UNSPECIFIED_BIT_RATE(0) if not needed 
        /// </summary>
        public int iMaxBitrate;
        /// <summary>
        ///  the maximum QP encoder supports
        ///  Higher qp means higher compression, lower qp is more quality
        /// </summary>
        public int iMaxQp;
        /// <summary>
        /// the minmum QP encoder supports
        /// </summary>
        public int iMinQp;
        /// <summary>
        /// the maximum NAL size.  This value should be not 0 for dynamic slice mode
        /// </summary>
        public uint uiMaxNalSize;           

        /*LTR settings*/
        [MarshalAs(UnmanagedType.U1)] public bool bEnableLongTermReference;
        /// <summary>
        ///  the number of LTR(long term reference),TODO: not supported to set it arbitrary yet
        /// </summary>
        public int iLTRRefNum;
        /// <summary>
        /// the LTR marked period that is used in feedback. 0 is 30
        /// </summary>
        public uint iLtrMarkPeriod;
        /* multi-thread settings*/

        /// <summary>
        /// 1 # 0: auto(dynamic imp. internal encoder); 1: multiple threads imp. disabled; lager than 1: count number of threads;
        /// </summary>
        public ushort iMultipleThreadIdc;
        /// <summary>
        /// Only used when uiSliceMode=1 or 3, will change slicing of a picture during the run-time of multi-thread encoding, so the result of each run may be different
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bUseLoadBalancing;

        /* Deblocking loop filter */
        /// <summary>
        /// 0: on, 1: off, 2: on except for slice boundaries
        /// </summary>
        public int iLoopFilterDisableIdc;
        /// <summary>
        /// AlphaOffset: valid range [-6, 6], default 0
        /// </summary>
        public int iLoopFilterAlphaC0Offset;
        /// <summary>
        /// BetaOffset: valid range [-6, 6], default 0
        /// </summary>
        public int iLoopFilterBetaOffset;
        /*pre-processing feature*/
        /// <summary>
        /// denoise control
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bEnableDenoise;
        /// <summary>
        /// background detection control //VAA_BACKGROUND_DETECTION //BGD cmd
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bEnableBackgroundDetection;
        /// <summary>
        /// adaptive quantization control
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bEnableAdaptiveQuant;
        /// <summary>
        /// enable frame cropping flag: TRUE always in application
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bEnableFrameCroppingFlag;    
        [MarshalAs(UnmanagedType.U1)] public bool bEnableSceneChangeDetect;
        /// <summary>
        /// LTR advanced setting
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bIsLosslessLink;
        /// <summary>
        /// fix rate control overshooting
        /// </summary>
        [MarshalAs(UnmanagedType.U1)]   public bool bFixRCOverShoot;
        /// <summary>
        /// the target bits of IDR is (idr_bitrate_ratio/100) * average target bit per frame.
        /// </summary>
        public int iIdrBitrateRatio;           
    }

    public enum EUsageType 
    {
        CAMERA_VIDEO_REAL_TIME,      
        SCREEN_CONTENT_REAL_TIME,
        /// <summary>
        /// Doesnt seem to be supported
        /// </summary>
        CAMERA_VIDEO_NON_REAL_TIME,
        /// <summary>
        /// Doesnt seem to be supported
        /// </summary>
        SCREEN_CONTENT_NON_REAL_TIME,
        /// <summary>
        /// Doesnt seem to be supported
        /// </summary>
        INPUT_CONTENT_TYPE_ALL,
    }

    public enum RC_MODES
    {
        /// <summary>
        /// quality mode
        /// </summary>
        RC_QUALITY_MODE = 0,
        /// <summary>
        /// bitrate mode
        /// </summary>
        RC_BITRATE_MODE = 1,
        /// <summary>
        /// no bitrate control,only using buffer status,adjust the video quality
        /// </summary>
        RC_BUFFERBASED_MODE = 2,
        /// <summary>
        /// rate control based timestamp
        /// </summary>
        RC_TIMESTAMP_MODE = 3,
        /// <summary>
        /// this is in-building RC MODE, WILL BE DELETED after algorithm tuning!
        /// </summary>
        RC_BITRATE_MODE_POST_SKIP = 4,
        /// <summary>
        ///  rate control off mode
        /// </summary>
        RC_OFF_MODE = -1,
    };
     public enum ECOMPLEXITY_MODE
    {
        /// <summary>
        /// the lowest compleixty,the fastest speed,
        /// </summary>
        LOW_COMPLEXITY = 0,
        /// <summary>
        /// medium complexity, medium speed,medium quality
        /// </summary>
        MEDIUM_COMPLEXITY,
        /// <summary>
        ///  high complexity, lowest speed, high quality
        /// </summary>
        HIGH_COMPLEXITY
    }
    ;
    public enum EParameterSetStrategy
    {
        /// <summary>
        /// constant id in SPS/PPS
        /// </summary>
        CONSTANT_ID = 0,
        /// <summary>
        /// SPS/PPS id increases at each IDR
        /// </summary>
        INCREASING_ID = 0x01,
        /// <summary>
        /// using SPS in the existing list if possible
        /// </summary>
        SPS_LISTING = 0x02,
        SPS_LISTING_AND_PPS_INCREASING = 0x03,
        SPS_PPS_LISTING = 0x06,
    }
    ;

    [StructLayout(LayoutKind.Sequential)]
    public struct SSpatialLayerConfig
    {
        /// <summary>
        /// width of picture in luminance samples of a layer
        /// </summary>
        public int iVideoWidth;
        /// <summary>
        /// height of picture in luminance samples of a layer
        /// </summary>
        public int iVideoHeight;
        /// <summary>
        /// frame rate specified for a layer
        /// </summary>
        public float fFrameRate;
        /// <summary>
        /// target bitrate for a spatial layer, in unit of bps
        /// </summary>
        public int iSpatialBitrate;
        /// <summary>
        /// maximum  bitrate for a spatial layer, in unit of bps
        /// </summary>
        public int iMaxSpatialBitrate;
        /// <summary>
        /// value of profile IDC (PRO_UNKNOWN for auto-detection)
        /// </summary>
        public EProfileIdc uiProfileIdc;
        /// <summary>
        /// value of profile IDC (0 for auto-detection)
        /// </summary>
        public ELevelIdc uiLevelIdc;
        /// <summary>
        ///  value of level IDC (0 for auto-detection)
        /// </summary>
        public int iDLayerQp;      

        public SSliceArgument sSliceArgument;

        // Note: members bVideoSignalTypePresent through uiColorMatrix below are also defined in SWelsSPS in parameter_sets.h.
        /// <summary>
        /// false => do not write any of the following information to the header
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bVideoSignalTypePresent;
        /// <summary>
        /// EVideoFormatSPS; 3 bits in header; 0-5 => component, kpal, ntsc, secam, mac, undef
        /// </summary>
        public byte uiVideoFormat;
        /// <summary>
        /// false => analog video data range [16, 235]; true => full data range [0,255]
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bFullRange;
        /// <summary>
        /// false => do not write any of the following three items to the header
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bColorDescriptionPresent;
        /// <summary>
        /// EColorPrimaries; 8 bits in header; 0 - 9 => ???, bt709, undef, ???, bt470m, bt470bg,
        /// smpte170m, smpte240m, film, bt2020
        /// </summary>
        public byte uiColorPrimaries;
        ///<summary>
        /// ETransferCharacteristics; 8 bits in header; 0 - 15 => ???, bt709, undef, ???, bt470m, bt470bg, smpte170m,
        ///   smpte240m, linear, log100, log316, iec61966-2-4, bt1361e, iec61966-2-1, bt2020-10, bt2020-12
        ///</summary>                
        public byte uiTransferCharacteristics;
        /// <summary>
        /// EColorMatrix; 8 bits in header (corresponds to FFmpeg "colorspace"); 0 - 10 => GBR, bt709,
        /// undef, ???, fcc, bt470bg, smpte170m, smpte240m, YCgCo, bt2020nc, bt2020c
        /// </summary>
        public byte uiColorMatrix;

        /// <summary>
        /// aspect ratio present in VUI
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bAspectRatioPresent;
        /// <summary>
        /// aspect ratio idc
        /// </summary>
        public ESampleAspectRatio eAspectRatio;
        /// <summary>
        /// use if aspect ratio idc == 255
        /// </summary>
        public ushort sAspectRatioExtWidth;
        /// <summary>
        /// use if aspect ratio idc == 255
        /// </summary>
        public ushort sAspectRatioExtHeight; 

    };
    public enum EProfileIdc
    {
        PRO_UNKNOWN = 0,
        PRO_BASELINE = 66,
        PRO_MAIN = 77,
        PRO_EXTENDED = 88,
        PRO_HIGH = 100,
        PRO_HIGH10 = 110,
        PRO_HIGH422 = 122,
        PRO_HIGH444 = 144,
        PRO_CAVLC444 = 244,

        PRO_SCALABLE_BASELINE = 83,
        PRO_SCALABLE_HIGH = 86
    }
    ;

    public enum ELevelIdc
    {
        LEVEL_UNKNOWN = 0,
        LEVEL_1_0 = 10,
        LEVEL_1_B = 9,
        LEVEL_1_1 = 11,
        LEVEL_1_2 = 12,
        LEVEL_1_3 = 13,
        LEVEL_2_0 = 20,
        LEVEL_2_1 = 21,
        LEVEL_2_2 = 22,
        LEVEL_3_0 = 30,
        LEVEL_3_1 = 31,
        LEVEL_3_2 = 32,
        LEVEL_4_0 = 40,
        LEVEL_4_1 = 41,
        LEVEL_4_2 = 42,
        LEVEL_5_0 = 50,
        LEVEL_5_1 = 51,
        LEVEL_5_2 = 52
    }
    ;

   
    [StructLayout(LayoutKind.Sequential)]
    public struct SSliceArgument
    {
        /// <summary>
        /// by default, uiSliceMode will be SM_SINGLE_SLICE
        /// </summary>
        public SliceModeEnum uiSliceMode;
        /// <summary>
        /// only used when uiSliceMode=1, when uiSliceNum=0 means auto design it with cpu core number
        /// </summary>
        public uint uiSliceNum;
        /// <summary>
        /// only used when uiSliceMode=2; when =0 means setting one MB row a slice
        /// </summary>
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = ((128 - ((4 * 4) + 1 + 4)) / 3))]
        public uint[] uiSliceMbNum;
        /// <summary>
        /// now only used when uiSliceMode=4
        /// </summary>
        public uint uiSliceSizeConstraint; 
    }
;
    public enum SliceModeEnum
    {
        /// <summary>
        /// SliceNum=1
        /// </summary>
        SM_SINGLE_SLICE = 0,
        /// <summary>
        /// according to SliceNum,  enabled dynamic slicing for multi-thread
        /// </summary>
        SM_FIXEDSLCNUM_SLICE = 1,
        /// <summary>
        /// according to SlicesAssign    
        /// need input of MB numbers each slice. In addition, 
        /// if other constraint in SSliceArgument is presented, need to follow the constraints.
        /// Typically if MB num and slice size are both constrained, re-encoding may be involved.
        /// </summary>
        SM_RASTER_SLICE = 2,
        /// <summary>
        /// according to SliceSize       | slicing according to size, the slicing will be dynamic(have no idea about slice_nums until encoding current frame)
        /// </summary>
        SM_SIZELIMITED_SLICE = 3, 
        SM_RESERVED = 4
    }
    ;
    public enum TRACE_LEVEL
    {
        WELS_LOG_QUIET = 0x00,         
        WELS_LOG_ERROR = 1 << 0,        
        WELS_LOG_WARNING = 1 << 1,       
        WELS_LOG_INFO = 1 << 2,        
        WELS_LOG_DEBUG = 1 << 3,        
        WELS_LOG_DETAIL = 1 << 4,       
        WELS_LOG_RESV = 1 << 5,        
        WELS_LOG_LEVEL_COUNT = 6,
        WELS_LOG_DEFAULT = WELS_LOG_WARNING  
    }
    public enum ESampleAspectRatio
    {
        ASP_UNSPECIFIED = 0,
        ASP_1x1 = 1,
        ASP_12x11 = 2,
        ASP_10x11 = 3,
        ASP_16x11 = 4,
        ASP_40x33 = 5,
        ASP_24x11 = 6,
        ASP_20x11 = 7,
        ASP_32x11 = 8,
        ASP_80x33 = 9,
        ASP_18x11 = 10,
        ASP_15x11 = 11,
        ASP_64x33 = 12,
        ASP_160x99 = 13,

        ASP_EXT_SAR = 255
    }
    ;
    //---------------------------
    [StructLayout(LayoutKind.Sequential)]
    public struct TagSVCDecodingParam
    {
        /// <summary>
        /// file name of reconstructed frame used for PSNR calculation based debug
        /// </summary>
        [MarshalAs(UnmanagedType.LPStr)]
        public string pFileNameRestructed;

        public uint uiCpuLoad;
        /// <summary>
        /// setting target dq layer id
        /// </summary>
        public byte uiTargetDqLayer;

        /// <summary>
        /// whether active error concealment feature in decoder
        /// </summary>
        public ERROR_CON_IDC eEcActiveIdc;
        /// <summary>
        /// decoder for parse only, no reconstruction. When it is true, SPS/PPS size should not exceed SPS_PPS_BS_SIZE (128).
        /// Otherwise, it will return error info
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] public bool bParseOnly;

        /// <summary>
        ///  video stream property
        /// </summary>
        public SVideoProperty sVideoProperty;    
    }


    public enum ERROR_CON_IDC
    {
        ERROR_CON_DISABLE = 0,
        ERROR_CON_FRAME_COPY,
        ERROR_CON_SLICE_COPY,
        ERROR_CON_FRAME_COPY_CROSS_IDR,
        ERROR_CON_SLICE_COPY_CROSS_IDR,
        ERROR_CON_SLICE_COPY_CROSS_IDR_FREEZE_RES_CHANGE,
        ERROR_CON_SLICE_MV_COPY_CROSS_IDR,
        ERROR_CON_SLICE_MV_COPY_CROSS_IDR_FREEZE_RES_CHANGE
    }
    ;
    [StructLayout(LayoutKind.Sequential)]
    public struct SVideoProperty
    {
        public uint size;          ///< size of the struct
        public VIDEO_BITSTREAM_TYPE eVideoBsType;  ///< video stream type (AVC/SVC)
    }
    ;
    public enum VIDEO_BITSTREAM_TYPE
    {
        VIDEO_BITSTREAM_AVC = 0,
        VIDEO_BITSTREAM_SVC = 1,
        VIDEO_BITSTREAM_DEFAULT = VIDEO_BITSTREAM_SVC
    }
    ;
    //---------------------------
    /**
* @brief Option types introduced in SVC encoder application
*/
    public enum ENCODER_OPTION
    {
        ENCODER_OPTION_DATAFORMAT = 0,
        /// <summary>
        /// IDR period,0/-1 means no Intra period (only the first frame); lager than 0 means the desired IDR period, must be multiple of (2^temporal_layer)
        /// <br/>type is int32_t
        /// </summary>
        ENCODER_OPTION_IDR_INTERVAL,
        /// <summary>
        ///  structure of Base Param
        ///  <br/>type is SEncParamBase
        /// </summary>
        ENCODER_OPTION_SVC_ENCODE_PARAM_BASE,
        /// <summary>
        /// structure of Extension Param
        /// <br/>type is SEncParamExt
        /// </summary>
        ENCODER_OPTION_SVC_ENCODE_PARAM_EXT,
        /// <summary>
        /// maximal input frame rate, current supported range: MAX_FRAME_RATE = 30,MIN_FRAME_RATE = 1
        /// <br/>type is float
        /// </summary>
        ENCODER_OPTION_FRAME_RATE,
        /// <summary>
        /// <br/>type is SBitrateInfo
        /// </summary>
        ENCODER_OPTION_BITRATE,
        /// <summary>
        /// <br/>type is SBitrateInfo
        /// </summary>
        ENCODER_OPTION_MAX_BITRATE,
        /// <summary>
        /// this feature not supported.
        /// </summary>
        ENCODER_OPTION_INTER_SPATIAL_PRED,
        /// <summary>
        /// // 0:quality mode;1:bit-rate mode;2:bitrate limited mode
        /// <br/>int32
        /// </summary>
        ENCODER_OPTION_RC_MODE,
        /// <summary>
        /// enable frame skip
        /// <br/>byte 1 or 0
        /// </summary>
        ENCODER_OPTION_RC_FRAME_SKIP,
        /// <summary>
        /// ///< 0:disable padding;1:padding
        /// <br/>int32_t
        /// </summary>
        ENCODER_PADDING_PADDING,

        /// <summary>
        /// assgin the profile for each layer
        /// <br/>type is SProfileInfo
        /// </summary>
        ENCODER_OPTION_PROFILE,
        /// <summary>
        /// assgin the level for each layer
        /// <br/>type SLevelInfo
        /// </summary>
        ENCODER_OPTION_LEVEL,
        /// <summary>
        /// the number of refererence frame
        /// <br/>int32_t
        /// </summary>
        ENCODER_OPTION_NUMBER_REF,
        /// <summary>
        /// the delivery info which is a feedback from app level.
        /// <br/> data type is SDeliveryStatus
        /// </summary>
        ENCODER_OPTION_DELIVERY_STATUS,

        /// <summary>
        /// LTR recovery request.
        /// <br/>Data type is SLTRRecoverRequest
        /// </summary>
        ENCODER_LTR_RECOVERY_REQUEST,
        /// <summary>
        /// Data type is SLTRMarkingFeedback
        /// </summary>
        ENCODER_LTR_MARKING_FEEDBACK,
        /// <summary>
        /// Ltr marking period.
        /// uint32_t
        /// </summary>
        ENCODER_LTR_MARKING_PERIOD,
        /// <summary>
        /// < 0:disable LTR;larger than 0 enable LTR; LTR number is fixed to be 2 in current encoder.
        /// <br/>Datatype is SLTRConfig
        /// </summary>
        ENCODER_OPTION_LTR,
        /// <summary>
        /// Encoder complexity.
        ///<br/> int32_t or ECOMPLEXITY_MODE enum
        /// </summary>
        ENCODER_OPTION_COMPLEXITY,

        /// <summary>
        /// byte 1 or 0
        /// </summary>
        ENCODER_OPTION_ENABLE_SSEI,
        /// <summary>
        /// enable prefix: true--enable prefix; false--disable prefix.
        /// <br/>byte 1 or 0
        /// </summary>
        ENCODER_OPTION_ENABLE_PREFIX_NAL_ADDING,

        /// <summary>
        /// EParameterSetStrategy enum
        /// </summary>
        ENCODER_OPTION_SPS_PPS_ID_STRATEGY, ///< different stategy in adjust ID in SPS/PPS: 0- constant ID, 1-additional ID, 6-mapping and additional

        ENCODER_OPTION_CURRENT_PATH,
        ENCODER_OPTION_DUMP_FILE,                  ///< dump layer reconstruct frame to a specified file
        ENCODER_OPTION_TRACE_LEVEL,                ///< trace info based on the trace level
        ENCODER_OPTION_TRACE_CALLBACK,             ///< a void (*)(void* context, int level, const char* message) function which receives log messages
        ENCODER_OPTION_TRACE_CALLBACK_CONTEXT,

        /// <summary>
        /// SEncoderStatistics
        /// </summary>
        ENCODER_OPTION_GET_STATISTICS,
        /// <summary>
        /// log interval in millisecond
        /// <br/>int32
        /// </summary>
        ENCODER_OPTION_STATISTICS_LOG_INTERVAL,

        /// <summary>
        /// advanced algorithmetic settings
        /// <br/> byte
        /// </summary>
        ENCODER_OPTION_IS_LOSSLESS_LINK,

        /// <summary>
        /// bit vary percentage
        /// <br/>int
        /// </summary>
        ENCODER_OPTION_BITS_VARY_PERCENTAGE 
    }
    ;

    /**
    * @brief Option types introduced in decoder application
*/
    public enum DECODER_OPTION
    {
        /// <summary>
        /// end of stream flag
        /// int
        /// </summary>
        DECODER_OPTION_END_OF_STREAM = 1,
        /// <summary>
        /// feedback whether or not have VCL NAL in current AU for application layer
        /// int
        /// </summary>
        DECODER_OPTION_VCL_NAL,
        /// <summary>
        /// feedback temporal id for application layer
        /// int
        /// </summary>
        DECODER_OPTION_TEMPORAL_ID,
        /// <summary>
        /// feedback current decoded frame number
        /// int
        /// </summary>
        DECODER_OPTION_FRAME_NUM,
        /// <summary>
        /// feedback current frame belong to which IDR period
        /// int
        /// </summary>
        DECODER_OPTION_IDR_PIC_ID,
        /// <summary>
        /// feedback wether current frame mark a LTR
        /// int
        /// </summary>
        DECODER_OPTION_LTR_MARKING_FLAG,
        /// <summary>
        /// feedback frame num marked by current Frame
        /// int
        /// </summary>
        DECODER_OPTION_LTR_MARKED_FRAME_NUM,
        /// <summary>
        /// indicate decoder error concealment method
        /// ERROR_CON_IDC
        /// </summary>
        DECODER_OPTION_ERROR_CON_IDC,         
        /// <summary>
        /// int
        /// </summary>
        DECODER_OPTION_TRACE_LEVEL,
        DECODER_OPTION_TRACE_CALLBACK,        
        DECODER_OPTION_TRACE_CALLBACK_CONTEXT,

        /// <summary>
        ///feedback decoder statistics.
        ///SDecoderStatistics
        /// </summary>
        DECODER_OPTION_GET_STATISTICS,
        /// <summary>
        /// feedback decoder Sample Aspect Ratio info in Vui.
        /// SVuiSarInfo
        /// </summary>
        DECODER_OPTION_GET_SAR_INFO,
        /// <summary>
        /// get current AU profile info, only is used in GetOption
        /// </summary>
        DECODER_OPTION_PROFILE,
        /// <summary>
        /// get current AU level info,only is used in GetOption
        /// </summary>
        DECODER_OPTION_LEVEL,                 
        DECODER_OPTION_STATISTICS_LOG_INTERVAL,
        /// <summary>
        /// feedback current frame is ref pic or not. 
        /// int 0 or 1
        /// </summary>
        DECODER_OPTION_IS_REF_PIC,
        /// <summary>
        /// number of frames remaining in decoder buffer when pictures are required to re-ordered into display-order.
        /// </summary>
        DECODER_OPTION_NUM_OF_FRAMES_REMAINING_IN_BUFFER,
        /// <summary>
        /// number of decoding threads. The maximum thread count is equal or less than lesser of (cpu core counts and 16).
        /// </summary>
        DECODER_OPTION_NUM_OF_THREADS,
    }
      ;

     public enum KEY_FRAME_REQUEST_TYPE
    {
        NO_RECOVERY_REQUSET = 0,
        LTR_RECOVERY_REQUEST = 1,
        IDR_RECOVERY_REQUEST = 2,
        NO_LTR_MARKING_FEEDBACK = 3,
        LTR_MARKING_SUCCESS = 4,
        LTR_MARKING_FAILED = 5
    }
    ;

    /**
* @brief Structure for LTR recover request
*/
    [StructLayout(LayoutKind.Sequential)]
    public struct SLTRRecoverRequest
    {
        /// <summary>
        /// IDR request or LTR recovery request
        /// </summary>
        public uint uiFeedbackType;
        /// <summary>
        ///  distinguish request from different IDR
        /// </summary>
        public uint uiIDRPicId;           
        public int iLastCorrectFrameNum;
        /// <summary>
        /// specify current decoder frame_num.
        /// </summary>
        public int iCurrentFrameNum;
        /// <summary>
        /// specify the layer for recovery request
        /// </summary>
        public int iLayerId;          
    }
    ;

    /**
    * @brief Structure for LTR marking feedback
*/
    [StructLayout(LayoutKind.Sequential)]
    public struct SLTRMarkingFeedback
    {
        /// <summary>
        ///  mark failed or successful
        /// </summary>
        public uint uiFeedbackType;
        /// <summary>
        /// distinguish request from different IDR
        /// </summary>
        public uint uiIDRPicId;
        /// <summary>
        /// specify current decoder frame_num
        /// </summary>
        public int iLTRFrameNum;
        /// <summary>
        /// specify the layer for LTR marking feedback
        /// </summary>
        public int iLayerId;
    }
    ;

    /**
    * @brief Structure for LTR configuration
*/
    [StructLayout(LayoutKind.Sequential)]
    public struct SLTRConfig {
        /// <summary>
        /// 1: on, 0: off
        /// </summary>
        [MarshalAs(UnmanagedType.U1)] 
        public bool bEnableLongTermReference;
        public int iLTRRefNum;
    }
    ;
    [StructLayout(LayoutKind.Sequential)]
    public struct TagBitrateInfo
    {
        public LAYER_NUM iLayer;
        /// <summary>
        /// the maximum bitrate
        /// </summary>
        public int iBitrate;
    }
    public enum LAYER_NUM
    {
        SPATIAL_LAYER_0 = 0,
        SPATIAL_LAYER_1 = 1,
        SPATIAL_LAYER_2 = 2,
        SPATIAL_LAYER_3 = 3,
        SPATIAL_LAYER_ALL = 4
    }
    ;
    [StructLayout(LayoutKind.Sequential)]
    public struct SProfileInfo
    {
        public int iLayer;
        /// <summary>
        /// the profile info
        /// </summary>
        public EProfileIdc uiProfileIdc;
    }
    ;

    /**
    * @brief  Structure for level info in layer
    *
*/
    [StructLayout(LayoutKind.Sequential)]
    public struct SLevelInfo
    {
        public int iLayer;
        /// <summary>
        /// the level info
        /// </summary>
        public ELevelIdc uiLevelIdc;           
    }
    ;
    /**
    * @brief Structure for dilivery status
    *
*/
    [StructLayout(LayoutKind.Sequential)]
    public struct SDeliveryStatus
    {
        /// <summary>
        /// 0: the previous frame isn't delivered,1: the previous frame is delivered
        /// </summary>
        [MarshalAs(UnmanagedType.U1)]
        public bool bDeliveryFlag;
        /// <summary>
        /// the frame type that is dropped; reserved
        /// </summary>
        public int iDropFrameType;
        /// <summary>
        /// the frame size that is dropped; reserved
        /// </summary>
        public int iDropFrameSize;
    }
    ;

    /**
    * @brief The capability of decoder, for SDP negotiation
*/
    [StructLayout(LayoutKind.Sequential)]
    public struct SDecoderCapability
    {
        public int iProfileIdc;     ///< profile_idc
        public int iProfileIop;     ///< profile-iop
        public int iLevelIdc;       ///< level_idc
        public int iMaxMbps;        ///< max-mbps
        public int iMaxFs;          ///< max-fs
        public int iMaxCpb;         ///< max-cpb
        public int iMaxDpb;         ///< max-dpb
        public int iMaxBr;          ///< max-br
        [MarshalAs(UnmanagedType.U1)] 
        public bool bRedPicCap;     ///< redundant-pic-cap
    }
    ;
    [StructLayout(LayoutKind.Sequential)]

    public struct SEncoderStatistics
    {
        /// <summary>
        /// the width of encoded frame
        /// </summary>
        public uint uiWidth;
        /// <summary>
        /// the height of encoded frame
        /// </summary>
        public uint uiHeight;
        //following standard, will be 16x aligned, if there are multiple spatial, this is of the highest
        /// <summary>
        /// average_Encoding_Time
        /// </summary>
        public float fAverageFrameSpeedInMs;

        // rate control related
        /// <summary>
        /// the average frame rate in, calculate since encoding starts, supposed that the input timestamp is in unit of ms
        /// </summary>
        public float fAverageFrameRate;
        /// <summary>
        /// the frame rate in, in the last second, supposed that the input timestamp is in unit of ms (? useful for checking BR, but is it easy to calculate?
        /// </summary>
        public float fLatestFrameRate;
        /// <summary>
        /// sendrate in Bits per second, calculated within the set time-window
        /// </summary>
        public uint uiBitRate;
        /// <summary>
        /// the average QP of last encoded frame
        /// </summary>
        public uint uiAverageFrameQP;

        /// <summary>
        /// number of frames
        /// </summary>
        public uint uiInputFrameCount;
        /// <summary>
        /// number of frames
        /// </summary>
        public uint uiSkippedFrameCount;

        /// <summary>
        ///  uiResolutionChangeTimes
        /// </summary>
        public uint uiResolutionChangeTimes;
        /// <summary>
        /// number of IDR requests
        /// </summary>
        public uint uiIDRReqNum;
        /// <summary>
        /// number of actual IDRs sent
        /// </summary>
        public uint uiIDRSentNum;
        /// <summary>
        /// number of LTR sent/marked
        /// </summary>
        public uint uiLTRSentNum;

        /// <summary>
        /// Timestamp of updating the statistics
        /// </summary>
        public long iStatisticsTs;                 

        public uint iTotalEncodedBytes;
        public uint iLastStatisticsBytes;
        public uint iLastStatisticsFrameCount;
    }
    ;
    [StructLayout(LayoutKind.Sequential)]

    public struct SDecoderStatistics
    {
        /// <summary>
        /// the width of encode/decode frame
        /// </summary>
        public uint uiWidth;
        /// <summary>
        /// the height of encode/decode frame
        /// </summary>
        public uint uiHeight;
        /// <summary>
        /// average_Decoding_Time
        /// </summary>
        public float fAverageFrameSpeedInMs;
        /// <summary>
        /// actual average_Decoding_Time, including freezing pictures
        /// </summary>
        public float fActualAverageFrameSpeedInMs;
        /// <summary>
        ///  number of frames
        /// </summary>
        public uint uiDecodedFrameCount;
        /// <summary>
        /// uiResolutionChangeTimes
        /// </summary>
        public uint uiResolutionChangeTimes;
        /// <summary>
        /// number of correct IDR received
        /// </summary>
        public uint uiIDRCorrectNum;
        /// <summary>
        ///  when EC is on, the average ratio of total EC areas, can be an indicator of reconstruction quality
        /// </summary>
        public uint uiAvgEcRatio;
        /// <summary>
        /// when EC is on, the rough average ratio of propogate EC areas, can be an indicator of reconstruction quality
        /// </summary>
        public uint uiAvgEcPropRatio;
        /// <summary>
        /// number of actual unintegrity IDR or not received but eced
        /// </summary>
        public uint uiEcIDRNum;                     
        public uint uiEcFrameNum;
        /// <summary>
        /// number of whole lost IDR
        /// </summary>
        public uint uiIDRLostNum;
        /// <summary>
        /// number of freezing IDR with error (partly received), under resolution change
        /// </summary>
        public uint uiFreezingIDRNum;
        /// <summary>
        /// number of freezing non-IDR with error
        /// </summary>
        public uint uiFreezingNonIDRNum;
        /// <summary>
        /// average luma QP. default: -1, no correct frame outputted
        /// </summary>
        public int iAvgLumaQp;
        /// <summary>
        /// number of Sps Invalid report
        /// </summary>
        public int iSpsReportErrorNum;
        /// <summary>
        /// number of SubSps Invalid report
        /// </summary>
        public int iSubSpsReportErrorNum;
        /// <summary>
        /// number of Pps Invalid report
        /// </summary>
        public int iPpsReportErrorNum;
        /// <summary>
        /// number of Sps NoExist Nal
        /// </summary>
        public int iSpsNoExistNalNum;
        /// <summary>
        ///  number of SubSps NoExist Nal
        /// </summary>
        public int iSubSpsNoExistNalNum;
        /// <summary>
        /// number of Pps NoExist Nal
        /// </summary>
        public int iPpsNoExistNalNum;

        /// <summary>
        /// Profile idc in syntax
        /// </summary>
        public uint uiProfile;
        /// <summary>
        /// level idc according to Annex A-1
        /// </summary>
        public uint uiLevel;

        /// <summary>
        /// current active SPS id
        /// </summary>
        public int iCurrentActiveSpsId;              
        /// <summary>
        /// current active PPS id
        /// </summary>
        public int iCurrentActivePpsId;

        /// <summary>
        /// frame interval of statistics log
        /// </summary>
        public uint iStatisticsLogInterval;                  
    }
    ; 

    /**
* @brief Structure for sample aspect ratio (SAR) info in VUI
*/
    [StructLayout(LayoutKind.Sequential)]

    public struct SVuiSarInfo
    {
        /// <summary>
        /// SAR width
        /// </summary>
        public uint uiSarWidth;  
        /// <summary>
        /// SAR height
        /// </summary>
        public uint uiSarHeight;
        /// <summary>
        /// SAR overscan flag
        /// </summary>
        [MarshalAs(UnmanagedType.U1)]
        public  bool bOverscanAppropriateFlag;               
    }
    ;
    #endregion
}
