# This CMake is written for linux
# LLVM Clang 17 is recommended for linux.
#wget https://apt.llvm.org/llvm.sh
#./llvm.sh 17
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("OpenH264Wrapper2")


add_definitions(-DUNICODE -D_UNICODE)
add_compile_options(-Rpass=loop-vectorize)
# change this if your target doesnt support avx2 
add_compile_options(-mavx2)
add_compile_options(-o2)


if(ANDROID)
    # Set the correct Android NDK paths
    set(CMAKE_TOOLCHAIN_FILE "C:/Microsoft/AndroidNDK/android-ndk-r23c/build/cmake/android.toolchain.cmake" CACHE STRING "" FORCE)
    set(CMAKE_ANDROID_NDK "C:/Microsoft/AndroidNDK/android-ndk-r23c")

    set(CMAKE_ANDROID_ARCH_ABI "arm64-v8a" CACHE STRING "" FORCE)
    set(CMAKE_ANDROID_API 21 CACHE STRING "" FORCE)
    set(CMAKE_ANDROID_STL_TYPE "c++_shared" CACHE STRING "" FORCE)

    # DO NOT manually set the compiler—let CMake handle it via toolchain file
    unset(CMAKE_C_COMPILER CACHE)
    unset(CMAKE_CXX_COMPILER CACHE)
   

elseif(UNIX)
    set(CMAKE_C_COMPILER "/usr/bin/clang-17")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++-17")
else()
    set(CMAKE_C_COMPILER "clang-cl")
    set(CMAKE_CXX_COMPILER "clang-cl") 
endif()

add_library (CMakeProject1 SHARED 
"codec_api.h"
"codec_app_def.h"
"codec_def.h"
"codec_ver.h"
"Converter.h"
"Decoder.h"
"EncodedFrame.h"
"Encoder.h"
"ImageTypes.h"
"pch.h"
"Resource.h"
"Exports.h"
"ThreadPool.h"
"Yuv2Rgb.h"
"Rgb2Yuv.h"
"AVX2Common.h"
"Downscale.h"
"Logger.h"
"Converter.cpp"
"Decoder.cpp" 
"dllmain.cpp"
"Encoder.cpp"
"pch.cpp"
"Yuv2Rgb.cpp" 
"Yuv2RgbSSE2.cpp"
"Yuv2RgbNEON.cpp" 
"Yuv2RgbAVX2.cpp"
"Rgb2YuvSSE.cpp" 
"Rgb2YuvAVX2.cpp" 
"ThreadPool.cpp"
"Rgb2YuvNEON.cpp"
"YuvNV12toYV12.cpp"
"Rgb2Yuv.cpp") 

if (CMAKE_VERSION VERSION_GREATER 3.12) 
  set_property(TARGET CMakeProject1 PROPERTY CXX_STANDARD 17)
endif() 
#output name  
#H264SharpNative-linux64.so 
#H264SharpNative-linux64.so


if(ANDROID)
set_target_properties(CMakeProject1 PROPERTIES PREFIX "")
set_target_properties(CMakeProject1 PROPERTIES OUTPUT_NAME "H264SharpNative-android-arm64")
set_target_properties(CMakeProject1 PROPERTIES SUFFIX ".so")

elseif(UNIX)
set_target_properties(CMakeProject1 PROPERTIES PREFIX "")
set_target_properties(CMakeProject1 PROPERTIES OUTPUT_NAME "H264SharpNative-linux-arm64") 
set_target_properties(CMakeProject1 PROPERTIES SUFFIX ".so")
else()
set_target_properties(CMakeProject1 PROPERTIES PREFIX "")
set_target_properties(CMakeProject1 PROPERTIES OUTPUT_NAME "H264SharpNative-win64")
set_target_properties(CMakeProject1 PROPERTIES SUFFIX ".dll")
endif()

# 32 bit linux
#set_target_properties(CMakeProject1 PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

# arm-32
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --target=arm-linux-gnueabihf")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --target=arm-linux-gnueabihf") 
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --target=arm-linux-gnueabihf")

#req for arm32 
#sudo apt install gcc-arm-linux-gnueabihf libc6-dev-armhf-cross
#sudo apt install build-essential gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf libc6-dev-armhf-cross