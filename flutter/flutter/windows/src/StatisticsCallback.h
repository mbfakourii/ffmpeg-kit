#ifndef FFMPEG_KIT_STATISTICS_CALLBACK_H
#define FFMPEG_KIT_STATISTICS_CALLBACK_H

#include "Statistics.h"
#include <iostream>
#include <memory>
#include <functional>

namespace ffmpeg_kit_flutter {

    /**
     * <p>Callback that receives statistics generated for <code>FFmpegKit</code> sessions.
     *
     * @param statistics statistics entry
     */
    typedef std::function<void(
            const std::shared_ptr <ffmpeg_kit_flutter::Statistics> statistics)> StatisticsCallback;

}

#endif // FFMPEG_KIT_STATISTICS_CALLBACK_H
