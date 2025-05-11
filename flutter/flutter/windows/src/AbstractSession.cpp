#include "AbstractSession.h"
#include "FFmpegKit.h"
#include "FFmpegKitConfig.h"
#include "LogCallback.h"
#include "ReturnCode.h"
#include <mutex>
#include <thread>
#include <iostream>
#include <atomic>
#include <algorithm>
#include <condition_variable>

static std::atomic<long> sessionIdGenerator(1);

extern void addSessionToSessionHistory(const std::shared_ptr <ffmpeg_kit_flutter::Session> session);

ffmpeg_kit_flutter::AbstractSession::AbstractSession(const std::list <std::string> &arguments,
                                                     const ffmpeg_kit_flutter::LogCallback logCallback,
                                                     const LogRedirectionStrategy logRedirectionStrategy)
        :
        _arguments{std::make_shared < std::list < std::string >> (arguments)},
        _sessionId{sessionIdGenerator++},
        _logCallback{logCallback},
        _createTime{std::chrono::system_clock::now()},
        _logs{std::make_shared < std::list < std::shared_ptr < ffmpeg_kit_flutter::Log>>>()},
        _state{SessionStateCreated},
        _returnCode{nullptr},
        _logRedirectionStrategy{logRedirectionStrategy} {
}

void ffmpeg_kit_flutter::AbstractSession::waitForAsynchronousMessagesInTransmit(
        const int timeout) const {
    std::mutex mutex;
    std::unique_lock <std::mutex> lock(mutex);
    std::condition_variable condition_variable;
    const std::chrono::time_point <std::chrono::system_clock> expireTime =
            std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);

    while (this->thereAreAsynchronousMessagesInTransmit() &&
           (std::chrono::system_clock::now() < expireTime)) {
        condition_variable.wait_for(lock, std::chrono::milliseconds(100));
    }
}

ffmpeg_kit_flutter::LogCallback ffmpeg_kit_flutter::AbstractSession::getLogCallback() const {
    return _logCallback;
}

long ffmpeg_kit_flutter::AbstractSession::getSessionId() const {
    return _sessionId;
}

std::chrono::time_point <std::chrono::system_clock>
ffmpeg_kit_flutter::AbstractSession::getCreateTime() const {
    return _createTime;
}

std::chrono::time_point <std::chrono::system_clock>
ffmpeg_kit_flutter::AbstractSession::getStartTime() const {
    return _startTime;
}

std::chrono::time_point <std::chrono::system_clock>
ffmpeg_kit_flutter::AbstractSession::getEndTime() const {
    return _endTime;
}

long ffmpeg_kit_flutter::AbstractSession::getDuration() const {
    const std::chrono::time_point <std::chrono::system_clock> startTime = _startTime;
    const std::chrono::time_point <std::chrono::system_clock> endTime = _endTime;

    if (startTime.time_since_epoch() != std::chrono::microseconds(0) &&
        endTime.time_since_epoch() != std::chrono::microseconds(0)) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime).count();
        return static_cast<long>(duration);
    }

    return 0;
}

std::shared_ptr <std::list<std::string>> ffmpeg_kit_flutter::AbstractSession::getArguments() const {
    return _arguments;
}

