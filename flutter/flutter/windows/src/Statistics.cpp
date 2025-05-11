#include "Statistics.h"

ffmpeg_kit_flutter::Statistics::Statistics(const long sessionId, const int videoFrameNumber,
                                           const float videoFps, const float videoQuality,
                                           const int64_t size, const double time,
                                           const double bitrate, const double speed) :
        _sessionId{sessionId}, _videoFrameNumber{videoFrameNumber}, _videoFps{videoFps},
        _videoQuality{videoQuality}, _size{size}, _time{time}, _bitrate{bitrate}, _speed{speed} {
}

long ffmpeg_kit_flutter::Statistics::getSessionId() {
    return _sessionId;
}

int ffmpeg_kit_flutter::Statistics::getVideoFrameNumber() {
    return _videoFrameNumber;
}

float ffmpeg_kit_flutter::Statistics::getVideoFps() {
    return _videoFps;
}

float ffmpeg_kit_flutter::Statistics::getVideoQuality() {
    return _videoQuality;
}

int64_t ffmpeg_kit_flutter::Statistics::getSize() {
    return _size;
}

double ffmpeg_kit_flutter::Statistics::getTime() {
    return _time;
}

double ffmpeg_kit_flutter::Statistics::getBitrate() {
    return _bitrate;
}

double ffmpeg_kit_flutter::Statistics::getSpeed() {
    return _speed;
}
