#ifndef FFMPEG_KIT_SIGNAL_H
#define FFMPEG_KIT_SIGNAL_H

namespace ffmpeg_kit_flutter {

    enum Signal {
        SignalInt = 2,
        SignalQuit = 3,
        SignalPipe = 13,
        SignalTerm = 15,
        SignalXcpu = 24
    };

}

#endif // FFMPEG_KIT_SIGNAL_H
