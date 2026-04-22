#ifndef FFMPEG_KIT_LEVEL_H
#define FFMPEG_KIT_LEVEL_H

namespace ffmpeg_kit_flutter {

    /**
     * <p>Enumeration type for log levels.
     */
    enum Level {

        /**
         * This log level is defined by FFmpegKit. It is used to specify logs printed to stderr by
         * FFmpeg. Logs that has this level are not filtered and always redirected.
         */
        LevelAVLogStdErr = -16,

        /**
         * Print no output.
         */
        LevelAVLogQuiet = -8,

        /**
         * Something went really wrong and we will crash now.
         */
        LevelAVLogPanic = 0,

        /**
         * Something went wrong and recovery is not possible.
         * For example, no header was found for a format which depends
         * on headers or an illegal combination of parameters is used.
         */
        LevelAVLogFatal = 8,

        /**
         * Something went wrong and cannot losslessly be recovered.
         * However, not all future data is affected.
         */
        LevelAVLogError = 16,

        /**
         * Something somehow does not look correct. This may or may not
         * lead to problems. An example would be the use of '-vstrict -2'.
         */
        LevelAVLogWarning = 24,

        /**
         * Standard information.
         */
        LevelAVLogInfo = 32,

        /**
         * Detailed information.
         */
        LevelAVLogVerbose = 40,

        /**
         * Stuff which is only useful for libav* developers.
         */
        LevelAVLogDebug = 48,

        /**
         * Extremely verbose debugging, useful for libav* development.
         */
        LevelAVLogTrace = 56

    };

}

#endif // FFMPEG_KIT_LEVEL_H
