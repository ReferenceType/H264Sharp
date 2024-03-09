using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Runtime.InteropServices;
using System.IO;
using System.Diagnostics;

namespace OpenH264Sample
{
    [Flags]
    public enum AviMainHeaderFlags : uint
    {
        HasIndex = 0x00000010,
        MustUseIndex = 0x00000020,
        IsInterleaved = 0x00000100,
        TrustCkType = 0x00000800,
        WasCaptureFile = 0x00010000,
        Copyrighted = 0x00020000
    }
    public class AviWriter
    {
        private List<Idx1Entry> idx1List;
        private List<Idx1Entry> idx1List2;
        private RiffList moviList;
        private RiffFile riffFile;
        private Stream outputAvi;
        private RiffList hdrlList;
        private string fourCC;
        private int width;
        private float fps;
        private int height;
        private object locker = new object();

        public AviWriter(System.IO.Stream outputAvi, string fourCC, int width, int height, float fps)
        {
            // A RIFF file consists of a RIFF header followed by zero or more lists and chunks.
            // The RIFF header consists of the FOURCC of 'RIFF', the 4-byte data size, the FOURCC that identifies the data, and the data.
            // The list consists of the FOURCC of 'LIST', the 4-byte data size, the FOURCC that identifies the data, and the data.
            // A chunk consists of FOURCC that identifies data, 4-byte data size, and data.
            // FOURCC, which identifies chunk data, consists of a 2-digit stream number followed by a 2-character code (dc = video, wb = audio, tx = subtitles, etc.).
            // An AVI file is a RIFF file consisting of a FOURCC of 'AVI', two required LIST chunks ('hdrl''movi'), and an optional index chunk.
            this.fourCC = fourCC;
            this.width = width;
            this.height = height;
            this.fps = fps;
            this.outputAvi = outputAvi;

            riffFile = new RiffFile(outputAvi, "AVI ");
            // Create hdrl list with temporary number of frames
            hdrlList = riffFile.CreateList("hdrl");
            WriteAviLists(hdrlList, fourCC, width, height, fps, 0,0);
            hdrlList.Close();

            // Create movi list and add data chunks for each OnAddImage
             idx1List = new List<Idx1Entry>();
             idx1List2 = new List<Idx1Entry>();
             moviList = riffFile.CreateList("movi");
               
        }
        bool started;
        Stopwatch sw = new Stopwatch();
        public void AddImage(byte[] data, bool keyFrame)
        {
            lock (locker)
            {
                if (!started)
                {
                    started = true;
                    sw.Start();
                }
                var idx1 = WriteMoviList(moviList, "00dc", data);
                idx1.KeyFrame = keyFrame;
                idx1List.Add(idx1);
            }
           
        }

        public void AddImage(byte[] data,int offset,int count, bool keyFrame)
        {
            lock (locker)
            {
                if (!started)
                {
                    started = true;
                    sw.Start();
                }
                var idx1 = WriteMoviList(moviList, "00dc", data, offset, count);
                idx1.KeyFrame = keyFrame;
                idx1List.Add(idx1);
            }
            
        }

        public void AddAudio(byte[] data)
        {
            lock (locker)
            {
                var idx1 = WriteMoviList(moviList, "01wb", data);
                idx1List.Add(idx1);
            }
         
        }

        public void AddAudio(byte[] data,int offset,int count)
        {
            lock (locker)
            {
                var idx1 = WriteMoviList(moviList, "01wb", data, offset, count);
                idx1List.Add(idx1);
            }
          
        }

