#include "FFmpegSession.h"
#include "FFmpegKitConfig.h"
#include "LogCallback.h"
#include "StatisticsCallback.h"

extern void addSessionToSessionHistory(const std::shared_ptr <ffmpeg_kit_flutter::Session> session);

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegSession::create(const std::list <std::string> &arguments) {
    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> session = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
            std::make_shared<ffmpeg_kit_flutter::FFmpegSession::PublicFFmpegSession>(arguments,
                                                                                     nullptr,
                                                                                     nullptr,
                                                                                     nullptr,
                                                                                     ffmpeg_kit_flutter::FFmpegKitConfig::getLogRedirectionStrategy()));
    addSessionToSessionHistory(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegSession::create(const std::list <std::string> &arguments,
                                          FFmpegSessionCompleteCallback completeCallback) {
    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> session = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
            std::make_shared<ffmpeg_kit_flutter::FFmpegSession::PublicFFmpegSession>(arguments,
                                                                                     completeCallback,
                                                                                     nullptr,
                                                                                     nullptr,
                                                                                     ffmpeg_kit_flutter::FFmpegKitConfig::getLogRedirectionStrategy()));
    addSessionToSessionHistory(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegSession::create(const std::list <std::string> &arguments,
                                          FFmpegSessionCompleteCallback completeCallback,
                                          ffmpeg_kit_flutter::LogCallback logCallback,
                                          ffmpeg_kit_flutter::StatisticsCallback statisticsCallback) {
    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> session = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
            std::make_shared<ffmpeg_kit_flutter::FFmpegSession::PublicFFmpegSession>(arguments,
                                                                                     completeCallback,
                                                                                     logCallback,
                                                                                     statisticsCallback,
                                                                                     ffmpeg_kit_flutter::FFmpegKitConfig::getLogRedirectionStrategy()));
    addSessionToSessionHistory(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegSession::create(const std::list <std::string> &arguments,
                                          FFmpegSessionCompleteCallback completeCallback,
                                          ffmpeg_kit_flutter::LogCallback logCallback,
                                          ffmpeg_kit_flutter::StatisticsCallback statisticsCallback,
                                          LogRedirectionStrategy logRedirectionStrategy) {
    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> session = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
            std::make_shared<ffmpeg_kit_flutter::FFmpegSession::PublicFFmpegSession>(arguments,
                                                                                     completeCallback,
                                                                                     logCallback,
                                                                                     statisticsCallback,
                                                                                     logRedirectionStrategy));
    addSessionToSessionHistory(session);
    return session;
}

struct ffmpeg_kit_flutter::FFmpegSession::PublicFFmpegSession
        : public ffmpeg_kit_flutter::FFmpegSession {
    PublicFFmpegSession(const std::list <std::string> &arguments,
                        FFmpegSessionCompleteCallback completeCallback,
                        ffmpeg_kit_flutter::LogCallback logCallback,
                        ffmpeg_kit_flutter::StatisticsCallback statisticsCallback,
                        LogRedirectionStrategy logRedirectionStrategy) :
            FFmpegSession(arguments, completeCallback, logCallback, statisticsCallback,
                          logRedirectionStrategy) {
    }
};

ffmpeg_kit_flutter::FFmpegSession::FFmpegSession(const std::list <std::string> &arguments,
                                                 FFmpegSessionCompleteCallback completeCallback,
                                                 ffmpeg_kit_flutter::LogCallback logCallback,
                                                 ffmpeg_kit_flutter::StatisticsCallback statisticsCallback,
                                                 LogRedirectionStrategy logRedirectionStrategy) :
        ffmpeg_kit_flutter::AbstractSession(arguments, logCallback, logRedirectionStrategy),
        _completeCallback{completeCallback}, _statisticsCallback{statisticsCallback}, _statistics{
        std::make_shared < std::list < std::shared_ptr < ffmpeg_kit_flutter::Statistics>>>()} {
}

ffmpeg_kit_flutter::StatisticsCallback ffmpeg_kit_flutter::FFmpegSession::getStatisticsCallback() {
    return _statisticsCallback;
}

ffmpeg_kit_flutter::FFmpegSessionCompleteCallback
ffmpeg_kit_flutter::FFmpegSession::getCompleteCallback() {
    return _completeCallback;
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Statistics>>>

ffmpeg_kit_flutter::FFmpegSession::getAllStatisticsWithTimeout(const int waitTimeout) {
    this->waitForAsynchronousMessagesInTransmit(waitTimeout);

    if (this->thereAreAsynchronousMessagesInTransmit()) {
        std::cout
                << "getAllStatisticsWithTimeout was called to return all statistics but there are still statistics being transmitted for session id "
                << this->getSessionId() << "." << std::endl;
    }

    return this->getStatistics();
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Statistics>>>

ffmpeg_kit_flutter::FFmpegSession::getAllStatistics() {
    return this->getAllStatisticsWithTimeout(
            ffmpeg_kit_flutter::AbstractSession::DefaultTimeoutForAsynchronousMessagesInTransmit);
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Statistics>>>

ffmpeg_kit_flutter::FFmpegSession::getStatistics() {
    return _statistics;
}

std::shared_ptr <ffmpeg_kit_flutter::Statistics>
ffmpeg_kit_flutter::FFmpegSession::getLastReceivedStatistics() {
    if (_statistics->size() > 0) {
        return _statistics->back();
    } else {
        return nullptr;
    }
}

void ffmpeg_kit_flutter::FFmpegSession::addStatistics(
        const std::shared_ptr <ffmpeg_kit_flutter::Statistics> statistics) {
    _statistics->push_back(statistics);
}

bool ffmpeg_kit_flutter::FFmpegSession::isFFmpeg() const {
    return true;
}

bool ffmpeg_kit_flutter::FFmpegSession::isFFprobe() const {
    return false;
}

bool ffmpeg_kit_flutter::FFmpegSession::isMediaInformation() const {
    return false;
}