std::string ffmpeg_kit_flutter::AbstractSession::getCommand() const {
    return ffmpeg_kit_flutter::FFmpegKitConfig::argumentsToString(_arguments);
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Log>>>

ffmpeg_kit_flutter::AbstractSession::getAllLogsWithTimeout(const int waitTimeout) const {
    this->waitForAsynchronousMessagesInTransmit(waitTimeout);

    if (this->thereAreAsynchronousMessagesInTransmit()) {
        std::cout
                << "getAllLogsWithTimeout was called to return all logs but there are still logs being transmitted for session id "
                << _sessionId << "." << std::endl;
    }

    return this->getLogs();
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Log>>>

ffmpeg_kit_flutter::AbstractSession::getAllLogs() const {
    return this->getAllLogsWithTimeout(
            ffmpeg_kit_flutter::AbstractSession::DefaultTimeoutForAsynchronousMessagesInTransmit);
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Log>>>

ffmpeg_kit_flutter::AbstractSession::getLogs() const {
    return _logs;
}

std::string
ffmpeg_kit_flutter::AbstractSession::getAllLogsAsStringWithTimeout(const int waitTimeout) const {
    this->waitForAsynchronousMessagesInTransmit(waitTimeout);

    if (this->thereAreAsynchronousMessagesInTransmit()) {
        std::cout
                << "getAllLogsAsStringWithTimeout was called to return all logs but there are still logs being transmitted for session id "
                << _sessionId << "." << std::endl;
    }

    return this->getLogsAsString();
}

std::string ffmpeg_kit_flutter::AbstractSession::getAllLogsAsString() const {
    return this->getAllLogsAsStringWithTimeout(
            ffmpeg_kit_flutter::AbstractSession::DefaultTimeoutForAsynchronousMessagesInTransmit);
}

std::string ffmpeg_kit_flutter::AbstractSession::getLogsAsString() const {
    std::string concatenatedString;

    std::for_each(_logs->cbegin(), _logs->cend(),
                  [&](std::shared_ptr <ffmpeg_kit_flutter::Log> log) {
                      concatenatedString.append(log->getMessage());
                  });

    return concatenatedString;
}

std::string ffmpeg_kit_flutter::AbstractSession::getOutput() const {
    return this->getAllLogsAsString();
}

ffmpeg_kit_flutter::SessionState ffmpeg_kit_flutter::AbstractSession::getState() const {
    return _state;
}

std::shared_ptr <ffmpeg_kit_flutter::ReturnCode>
ffmpeg_kit_flutter::AbstractSession::getReturnCode() const {
    return _returnCode;
}

std::string ffmpeg_kit_flutter::AbstractSession::getFailStackTrace() const {
    return _failStackTrace;
}

ffmpeg_kit_flutter::LogRedirectionStrategy
ffmpeg_kit_flutter::AbstractSession::getLogRedirectionStrategy() const {
    return _logRedirectionStrategy;
}

bool ffmpeg_kit_flutter::AbstractSession::thereAreAsynchronousMessagesInTransmit() const {
    return (FFmpegKitConfig::messagesInTransmit(_sessionId) != 0);
}

void
ffmpeg_kit_flutter::AbstractSession::addLog(const std::shared_ptr <ffmpeg_kit_flutter::Log> log) {
    _logs->push_back(log);
}

void ffmpeg_kit_flutter::AbstractSession::startRunning() {
    _state = SessionStateRunning;
    _startTime = std::chrono::system_clock::now();
}

void ffmpeg_kit_flutter::AbstractSession::complete(
        const std::shared_ptr <ffmpeg_kit_flutter::ReturnCode> returnCode) {
    _returnCode = returnCode;
    _state = SessionStateCompleted;
    _endTime = std::chrono::system_clock::now();
}

void ffmpeg_kit_flutter::AbstractSession::fail(const char *error) {
    _failStackTrace = error;
    _state = SessionStateFailed;
    _endTime = std::chrono::system_clock::now();
}

bool ffmpeg_kit_flutter::AbstractSession::isFFmpeg() const {
    // IMPLEMENTED IN SUBCLASSES
    return false;
}

bool ffmpeg_kit_flutter::AbstractSession::isFFprobe() const {
    // IMPLEMENTED IN SUBCLASSES
    return false;
}

bool ffmpeg_kit_flutter::AbstractSession::isMediaInformation() const {
    // IMPLEMENTED IN SUBCLASSES
    return false;
}

void ffmpeg_kit_flutter::AbstractSession::cancel() {
    if (_state == SessionStateRunning) {
        FFmpegKit::cancel(_sessionId);
    }
}