        int maxVid = 0;
        int maxAud = 0;
        public void Close()
        {
            lock(locker)
            {
                sw.Stop();
                int sec = (int)sw.ElapsedMilliseconds / 1000;
                int frameCount = idx1List.Where(x => x.aud == false).Count();
                fps = frameCount / Math.Max(1, sec);

                moviList.Close();

                WriteIdx1Chunk(riffFile, idx1List);

                int audioCnt = idx1List.Where(x => x.aud == true).Sum(x => x.Length);
                maxAud = idx1List.Where(x => x.aud == true).Max(x => x.Length);
                maxVid = idx1List.Where(x => x.aud == false).Max(x => x.Length);

                var offset = hdrlList.Begin;
                riffFile.BaseStream.Seek(offset, System.IO.SeekOrigin.Begin);
                riffFile.BaseStream.Seek(12, System.IO.SeekOrigin.Current);
                WriteAviLists(riffFile, fourCC, width, height, fps, idx1List.Count, audioCnt / 2);
                riffFile.BaseStream.Seek(0, System.IO.SeekOrigin.End);


                riffFile.Close();
                outputAvi.Dispose();
            }
          

        }
        private void WriteAviLists(RiffList hdrlList, string fourCC, int width, int height, float fps, int frames,int auds)
        {
            int streams = 2; // if there is audio 2

            // Add LIST chunk 'hdrl'

            // The 'hdrl' list starts with the AVI main header, which is included in the 'avih' chunk.
            // The main header contains global information about the entire AVI file, such as the number of streams in the file, and the width and height of the AVI sequence.
            // The main header chunk consists of an AVIMAINHEADER structure.
            {
                var chunk = hdrlList.CreateChunk("avih");
                var avih = new AVIMAINHEADER();
                avih.dwMicroSecPerFrame = 1000000U / (uint)fps; /*(uint)(1 / fps * 1000 * 1000);*/
                avih.dwMaxBytesPerSec = (uint)(maxAud + maxVid);//25000; // ffmpegと同じ値に
                avih.dwFlags = (uint)(AviMainHeaderFlags.HasIndex | AviMainHeaderFlags.IsInterleaved | AviMainHeaderFlags.TrustCkType|AviMainHeaderFlags.WasCaptureFile);
                avih.dwTotalFrames = (uint)frames;
                avih.dwStreams = (uint)streams;
                avih.dwSuggestedBufferSize = (uint)maxVid;
                avih.dwWidth = (uint)width;
                avih.dwHeight = (uint)height;

                var data = StructureToBytes(avih);
                chunk.Write(data);
                chunk.Close();
            }

            // The main header is followed by one or more 'strl' lists. A 'strl' list is required for each data stream.
            // Each 'strl' list contains information about a single stream in the file and always includes a stream header chunk ('strh') and a stream format chunk ('strf').
            // The stream header chunk ('strh') consists of an AVISTREAMHEADER structure.
            // The stream format chunk ('strf') must follow the stream header chunk.
            // The stream format chunk describes the format of the data in the stream. The data contained in this chunk varies depending on the stream type.
            // For video streams, this information is a BITMAPINFO structure that optionally contains palette information. For audio streams, this information is a WAVEFORMATEX structure.

            // 'strl' chunk for video stream
            var strl_list = hdrlList.CreateList("strl");
            {
                var chunk = strl_list.CreateChunk("strh");
                var strh = new AVISTREAMHEADER();
                strh.fccType = ToFourCC("vids");
                strh.fccHandler = ToFourCC(fourCC);
                strh.dwScale = 1000 * 1000; // fps = dwRate / dwScale. To express 30 frames per second, sometimes dwScale=33333 and dwRate=1000000, and sometimes dwScale=1 and dwRate=30.
                strh.dwRate = (int)(fps * strh.dwScale);
                strh.dwLength = frames;
                strh.dwSuggestedBufferSize = 0x100000;
                strh.dwQuality = -1;
                strh.wPriority = 0;
                var data = StructureToBytes(strh);
                chunk.Write(data);
                chunk.Close();
            }
            {
                var chunk = strl_list.CreateChunk("strf");
                var strf = new BITMAPINFOHEADER();
                strf.biWidth = width;
                strf.biHeight = height;
                strf.biBitCount = 24;
                strf.biSizeImage = strf.biHeight * ((3 * strf.biWidth + 3) / 4) * 4; // らしい
                strf.biCompression = ToFourCC(fourCC);
                strf.biSize = System.Runtime.InteropServices.Marshal.SizeOf(strf);
                strf.biPlanes = 1;

                var data = StructureToBytes(strf);
                chunk.Write(data);
                chunk.Close();
            }
            strl_list.Close();

            // AUDIO
            /*
             *  fccType               : 'auds'
                         fccHandler            : '    '
                         dwFlags               : 0x0
                         wPriority             : 0
                         wLanguage             : 0x0 undefined
                         dwInitialFrames       : 0
                         dwScale               : 1 (32000.000 Samples/Sec)
                         dwRate                : 32000
                         dwStart               : 0
                         dwLength              : 2340474
                         dwSuggestedBufferSize : 4272
                         dwQuality             : 0
                         dwSampleSize          : 4
                         rcFrame               : 0,0,0,0
             */
            var strl_list2 = hdrlList.CreateList("strl");
            {
                var chunk = strl_list.CreateChunk("strh");
                var strh = new AVISTREAMHEADER();
                strh.fccType = ToFourCC("auds");
                strh.fccHandler = 0;
                strh.dwScale = 1;
                strh.dwRate = 24000;
                strh.dwLength = auds;
                strh.dwSuggestedBufferSize = maxAud;
                strh.dwQuality = 0;
                strh.dwSampleSize = 2;
                strh.wPriority = 1;

                var data = StructureToBytes(strh);
                chunk.Write(data);
                chunk.Close();
            }
            /*
             * strf (00000012)
                         wFormatTag      : 1 PCM
                         nChannels       : 2
                         nSamplesPerSec  : 32000
                         nAvgBytesPerSec : 128000
                         nBlockAlign     : 4
                         wBitsPerSample  : 16
                         cbSize          : 0

             */
            {
                var chunk = strl_list.CreateChunk("strf");
                var strf = new WAVEFOTMATEX();
                strf.wFormatTag = 1;
                strf.nChannels = 1;
                strf.nSamplesPerSec = 24000;
                strf.nAvgBytesPerSec = 48000;
                strf.nBlockAlign = 2;
                strf.wBitsPerSample = 16;
                strf.cbSize = 0;

                var data = StructureToBytes(strf);
                chunk.Write(data);
                chunk.Close();
            }
            strl_list2.Close();
        }

