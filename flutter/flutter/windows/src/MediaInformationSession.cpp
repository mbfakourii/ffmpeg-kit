#include "MediaInformationSession.h"
#include "LogCallback.h"
#include "MediaInformation.h"

extern void addSessionToSessionHistory(const std::shared_ptr <ffmpeg_kit_flutter::Session> session);

std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
ffmpeg_kit_flutter::MediaInformationSession::create(const std::list <std::string> &arguments) {
    auto session = std::static_pointer_cast<ffmpeg_kit_flutter::MediaInformationSession>(
            std::make_shared<ffmpeg_kit_flutter::MediaInformationSession::PublicMediaInformationSession>(
                    arguments, nullptr, nullptr));
    addSessionToSessionHistory(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
ffmpeg_kit_flutter::MediaInformationSession::create(const std::list <std::string> &arguments,
                                                    ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback completeCallback) {
    auto session = std::static_pointer_cast<ffmpeg_kit_flutter::MediaInformationSession>(
            std::make_shared<ffmpeg_kit_flutter::MediaInformationSession::PublicMediaInformationSession>(
                    arguments, completeCallback, nullptr));
    addSessionToSessionHistory(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
ffmpeg_kit_flutter::MediaInformationSession::create(const std::list <std::string> &arguments,
                                                    ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback completeCallback,
                                                    ffmpeg_kit_flutter::LogCallback logCallback) {
    auto session = std::static_pointer_cast<ffmpeg_kit_flutter::MediaInformationSession>(
            std::make_shared<ffmpeg_kit_flutter::MediaInformationSession::PublicMediaInformationSession>(
                    arguments, completeCallback, logCallback));
    addSessionToSessionHistory(session);
    return session;
}

struct ffmpeg_kit_flutter::MediaInformationSession::PublicMediaInformationSession
        : public ffmpeg_kit_flutter::MediaInformationSession {
    PublicMediaInformationSession(const std::list <std::string> &arguments,
                                  ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback completeCallback,
                                  ffmpeg_kit_flutter::LogCallback logCallback) :
            MediaInformationSession(arguments, completeCallback, logCallback) {
    }
};

ffmpeg_kit_flutter::MediaInformationSession::MediaInformationSession(
        const std::list <std::string> &arguments,
        ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback completeCallback,
        ffmpeg_kit_flutter::LogCallback logCallback) :
        ffmpeg_kit_flutter::AbstractSession(arguments, logCallback,
                                            ffmpeg_kit_flutter::LogRedirectionStrategyNeverPrintLogs),
        _completeCallback{completeCallback}, _mediaInformation{nullptr} {
}

std::shared_ptr <ffmpeg_kit_flutter::MediaInformation>
ffmpeg_kit_flutter::MediaInformationSession::getMediaInformation() {
    return _mediaInformation;
}

void ffmpeg_kit_flutter::MediaInformationSession::setMediaInformation( const std::shared_ptr <ffmpeg_kit_flutter::MediaInformation> mediaInformation) {
    _mediaInformation = mediaInformation;
}

ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback
ffmpeg_kit_flutter::MediaInformationSession::getCompleteCallback() {
    return _completeCallback;
}

bool ffmpeg_kit_flutter::MediaInformationSession::isFFmpeg() const {
    return false;
}

bool ffmpeg_kit_flutter::MediaInformationSession::isFFprobe() const {
    return false;
}

bool ffmpeg_kit_flutter::MediaInformationSession::isMediaInformation() const {
    return true;
}
