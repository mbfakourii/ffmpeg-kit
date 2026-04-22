#include "Log.h"

ffmpeg_kit_flutter::Log::Log(const long sessionId, const ffmpeg_kit_flutter::Level level,
                             const char *message) : _sessionId{sessionId}, _level{level},
                                                    _message{message} {
}

long ffmpeg_kit_flutter::Log::getSessionId() const {
    return _sessionId;
}

ffmpeg_kit_flutter::Level ffmpeg_kit_flutter::Log::getLevel() const {
    return _level;
}

std::string ffmpeg_kit_flutter::Log::getMessage() const {
    return _message;
}
