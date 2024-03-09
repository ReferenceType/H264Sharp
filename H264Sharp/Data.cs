using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

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
    /// only used for short lived operations because next decode will overwite this data
    /// use with caution
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public readonly ref struct RGBImagePointer
    {
        public readonly int Width;
        public readonly int Height;
        public readonly int Stride;
        public readonly IntPtr ImageBytes;

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

    [StructLayout(LayoutKind.Sequential)]
    public unsafe readonly ref struct YUVImagePointer
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
    /// <summary>
    /// Represents storable rgb image
    /// you can pool images and reuse them.
    /// </summary>
    public class RgbImage:IDisposable
    {
        public IntPtr ImageBytes;
        public int offset;
        public int Width;
        public int Height;
        public int Stride;
        private bool disposedValue;

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

    //------------------------
    [StructLayout(LayoutKind.Sequential)]
    public struct TagEncParamBase
    {
        public EUsageType iUsageType;  ///< application type; please refer to the definition of EUsageType

        public int iPicWidth;        ///< width of picture in luminance samples (the maximum of all layers if multiple spatial layers presents)
        public int iPicHeight;       ///< height of picture in luminance samples((the maximum of all layers if multiple spatial layers presents)
        public int iTargetBitrate;   ///< target bitrate desired, in unit of bps
        public RC_MODES iRCMode;          ///< rate control mode
        public float fMaxFrameRate;    ///< maximal input frame rate

    }
    ;

    
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
    ;

    public enum EUsageType 
    {
        CAMERA_VIDEO_REAL_TIME,      ///< camera video for real-time communication
        SCREEN_CONTENT_REAL_TIME,    ///< screen content signal
        CAMERA_VIDEO_NON_REAL_TIME,
        SCREEN_CONTENT_NON_REAL_TIME,
        INPUT_CONTENT_TYPE_ALL,
    }

    public enum RC_MODES
    {
        RC_QUALITY_MODE = 0,     ///< quality mode
        RC_BITRATE_MODE = 1,     ///< bitrate mode
        RC_BUFFERBASED_MODE = 2, ///< no bitrate control,only using buffer status,adjust the video quality
        RC_TIMESTAMP_MODE = 3, //rate control based timestamp
        RC_BITRATE_MODE_POST_SKIP = 4, ///< this is in-building RC MODE, WILL BE DELETED after algorithm tuning!
        RC_OFF_MODE = -1,         ///< rate control off mode
    };
     public enum ECOMPLEXITY_MODE
    {
        LOW_COMPLEXITY = 0,              ///< the lowest compleixty,the fastest speed,
        MEDIUM_COMPLEXITY,          ///< medium complexity, medium speed,medium quality
        HIGH_COMPLEXITY             ///< high complexity, lowest speed, high quality
    }
    ;
    public enum EParameterSetStrategy
    {
        CONSTANT_ID = 0,           ///< constant id in SPS/PPS
        INCREASING_ID = 0x01,      ///< SPS/PPS id increases at each IDR
        SPS_LISTING = 0x02,       ///< using SPS in the existing list if possible
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

    class Defs
    {
        public const int SAVED_NALUNIT_NUM_TMP = ((4 * 4) + 1 + 4); ///< SPS/PPS + SEI/SSEI + PADDING_NAL
        public const int MAX_SLICES_NUM_TMP = ((128 - SAVED_NALUNIT_NUM_TMP) / 3);
    }

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
        DECODER_OPTION_PROFILE,               ///< get current AU profile info, only is used in GetOption
        DECODER_OPTION_LEVEL,                 ///< get current AU level info,only is used in GetOption
        DECODER_OPTION_STATISTICS_LOG_INTERVAL,
        /// <summary>
        /// int 0 or 1
        /// </summary>
        DECODER_OPTION_IS_REF_PIC,             ///< feedback current frame is ref pic or not
        DECODER_OPTION_NUM_OF_FRAMES_REMAINING_IN_BUFFER,  ///< number of frames remaining in decoder buffer when pictures are required to re-ordered into display-order.
        DECODER_OPTION_NUM_OF_THREADS,         ///< number of decoding threads. The maximum thread count is equal or less than lesser of (cpu core counts and 16).
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
        public uint uiFeedbackType;       ///< IDR request or LTR recovery request
        public uint uiIDRPicId;           ///< distinguish request from different IDR
        public int iLastCorrectFrameNum;
        public int iCurrentFrameNum;     ///< specify current decoder frame_num.
        public int iLayerId;           //specify the layer for recovery request
    }
    ;

    /**
    * @brief Structure for LTR marking feedback
*/
    [StructLayout(LayoutKind.Sequential)]
    public struct SLTRMarkingFeedback
    {
        public uint uiFeedbackType; ///< mark failed or successful
        public uint uiIDRPicId;     ///< distinguish request from different IDR
        public int iLTRFrameNum;   ///< specify current decoder frame_num
        public int iLayerId;        //specify the layer for LTR marking feedback
    }
    ;

    /**
    * @brief Structure for LTR configuration
*/
    [StructLayout(LayoutKind.Sequential)]
    public struct SLTRConfig {
        [MarshalAs(UnmanagedType.U1)] 
        public bool bEnableLongTermReference; ///< 1: on, 0: off
        public int iLTRRefNum;               ///< TODO: not supported to set it arbitrary yet
    }
    ;
    [StructLayout(LayoutKind.Sequential)]
    public struct TagBitrateInfo
    {
        public LAYER_NUM iLayer;
        public int iBitrate;                    ///< the maximum bitrate
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
        public EProfileIdc uiProfileIdc;        ///< the profile info
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
        public ELevelIdc uiLevelIdc;            ///< the level info
    }
    ;
    /**
    * @brief Structure for dilivery status
    *
*/
    [StructLayout(LayoutKind.Sequential)]
    public struct SDeliveryStatus
    {
        [MarshalAs(UnmanagedType.U1)]
        public bool bDeliveryFlag;              ///< 0: the previous frame isn't delivered,1: the previous frame is delivered
        public int iDropFrameType;              ///< the frame type that is dropped; reserved
        public int iDropFrameSize;              ///< the frame size that is dropped; reserved
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
        public uint uiWidth;                        ///< the width of encoded frame
        public uint uiHeight;                       ///< the height of encoded frame
        //following standard, will be 16x aligned, if there are multiple spatial, this is of the highest
        public float fAverageFrameSpeedInMs;                ///< average_Encoding_Time

        // rate control related
        public float fAverageFrameRate;                     ///< the average frame rate in, calculate since encoding starts, supposed that the input timestamp is in unit of ms
        public float fLatestFrameRate;                      ///< the frame rate in, in the last second, supposed that the input timestamp is in unit of ms (? useful for checking BR, but is it easy to calculate?
        public uint uiBitRate;                      ///< sendrate in Bits per second, calculated within the set time-window
        public uint uiAverageFrameQP;                    ///< the average QP of last encoded frame

        public uint uiInputFrameCount;              ///< number of frames
        public uint uiSkippedFrameCount;            ///< number of frames

        public uint uiResolutionChangeTimes;        ///< uiResolutionChangeTimes
        public uint uiIDRReqNum;                    ///< number of IDR requests
        public uint uiIDRSentNum;                   ///< number of actual IDRs sent
        public uint uiLTRSentNum;                   ///< number of LTR sent/marked

        public long iStatisticsTs;                  ///< Timestamp of updating the statistics

        public uint iTotalEncodedBytes;
        public uint iLastStatisticsBytes;
        public uint iLastStatisticsFrameCount;
    }
    ;
    [StructLayout(LayoutKind.Sequential)]

    public struct SDecoderStatistics
    {
        public uint uiWidth;                        ///< the width of encode/decode frame
        public uint uiHeight;                       ///< the height of encode/decode frame
        public float fAverageFrameSpeedInMs;                ///< average_Decoding_Time
        public float fActualAverageFrameSpeedInMs;          ///< actual average_Decoding_Time, including freezing pictures
        public uint uiDecodedFrameCount;            ///< number of frames
        public uint uiResolutionChangeTimes;        ///< uiResolutionChangeTimes
        public uint uiIDRCorrectNum;                ///< number of correct IDR received
        //EC on related
        public uint uiAvgEcRatio;                                ///< when EC is on, the average ratio of total EC areas, can be an indicator of reconstruction quality
        public uint uiAvgEcPropRatio;                            ///< when EC is on, the rough average ratio of propogate EC areas, can be an indicator of reconstruction quality
        public uint uiEcIDRNum;                     ///< number of actual unintegrity IDR or not received but eced
        public uint uiEcFrameNum;                   ///<
        public uint uiIDRLostNum;                   ///< number of whole lost IDR
        public uint uiFreezingIDRNum;               ///< number of freezing IDR with error (partly received), under resolution change
        public uint uiFreezingNonIDRNum;            ///< number of freezing non-IDR with error
        public int iAvgLumaQp;                              ///< average luma QP. default: -1, no correct frame outputted
        public int iSpsReportErrorNum;                      ///< number of Sps Invalid report
        public int iSubSpsReportErrorNum;                   ///< number of SubSps Invalid report
        public int iPpsReportErrorNum;                      ///< number of Pps Invalid report
        public int iSpsNoExistNalNum;                       ///< number of Sps NoExist Nal
        public int iSubSpsNoExistNalNum;                    ///< number of SubSps NoExist Nal
        public int iPpsNoExistNalNum;                       ///< number of Pps NoExist Nal

        public uint uiProfile;                ///< Profile idc in syntax
        public uint uiLevel;                  ///< level idc according to Annex A-1

        public int iCurrentActiveSpsId;                     ///< current active SPS id
        public int iCurrentActivePpsId;                     ///< current active PPS id

        public  uint iStatisticsLogInterval;                  ///< frame interval of statistics log
    }
    ; 

    /**
* @brief Structure for sample aspect ratio (SAR) info in VUI
*/
    [StructLayout(LayoutKind.Sequential)]

    public struct SVuiSarInfo
    {
        public uint uiSarWidth;                     ///< SAR width
        public uint uiSarHeight;                    ///< SAR height
        [MarshalAs(UnmanagedType.U1)]
        public  bool bOverscanAppropriateFlag;               ///< SAR overscan flag
    }
    ;
}
