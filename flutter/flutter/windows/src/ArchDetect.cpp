#include "ArchDetect.h"

extern void *ffmpegKitInitialize();

const void *_archDetectInitializer{ffmpegKitInitialize()};

std::string ffmpeg_kit_flutter::ArchDetect::getArch() {
#ifdef FFMPEG_KIT_ARM64
    return "arm64";
#elif FFMPEG_KIT_I386
    return "i386";
#elif FFMPEG_KIT_X86_64
    return "x86_64";
#elif defined(_M_X64) || defined(__amd64__) || defined(__x86_64__)
    return "x86_64";
#elif defined(_M_IX86) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
    return "i386";
#else
    return "";
#endif
}
