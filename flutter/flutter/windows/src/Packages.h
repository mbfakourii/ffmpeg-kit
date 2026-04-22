#ifndef FFMPEG_KIT_PACKAGES_H
#define FFMPEG_KIT_PACKAGES_H

#include <set>
#include <iostream>
#include <memory>
#include <string>

namespace ffmpeg_kit_flutter {

    /**
     * <p>Helper class to extract binary package information.
     */
    class Packages {
    public:

        /**
         * Returns the FFmpegKit binary package name.
         *
         * @return predicted FFmpegKit binary package name
         */
        static std::string getPackageName();

        /**
         * Returns enabled external libraries by FFmpeg.
         *
         * @return enabled external libraries
         */
        static std::shared_ptr <std::set<std::string>> getExternalLibraries();
    };

}

#endif // FFMPEG_KIT_PACKAGES_H
