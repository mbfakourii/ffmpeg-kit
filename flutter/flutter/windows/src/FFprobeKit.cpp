#include "FFmpegKit.h"
#include "FFmpegKitConfig.h"
#include "FFprobeKit.h"

extern void *ffmpegKitInitialize();

const void *_ffprobeKitInitializer{ffmpegKitInitialize()};

static std::list <std::string> defaultGetMediaInformationCommandArguments(const std::string &path) {
    return std::list < std::string >
           {"-v", "error", "-hide_banner", "-print_format", "json", "-show_format", "-show_streams",
            "-show_chapters", "-i", path};
}

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeKit::executeWithArguments(const std::list <std::string> &arguments) {
    auto session = ffmpeg_kit_flutter::FFprobeSession::create(arguments);
    ffmpeg_kit_flutter::FFmpegKitConfig::ffprobeExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeKit::executeWithArgumentsAsync(const std::list <std::string> &arguments,
                                                          FFprobeSessionCompleteCallback completeCallback) {
    auto session = ffmpeg_kit_flutter::FFprobeSession::create(arguments, completeCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFprobeExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeKit::executeWithArgumentsAsync(const std::list <std::string> &arguments,
                                                          FFprobeSessionCompleteCallback completeCallback,
                                                          ffmpeg_kit_flutter::LogCallback logCallback) {
    auto session = ffmpeg_kit_flutter::FFprobeSession::create(arguments, completeCallback,
                                                              logCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFprobeExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeKit::execute(const std::string command) {
    auto session = ffmpeg_kit_flutter::FFprobeSession::create(
            FFmpegKitConfig::parseArguments(command.c_str()));
    ffmpeg_kit_flutter::FFmpegKitConfig::ffprobeExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeKit::executeAsync(const std::string command,
                                             FFprobeSessionCompleteCallback completeCallback) {
    auto session = ffmpeg_kit_flutter::FFprobeSession::create(
            FFmpegKitConfig::parseArguments(command.c_str()), completeCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFprobeExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession>
ffmpeg_kit_flutter::FFprobeKit::executeAsync(const std::string command,
                                             FFprobeSessionCompleteCallback completeCallback,
                                             ffmpeg_kit_flutter::LogCallback logCallback) {
    auto session = ffmpeg_kit_flutter::FFprobeSession::create(
            FFmpegKitConfig::parseArguments(command.c_str()), completeCallback, logCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFprobeExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
ffmpeg_kit_flutter::FFprobeKit::getMediaInformation(const std::string path) {
    auto arguments = defaultGetMediaInformationCommandArguments(path);
    auto session = ffmpeg_kit_flutter::MediaInformationSession::create(arguments);
    ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationExecute(session,
                                                                    ffmpeg_kit_flutter::AbstractSession::DefaultTimeoutForAsynchronousMessagesInTransmit);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
ffmpeg_kit_flutter::FFprobeKit::getMediaInformation(const std::string path, const int waitTimeout) {
    auto arguments = defaultGetMediaInformationCommandArguments(path);
    auto session = ffmpeg_kit_flutter::MediaInformationSession::create(arguments);
    ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationExecute(session, waitTimeout);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
ffmpeg_kit_flutter::FFprobeKit::getMediaInformationAsync(const std::string path,
                                                         MediaInformationSessionCompleteCallback completeCallback) {
    auto arguments = defaultGetMediaInformationCommandArguments(path);
    auto session = ffmpeg_kit_flutter::MediaInformationSession::create(arguments, completeCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncGetMediaInformationExecute(session,
                                                                         ffmpeg_kit_flutter::AbstractSession::DefaultTimeoutForAsynchronousMessagesInTransmit);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
ffmpeg_kit_flutter::FFprobeKit::getMediaInformationAsync(const std::string path,
                                                         MediaInformationSessionCompleteCallback completeCallback,
                                                         ffmpeg_kit_flutter::LogCallback logCallback,
                                                         const int waitTimeout) {
    auto arguments = defaultGetMediaInformationCommandArguments(path);
    auto session = ffmpeg_kit_flutter::MediaInformationSession::create(arguments, completeCallback,
                                                                       logCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncGetMediaInformationExecute(session, waitTimeout);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession>
ffmpeg_kit_flutter::FFprobeKit::getMediaInformationFromCommand(const std::string command) {
    auto session = ffmpeg_kit_flutter::MediaInformationSession::create(
            FFmpegKitConfig::parseArguments(command.c_str()));
    ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationExecute(session,
                                                                    ffmpeg_kit_flutter::AbstractSession::DefaultTimeoutForAsynchronousMessagesInTransmit);
    return session;
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::FFprobeSession>>>

ffmpeg_kit_flutter::FFprobeKit::listFFprobeSessions() {
    return ffmpeg_kit_flutter::FFmpegKitConfig::getFFprobeSessions();
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::MediaInformationSession>>>

ffmpeg_kit_flutter::FFprobeKit::listMediaInformationSessions() {
    return ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationSessions();
}
