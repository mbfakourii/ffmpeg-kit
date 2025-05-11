#ifndef FFMPEG_KIT_MEDIA_INFORMATION_PARSER_H
#define FFMPEG_KIT_MEDIA_INFORMATION_PARSER_H

#include "MediaInformation.h"
#include <memory>

namespace ffmpeg_kit_flutter {

    /**
     * A parser that constructs MediaInformation from FFprobe's json output.
     */
    class MediaInformationJsonParser {
    public:

        /**
         * Extracts <code>MediaInformation</code> from the given FFprobe json output.
         *
         * @param ffprobeJsonOutput FFprobe json output
         * @return created MediaInformation instance of nullptr if a parsing error occurs
         */
        static std::shared_ptr <ffmpeg_kit_flutter::MediaInformation>
        from(const std::string &ffprobeJsonOutput);

        /**
         * Extracts <code>MediaInformation</code> from the given FFprobe json output. If a parsing error occurs an
         * std::exception is thrown.
         *
         * @param ffprobeJsonOutput FFprobe json output
         * @return created MediaInformation instance
         */
        static std::shared_ptr <ffmpeg_kit_flutter::MediaInformation>
        fromWithError(const std::string &ffprobeJsonOutput);

    };

}

#endif // FFMPEG_KIT_MEDIA_INFORMATION_PARSER_H
