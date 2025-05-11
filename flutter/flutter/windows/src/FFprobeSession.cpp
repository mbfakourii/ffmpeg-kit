#include "FFprobeSession.h"
#include "FFmpegKitConfig.h"
#include "LogCallback.h"

extern void addSessionToSessionHistory(const std::shared_ptr <ffmpeg_kit_flutter::Session> session);

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeSession::create(const std::list <std::string> &arguments) {
    auto session = std::static_pointer_cast<ffmpeg_kit_flutter::FFprobeSession>(
            std::make_shared<ffmpeg_kit_flutter::FFprobeSession::PublicFFprobeSession>(arguments,
                                                                                       nullptr,
                                                                                       nullptr,
                                                                                       ffmpeg_kit_flutter::FFmpegKitConfig::getLogRedirectionStrategy()));
    addSessionToSessionHistory(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeSession::create(const std::list <std::string> &arguments,
                                           const FFprobeSessionCompleteCallback completeCallback) {
    auto session = std::static_pointer_cast<ffmpeg_kit_flutter::FFprobeSession>(
            std::make_shared<ffmpeg_kit_flutter::FFprobeSession::PublicFFprobeSession>(arguments,
                                                                                       completeCallback,
                                                                                       nullptr,
                                                                                       ffmpeg_kit_flutter::FFmpegKitConfig::getLogRedirectionStrategy()));
    addSessionToSessionHistory(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeSession::create(const std::list <std::string> &arguments,
                                           const FFprobeSessionCompleteCallback completeCallback,
                                           const ffmpeg_kit_flutter::LogCallback logCallback) {
    auto session = std::static_pointer_cast<ffmpeg_kit_flutter::FFprobeSession>(
            std::make_shared<ffmpeg_kit_flutter::FFprobeSession::PublicFFprobeSession>(arguments,
                                                                                       completeCallback,
                                                                                       logCallback,
                                                                                       ffmpeg_kit_flutter::FFmpegKitConfig::getLogRedirectionStrategy()));
    addSessionToSessionHistory(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeSession::create(const std::list <std::string> &arguments,
                                           const FFprobeSessionCompleteCallback completeCallback,
                                           const ffmpeg_kit_flutter::LogCallback logCallback,
                                           const LogRedirectionStrategy logRedirectionStrategy) {
    auto session = std::static_pointer_cast<ffmpeg_kit_flutter::FFprobeSession>(
            std::make_shared<ffmpeg_kit_flutter::FFprobeSession::PublicFFprobeSession>(arguments,
                                                                                       completeCallback,
                                                                                       logCallback,
                                                                                       logRedirectionStrategy));
    addSessionToSessionHistory(session);
    return session;
}

struct ffmpeg_kit_flutter::FFprobeSession::PublicFFprobeSession
        : public ffmpeg_kit_flutter::FFprobeSession {
    PublicFFprobeSession(const std::list <std::string> &arguments,
                         const FFprobeSessionCompleteCallback completeCallback,
                         const ffmpeg_kit_flutter::LogCallback logCallback,
                         const LogRedirectionStrategy logRedirectionStrategy) :
            FFprobeSession(arguments, completeCallback, logCallback, logRedirectionStrategy) {
    }
};

ffmpeg_kit_flutter::FFprobeSession::FFprobeSession(const std::list <std::string> &arguments,
                                                   const FFprobeSessionCompleteCallback completeCallback,
                                                   const ffmpeg_kit_flutter::LogCallback logCallback,
                                                   const LogRedirectionStrategy logRedirectionStrategy)
        :
        ffmpeg_kit_flutter::AbstractSession(arguments, logCallback, logRedirectionStrategy),
        _completeCallback{completeCallback} {
}

ffmpeg_kit_flutter::FFprobeSessionCompleteCallback
ffmpeg_kit_flutter::FFprobeSession::getCompleteCallback() {
    return _completeCallback;
}

bool ffmpeg_kit_flutter::FFprobeSession::isFFmpeg() const {
    return false;
}

bool ffmpeg_kit_flutter::FFprobeSession::isFFprobe() const {
    return true;
}

bool ffmpeg_kit_flutter::FFprobeSession::isMediaInformation() const {
    return false;
}
