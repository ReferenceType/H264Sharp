using System.IO;
using System;
using System.Runtime.InteropServices;

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

                bool isAndroid = IsRunningOnAndroid();
                if (isAndroid) 
                {
                    switch (RuntimeInformation.ProcessArchitecture)
                    {
                        case Architecture.Arm:
                            CiscoDllName = "libopenh264-2.4.1-android-arm.8.so";
                            break;
                        case Architecture.Arm64:
                            CiscoDllName = "libopenh264-2.4.1-android-arm64.8.so";
                            break;
                       
                    }
                }
                else
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

        }

        // you can assign it youself on runtime aswell.
        public static string CiscoDllName;

        public const string WrapperDllWinx64 = "H264SharpNative-win64.dll";
        public const string WrapperDllWinx86 = "H264SharpNative-win32.dll";

        public const string WrapperDllLinuxx64 = "H264SharpNative-linux64.so";
        public const string WrapperDllLinuxx86 = "H264SharpNative-linux32.so";

        public const string WrapperDllLinuxArm64 = "H264SharpNative-linux-arm64.so";
        public const string WrapperDllLinuxArm32 = "H264SharpNative-linux-arm32.so";

        public const string WrapperDllAndroidArm64 = "H264SharpNative-android-arm64.so";
        public const string WrapperDllAndroidArm32 = "H264SharpNative-android-arm32.so";


        // Helper method to detect Android
        internal static bool IsRunningOnAndroid()
        {
            try
            {
                if (Environment.GetEnvironmentVariable("ANDROID_ROOT") != null)
                    return true;

                if (Directory.Exists("/system/app") && Directory.Exists("/system/priv-app"))
                    return true;

                // Check for Java type availability (if using Xamarin)
                Type androidBuildType = Type.GetType("Android.OS.Build, Mono.Android");
                if (androidBuildType != null)
                    return true;

                return false;
            }
            catch
            {
                return false;
            }
        }

    }

}
