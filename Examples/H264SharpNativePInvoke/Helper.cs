using OpenCvSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace H264SharpNativePInvoke
{
    class Config
    {
        public int NumIterations { get; set; } = 1000;
        public int EnableCustomThreadPool { get; set; } = 1;
        public int Numthreads { get; set; } = 1;
        public int EnableSSE { get; set; } = 1;
        public int EnableAvx2 { get; set; } = 1;

        public void Print()
        {
            Console.WriteLine($"NumIterations: {NumIterations}");
            Console.WriteLine($"EnableCustomThreadPool: {EnableCustomThreadPool}");
            Console.WriteLine($"Numthreads: {Numthreads}");
            Console.WriteLine($"EnableSSE: {EnableSSE}");
            Console.WriteLine($"EnableAvx2: {EnableAvx2}");
            Console.WriteLine();
        }
    }
    class Helper
    {
        public static void ConvertI420ToNV12(IntPtr ImageBytes, int width, int height, IntPtr NV12Buffer)
        {
            int ySize = width * height;
            int uvSize = ySize / 4;

            // Y plane is the same in both formats
            unsafe
            {
                byte* src = (byte*)ImageBytes.ToPointer();
                byte* dst = (byte*)NV12Buffer.ToPointer();

                // Copy Y plane
                for (int i = 0; i < ySize; i++)
                {
                    dst[i] = src[i];
                }

                // Interleave U and V planes into UV plane
                byte* uSrc = src + ySize;
                byte* vSrc = uSrc + uvSize;
                byte* uvDst = dst + ySize;

                for (int i = 0; i < uvSize; i++)
                {
                    uvDst[2 * i] = uSrc[i];     // U component
                    uvDst[2 * i + 1] = vSrc[i]; // V component
                }
            }
        }

        public static void SaveRawRGBFrames(string videoPath, string outputFile)
        {
            //string tempOutputFile = outputFile + ".temp";
            using (var capture = new VideoCapture())
            using (var frame = new Mat())
            using (var fs = new FileStream(outputFile, FileMode.Create, FileAccess.Write))
            {
                int width = 1280;
                int height = 720;
                var targetSize = new OpenCvSharp.Size(width, height); // 1080p resolution
                int frameCount = 30; // Number of frames to save


                byte[] header = BitConverter.GetBytes(width)
                    .Concat(BitConverter.GetBytes(height))
                    .Concat(BitConverter.GetBytes(frameCount))
                    .ToArray();

                fs.Write(header, 0, header.Length);
                if (!capture.Open(videoPath))
                {
                    throw new IOException($"Could not open video file: {videoPath}");
                }


                int skips = 10;
                int savedFrames = 0;
                while (capture.Read(frame) && savedFrames < frameCount)
                {
                    if (skips > 0)
                    {
                        skips--;
                        continue;
                    }
                    using (var rgbFrame = frame.Resize(targetSize))
                    {
                        byte[] data = new byte[rgbFrame.Total() * rgbFrame.ElemSize()];
                        Marshal.Copy(rgbFrame.Data, data, 0, data.Length);
                        fs.Write(data, 0, data.Length);
                        savedFrames++;
                        Cv2.ImShow("Frame", rgbFrame);
                        Cv2.WaitKey(1);
                    }
                }
            }
            //using (FileStream zipToCreate = new FileStream(outputFile, FileMode.Create))
            //   {
            //       using (ZipArchive archive = new ZipArchive(zipToCreate, ZipArchiveMode.Create))
            //       {
            //           string fileName = Path.GetFileName(outputFile);
            //           ZipArchiveEntry entry = archive.CreateEntry(fileName);

            //           using (Stream entryStream = entry.Open())
            //           using (FileStream fileToCompress = new FileStream(tempOutputFile, FileMode.Open))
            //           {
            //               fileToCompress.CopyTo(entryStream);
            //           }
            //       }
            //   }

            //   // Delete the temporary file
            //   File.Delete(tempOutputFile);
        }


    }
}
