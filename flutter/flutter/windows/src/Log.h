#ifndef FFMPEG_KIT_LOG_H
#define FFMPEG_KIT_LOG_H

#include "Level.h"
#include <string>

namespace ffmpeg_kit_flutter {

    /**
     * <p>Log entry for an <code>FFmpegKit</code> session.
     */
    class Log {
    public:
        Log(const long sessionId, const ffmpeg_kit_flutter::Level level, const char *message);

        long getSessionId() const;

        ffmpeg_kit_flutter::Level getLevel() const;

        std::string getMessage() const;

    private:
        long _sessionId;
        ffmpeg_kit_flutter::Level _level;
        std::string _message;
    };

}

#endif // FFMPEG_KIT_LOG_H
