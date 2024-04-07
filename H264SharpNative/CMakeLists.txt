# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# CMakeList.txt : CMake project for CMakeProject1, include source and define
# project specific logic here.
#
add_definitions(-DUNICODE -D_UNICODE)
add_compile_options(-Rpass=loop-vectorize)
add_compile_options(-mavx2)
add_compile_options(-o2)


if(UNIX)
    # Set the C++ compiler only for UNIX systems (Linux and macOS)
    set(CMAKE_C_COMPILER "/usr/bin/clang-17")
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++-17")
else()
    set(CMAKE_C_COMPILER "clang-cl")
    set(CMAKE_CXX_COMPILER "clang-cl")
endif()

# Add source to this project's executable.
#add_executable (CMakeProject1 "CMakeProject1.cpp" "CMakeProject1.h")
add_library (CMakeProject1 SHARED 
"CMakeProject1.cpp"
"CMakeProject1.h"
"codec_api.h"
"codec_app_def.h"
"codec_def.h"
"codec_ver.h"
"ConverterLocal.h"
"Decoder.h"
"EncodedFrame.h"
"Encoder.h"
"ImageTypes.h"
"pch.h"
"Resource.h"
"TranscoderFactory.h"
"ConverterLocal.cpp"
"Decoder.cpp"
"dllmain.cpp"
"Encoder.cpp"
"ImageTypes.cpp"
"pch.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeProject1 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