        private class Idx1Entry
        {
            public string ChunkId { get; private set; }
            public int Length { get; private set; }
            public bool Padding { get; private set; }
            public bool KeyFrame { get; set; }

            public bool aud;

            public Idx1Entry(string chunkId, int length, bool padding, bool aud=false)
            {
                this.ChunkId = chunkId;
                this.Length = length;
                this.Padding = padding;
                this.aud = aud;
            }
        }

        // For example, if stream 0 contains audio, that stream's data chunk will have FOURCC '00wb'.
        // If stream 1 contains video, the data chunk for that stream will have FOURCC '01db' or '01dc'.
        private static Idx1Entry WriteMoviList(RiffList moviList, string chunkId, byte[] data)
        {
            var chunk = moviList.CreateChunk(chunkId);
            chunk.Write(data);

            // Data must be aligned on word boundaries
            // If the number of bytes is odd, write 1 byte of dummy data to align with word boundary
            int length = data.Length;
            bool padding = false;
            if (length % 2 != 0)
            {
                chunk.WriteByte(0x00); // Write a 1-byte dummy and align it to a word boundary
                padding = true;
            }

            chunk.Close();

            return new Idx1Entry(chunkId, length, padding, chunkId == "00dc" ? false:true) ;
        }

        private static Idx1Entry WriteMoviList(RiffList moviList, string chunkId, byte[] data,int offset,int count)
        {
            var chunk = moviList.CreateChunk(chunkId);
            chunk.Write(data,offset,count);

            // Data must be aligned on word boundaries
            // If the number of bytes is odd, write 1 byte of dummy data to align with word boundary
            int length = count;
            bool padding = false;
            if (length % 2 != 0)
            {
                chunk.WriteByte(0x00); // Write a 1-byte dummy and align it to a word boundary
                padding = true;
            }

            chunk.Close();

            return new Idx1Entry(chunkId, length, padding, chunkId == "00dc" ? false : true);
        }

