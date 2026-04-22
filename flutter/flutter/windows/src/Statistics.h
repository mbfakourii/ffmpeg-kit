#ifndef FFMPEG_KIT_STATISTICS_H
#define FFMPEG_KIT_STATISTICS_H

#include <stdlib.h>
#include <cstdint>

namespace ffmpeg_kit_flutter {

    /**
     * Statistics entry for an FFmpeg execute session.
     */
    class Statistics {
    public:

        Statistics(const long sessionId, const int videoFrameNumber, const float videoFps,
                   const float videoQuality, const int64_t size, const double time,
                   const double bitrate, const double speed);

        long getSessionId();

        int getVideoFrameNumber();

        float getVideoFps();

        float getVideoQuality();

        int64_t getSize();

        double getTime();

        double getBitrate();

        double getSpeed();

    private:
        long _sessionId;
        int _videoFrameNumber;
        float _videoFps;
        float _videoQuality;
        int64_t _size;
        double _time;
        double _bitrate;
        double _speed;
    };

}

#endif // FFMPEG_KIT_STATISTICS_H
