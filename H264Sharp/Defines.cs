//#define OS_LINUX
//#undef OS_WINDOWS
namespace H264Sharp
{
    public class Defines
    {
#if OS_WINDOWS
        public static string CiscoDllName64bit = "openh264-2.4.1-win64.dll";
        public static string CiscoDllName32bit = "openh264-2.4.1-win32.dll";

        public const string WrapperDllName64bit = "H264SharpNative-win64.dll";
        public const string WrapperDllName32bit = "H264SharpNative-win32.dll";
#elif OS_LINUX
        public static string CiscoDllName64bit = "./libopenh264-2.4.1-linux64.7.so";
        public static string CiscoDllName32bit = "./libopenh264-2.4.1-linux32.7.so";

        public const string WrapperDllName64bit = "H264SharpNative-linux64.so";
        public const string WrapperDllName32bit = "H264SharpNative-linux32.so";
#elif OS_FREEBSD
  // FreeBSD-specific dll
#elif OS_MAC
  // Mac-specific dll
#endif


    }
}
