using System;
using System.Runtime.InteropServices;

namespace H264Sharp
{
    public static class RuntimeScanApi
    {
        /// <summary>
        /// Determines whether the current process is a 64-bit process.
        /// </summary>
        /// <returns>true if the process is 64-bit; otherwise, false.</returns>
        public static readonly bool Is64BitProcess = Environment.Is64BitProcess;

        /// <summary>
        /// Indicates the operating system on which the application is running
        /// </summary>
        public static readonly OperatingSystem OperatingSystem = GetOperatingSystem();

        private static OperatingSystem GetOperatingSystem()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                return OperatingSystem.Windows;
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
                return OperatingSystem.Linux;
            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                return OperatingSystem.OSX;
            return OperatingSystem.Unknown;
        }
    }

    public enum OperatingSystem
    {
        Unknown,
        Windows,
        Linux,
        OSX
    }
}
