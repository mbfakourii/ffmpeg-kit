#ifndef FFMPEG_KIT_SESSION_STATE_H
#define FFMPEG_KIT_SESSION_STATE_H

namespace ffmpeg_kit_flutter {

    enum SessionState {
        SessionStateCreated = 0,
        SessionStateRunning = 1,
        SessionStateFailed = 2,
        SessionStateCompleted = 3
    };

}

#endif // FFMPEG_KIT_SESSION_STATE_H