        // The index contains a list of data chunks and their positions within the file.
        // The index consists of an AVIOLDINDEX structure, containing an entry for each chunk of data.
        // If the file contains an index, set the AVIF_HASINDEX flag in the dwFlags member of the AVIMAINHEADER structure.
        private static void WriteIdx1Chunk(RiffFile riff, List<Idx1Entry> IndexList)
        {
            const int AVIIF_KEYFRAME = 0x00000010; // 前後のフレームの情報なしにこのフレームの完全な情報を含んでいる
            var chunk = riff.CreateChunk("idx1");

            int offset = 4;
            foreach (var item in IndexList)
            {
                int length = item.Length;

                chunk.Write(ToFourCC(item.ChunkId));
                chunk.Write(item.KeyFrame? AVIIF_KEYFRAME: 0x00);
                chunk.Write(offset);
                chunk.Write(length);

                offset += 8 + length; // 8は多分00dcとﾃﾞｰﾀｻｲｽﾞ
                if (item.Padding) offset += 1;
            }

            chunk.Close();
        }

        private static int ToFourCC(string fourCC)
        {
            if (fourCC.Length != 4) throw new ArgumentException("must be 4 characters long.", "fourCC");
            return ((int)fourCC[3]) << 24 | ((int)fourCC[2]) << 16 | ((int)fourCC[1]) << 8 | ((int)fourCC[0]);
        }

        #region "Struct Marshalling"

        private static byte[] StructureToBytes<T>(T st) where T : struct
        {
            int size = System.Runtime.InteropServices.Marshal.SizeOf(st);
            IntPtr ptr = System.Runtime.InteropServices.Marshal.AllocHGlobal(size);
            System.Runtime.InteropServices.Marshal.StructureToPtr(st, ptr, false);

            byte[] data = new byte[size];
            System.Runtime.InteropServices.Marshal.Copy(ptr, data, 0, size);

            System.Runtime.InteropServices.Marshal.FreeHGlobal(ptr);
            return data;
        }
        
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct AVIMAINHEADER
        {
            public UInt32 dwMicroSecPerFrame;  // only used with AVICOMRPESSF_KEYFRAMES
            public UInt32 dwMaxBytesPerSec;
            public UInt32 dwPaddingGranularity; // only used with AVICOMPRESSF_DATARATE
            public UInt32 dwFlags;
            public UInt32 dwTotalFrames;
            public UInt32 dwInitialFrames;
            public UInt32 dwStreams;
            public UInt32 dwSuggestedBufferSize;
            public UInt32 dwWidth;
            public UInt32 dwHeight;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
            public UInt32[] dwReserved;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct RECT
        {
            public Int16 left;
            public Int16 top;
            public Int16 right;
            public Int16 bottom;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct AVISTREAMHEADER
        {
            public Int32 fccType;
            public Int32 fccHandler;
            public Int32 dwFlags;
            public Int16 wPriority;
            public Int16 wLanguage;
            public Int32 dwInitialFrames;
            public Int32 dwScale;
            public Int32 dwRate;
            public Int32 dwStart;
            public Int32 dwLength;
            public Int32 dwSuggestedBufferSize;
            public Int32 dwQuality;
            public Int32 dwSampleSize;
            public RECT rcFrame;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct BITMAPINFOHEADER
        {
            public Int32 biSize;
            public Int32 biWidth;
            public Int32 biHeight;
            public Int16 biPlanes;
            public Int16 biBitCount;
            public Int32 biCompression;
            public Int32 biSizeImage;
            public Int32 biXPelsPerMeter;
            public Int32 biYPelsPerMeter;
            public Int32 biClrUsed;
            public Int32 biClrImportant;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        private struct WAVEFOTMATEX
        {
            public Int16 wFormatTag;
            public Int16 nChannels;
            public Int32 nSamplesPerSec;
            public Int32 nAvgBytesPerSec;
            public Int16 nBlockAlign;
            public Int16 wBitsPerSample;
            public Int16 cbSize;
        }

        #endregion
    }
}
