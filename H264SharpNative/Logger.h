#pragma once
#include <iostream>

#if !defined(__ANDROID__) && !defined(__APPLE__)
#define PLATFORM_DESKTOP
#endif

class Logger {
public:
    Logger(const char* = nullptr) {} // Tag is ignored

    template<typename T>
    Logger& operator<<(const T& value) {
#ifdef PLATFORM_DESKTOP
        std::cout << value; 
#endif
        return *this; 
    }

    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
#ifdef PLATFORM_DESKTOP
        std::cout << manip; 
#endif
        return *this;
    }
};

extern Logger Log;