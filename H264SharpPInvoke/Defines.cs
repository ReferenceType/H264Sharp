using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace H264PInvoke
{
    public class Defines
    {
#if OS_WINDOWS
        public static string CiscoDllName64bit = "openh264-2.4.0-win64.dll";
        public static string CiscoDllName32bit = "openh264-2.4.0-win32.dll";

        public const string WrapperDllName64bit = "H264SharpNative-win64.dll";
        public const string WrapperDllName32bit = "H264SharpNative-win32.dll";
#elif OS_LINUX
  // Linux-specific dll
#elif OS_FREEBSD
  // FreeBSD-specific dll
#elif OS_MAC
  // Mac-specific dll
#endif


    }
}
