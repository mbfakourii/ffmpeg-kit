#ifndef FFMPEG_KIT_LOG_CALLBACK_H
#define FFMPEG_KIT_LOG_CALLBACK_H

#include "Log.h"
#include <iostream>
#include <memory>
#include <functional>

namespace ffmpeg_kit_flutter {

    /**
     * <p>Callback that receives logs generated for <code>FFmpegKit</code> sessions.
     *
     * @param log log entry
     */
    typedef std::function<void(const std::shared_ptr <ffmpeg_kit_flutter::Log> log)> LogCallback;

}

#endif // FFMPEG_KIT_LOG_CALLBACK_H
