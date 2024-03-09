using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace OpenH264Sample
{
    // https://msdn.microsoft.com/ja-jp/library/cc352264.aspx
    // A RIFF file consists of a RIFF header followed by zero or more lists and chunks.
    // A RIFF header is the same as a list, except that it must be at the root of the chunk hierarchy.
    // The list consists of the FOURCC of 'LIST', the 4-byte data size, the FOURCC that identifies the data, and the data.
    // A chunk consists of FOURCC that identifies data, 4-byte data size, and data.
    // The beginning of the chunk is aligned to word boundaries. Therefore, if the length of the data is odd, the end is padded with 0 (0 is added to make it even).
    class RiffFile : RiffList
    {
       
        public System.IO.Stream BaseStream { get; private set; }

        public RiffFile(System.IO.Stream output, string fourCC) : base(output, "RIFF", fourCC)
        {
            BaseStream = output;
        }

       
        public override void Close()
        {
            base.Close();
            BaseStream.Close();
        }
    }

    class RiffList : RiffBase
    {
        public string Id { get; private set; }

        private System.IO.BinaryWriter Writer;

        public RiffList(System.IO.Stream output, string fourCC, string id) : base(output, fourCC)
        {
            Writer = new System.IO.BinaryWriter(output);
            Writer.Write(ToFourCC(id));
            this.Id = id;
        }

        public RiffList CreateList(string fourCC)
        {
            return new RiffList(Writer.BaseStream, "LIST", fourCC);
        }

        public RiffChunk CreateChunk(string fourCC)
        {
            return new RiffChunk(Writer.BaseStream, fourCC);
        }
    }

    class RiffChunk : RiffBase
    {
        private System.IO.BinaryWriter BinWriter;
        public RiffChunk(System.IO.Stream output, string fourCC) : base(output, fourCC)
        {
            BinWriter = new System.IO.BinaryWriter(output);
        }

        public void Write(byte[] data)
        {
            BinWriter.BaseStream.Write(data, 0, data.Length);
        }
        public void Write(byte[] data,int offset,int count)
        {
            BinWriter.BaseStream.Write(data, offset, count);
        }
        public void Write(int value)
        {
            BinWriter.Write(value);
        }
        public void WriteByte(byte value)
        {
            BinWriter.Write(value);
        }
    }

    class RiffBase : IDisposable
    {
        
        public long Begin { get; private set; }
        public long SizeBegin { get; private set; }
        public long DataBegin { get; private set; }

        public uint ChunkSize { get; private set; }

        public string FourCC { get; private set; }
        internal static int ToFourCC(string fourCC)
        {
            if (fourCC.Length != 4) throw new ArgumentException("fourCCは4文字である必要があります。", "fourCC");
            return ((int)fourCC[3]) << 24 | ((int)fourCC[2]) << 16 | ((int)fourCC[1]) << 8 | ((int)fourCC[0]);
        }
        internal static string ToFourCC(int fourCC)
        {
            var bytes = new byte[4];
            bytes[0] = (byte)(fourCC >> 0 & 0xFF);
            bytes[1] = (byte)(fourCC >> 8 & 0xFF);
            bytes[2] = (byte)(fourCC >> 16 & 0xFF);
            bytes[3] = (byte)(fourCC >> 24 & 0xFF);
            return System.Text.ASCIIEncoding.ASCII.GetString(bytes);
        }
        private BinaryWriter writer;
        public RiffBase(System.IO.Stream output, string fourCC)
        {
            this.FourCC = fourCC;
            this.Begin = output.Position;

            writer = new System.IO.BinaryWriter(output);
            writer.Write(ToFourCC(fourCC));
            this.SizeBegin = output.Position;

            writer.Write(0u);
            this.DataBegin = output.Position;
        }
       
        public virtual void Close() 
        {
            var dataEnd = writer.BaseStream.Position;
            ChunkSize = (uint)(dataEnd - DataBegin);
            writer.BaseStream.Position = SizeBegin;
            writer.Write(ChunkSize);
            writer.BaseStream.Position = dataEnd;
        }

        #region IDisposable Support
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                Close();
            }
        }

        ~RiffBase() { Dispose(false); }
        #endregion
    }
}
