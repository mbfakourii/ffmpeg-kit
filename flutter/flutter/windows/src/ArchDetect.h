#ifndef FFMPEG_KIT_ARCH_DETECT_H
#define FFMPEG_KIT_ARCH_DETECT_H

#include <string>

namespace ffmpeg_kit_flutter {

    /**
     * Detects the running architecture.
     */
    class ArchDetect {
    public:

        /**
         * Returns architecture name loaded.
         *
         * @return architecture name loaded
         */
        static std::string getArch();
    };

}

#endif // FFMPEG_KIT_ARCH_DETECT_H
