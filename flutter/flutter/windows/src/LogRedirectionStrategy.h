#ifndef FFMPEG_KIT_LOG_REDIRECTION_STRATEGY_H
#define FFMPEG_KIT_LOG_REDIRECTION_STRATEGY_H

namespace ffmpeg_kit_flutter {

    enum LogRedirectionStrategy {
        LogRedirectionStrategyAlwaysPrintLogs = 0,
        LogRedirectionStrategyPrintLogsWhenNoCallbacksDefined = 1,
        LogRedirectionStrategyPrintLogsWhenGlobalCallbackNotDefined = 2,
        LogRedirectionStrategyPrintLogsWhenSessionCallbackNotDefined = 3,
        LogRedirectionStrategyNeverPrintLogs = 4
    };

}

#endif // FFMPEG_KIT_LOG_REDIRECTION_STRATEGY_H
