#include "ArchDetect.h"
#include "FFmpegKit.h"
#include "FFmpegKitConfig.h"
#include "Packages.h"

//extern "C" {
//    void cancel_operation(long id);
//}

extern void *ffmpegKitInitialize();

const void *_ffmpegKitInitializeri{ffmpegKitInitialize()};

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegKit::executeWithArguments(const std::list <std::string> &arguments) {
    auto session = ffmpeg_kit_flutter::FFmpegSession::create(arguments);
    ffmpeg_kit_flutter::FFmpegKitConfig::ffmpegExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegKit::executeWithArgumentsAsync(const std::list <std::string> &arguments,
                                                         FFmpegSessionCompleteCallback completeCallback) {
    auto session = ffmpeg_kit_flutter::FFmpegSession::create(arguments, completeCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFmpegExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegKit::executeWithArgumentsAsync(const std::list <std::string> &arguments,
                                                         FFmpegSessionCompleteCallback completeCallback,
                                                         ffmpeg_kit_flutter::LogCallback logCallback,
                                                         ffmpeg_kit_flutter::StatisticsCallback statisticsCallback) {
    auto session = ffmpeg_kit_flutter::FFmpegSession::create(arguments, completeCallback,
                                                             logCallback, statisticsCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFmpegExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegKit::execute(const std::string command) {
    auto session = ffmpeg_kit_flutter::FFmpegSession::create(
            FFmpegKitConfig::parseArguments(command.c_str()));
    ffmpeg_kit_flutter::FFmpegKitConfig::ffmpegExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegKit::executeAsync(const std::string command,
                                            FFmpegSessionCompleteCallback completeCallback) {
    auto session = ffmpeg_kit_flutter::FFmpegSession::create(
            FFmpegKitConfig::parseArguments(command.c_str()), completeCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFmpegExecute(session);
    return session;
}

std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession>
ffmpeg_kit_flutter::FFmpegKit::executeAsync(const std::string command,
                                            FFmpegSessionCompleteCallback completeCallback,
                                            ffmpeg_kit_flutter::LogCallback logCallback,
                                            ffmpeg_kit_flutter::StatisticsCallback statisticsCallback) {
    auto session = ffmpeg_kit_flutter::FFmpegSession::create(
            FFmpegKitConfig::parseArguments(command.c_str()), completeCallback, logCallback,
            statisticsCallback);
    ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFmpegExecute(session);
    return session;
}

void ffmpeg_kit_flutter::FFmpegKit::cancel() {

    /*
     * ZERO (0) IS A SPECIAL SESSION ID
     * WHEN IT IS PASSED TO THIS METHOD, A SIGINT IS GENERATED WHICH CANCELS ALL ONGOING SESSIONS
     */
    //cancel_operation(0);

    auto sessionPtr = ffmpeg_kit_flutter::FFmpegKitConfig::getSessions();
    if (sessionPtr) {
        std::list<std::shared_ptr<ffmpeg_kit_flutter::Session>> sessionList = *sessionPtr;
        for (const auto& session : sessionList) {
            auto ffmpegSession = std::dynamic_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(session);
            if (ffmpegSession) {
                ffmpegSession->canceled = true;
            }
        }
    }
    
}

void ffmpeg_kit_flutter::FFmpegKit::cancel(const long sessionId) {
    //cancel_operation(sessionId);

    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
        sessionId);
    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> ffmpegSession = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
        session);
    ffmpegSession->canceled = true;

}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::FFmpegSession>>>

ffmpeg_kit_flutter::FFmpegKit::listSessions() {
    return ffmpeg_kit_flutter::FFmpegKitConfig::getFFmpegSessions();
}
