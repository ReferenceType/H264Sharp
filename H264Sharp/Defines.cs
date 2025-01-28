//#define OS_LINUX_ARM
//#undef OS_WINDOWS
using System.Runtime.InteropServices;
using System;

namespace H264Sharp
{
    public class Defines
    {
        internal readonly static NativeBindings Native = new NativeBindings();

        static Defines()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                switch (RuntimeInformation.ProcessArchitecture)
                {
                    case Architecture.X86:
                        CiscoDllName = "openh264-2.4.1-win32.dll";
                        break;
                    case Architecture.X64:
                        CiscoDllName = "openh264-2.4.1-win64.dll";
                        break;
                }

            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                switch (RuntimeInformation.ProcessArchitecture)
                {
                    case Architecture.X86:
                        CiscoDllName = "./libopenh264-2.4.1-linux32.7.so";
                        break;
                    case Architecture.X64:
                        CiscoDllName = "./libopenh264-2.4.1-linux64.7.so";
                        break;
                    case Architecture.Arm:
                        CiscoDllName = "./libopenh264-2.4.1-linux-arm.7.so";
                        break;
                    case Architecture.Arm64:
                        CiscoDllName = "./libopenh264-2.4.1-linux-arm64.7.so";
                        break;
                }
            }

        }

        public static string CiscoDllName;

        public const string WrapperDllWinx86 = "H264SharpNative-win32.dll";
        public const string WrapperDllWinx64 = "H264SharpNative-win64.dll";

        public const string WrapperDllLinuxx64 = "H264SharpNative-linux64.so";
        public const string WrapperDllLinuxx86 = "H264SharpNative-linux32.so";

        public const string WrapperDllLinuxArm64 = "H264SharpNative-linux-arm64.so";
        public const string WrapperDllLinuxArm32 = "H264SharpNative-linux-arm32.so";

    }

}
