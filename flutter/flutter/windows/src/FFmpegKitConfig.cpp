#include <sys/types.h>
#include <sys/stat.h>

#define NOMINMAX // This undef for fix conflict with <windows.h> in rapidjson

#include <windows.h>
#include "ArchDetect.h"
#include "FFmpegKit.h"
#include "FFmpegKitConfig.h"
#include "FFmpegSession.h"
#include "FFprobeKit.h"
#include "FFprobeSession.h"
#include "Level.h"
#include "LogRedirectionStrategy.h"
#include "MediaInformationSession.h"
#include "Packages.h"
#include "SessionState.h"
#include <atomic>
#include <mutex>
#include <future>
#include <condition_variable>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <regex>
#include <thread>
#include <sstream>
#include <string>
#include <memory>
#include <io.h>
#include <locale>
#include <codecvt>
#include <direct.h>

extern "C" {
void set_report_callback(void (*callback)(int, float, float, int64_t, double, double, double));
void cancel_operation(long id);
}

/**
 * Generates ids for named ffmpeg kit pipes.
 */
static std::atomic<long> pipeIndexGenerator(1);

/* Session history variables */
static int sessionHistorySize;
static std::map<long, std::shared_ptr<ffmpeg_kit_flutter::Session>> sessionHistoryMap;
static std::list <std::shared_ptr<ffmpeg_kit_flutter::Session>> sessionHistoryList;
static std::recursive_mutex sessionMutex;

/** Session control variables */
#define SESSION_MAP_SIZE 1000
static std::atomic<short> sessionMap[SESSION_MAP_SIZE];
static std::atomic<int> sessionInTransitMessageCountMap[SESSION_MAP_SIZE];

/** Holds callback defined to redirect logs */
static ffmpeg_kit_flutter::LogCallback logCallback;

/** Holds callback defined to redirect statistics */
static ffmpeg_kit_flutter::StatisticsCallback statisticsCallback;

/** Holds complete callbacks defined to redirect asynchronous execution results */
static ffmpeg_kit_flutter::FFmpegSessionCompleteCallback ffmpegSessionCompleteCallback;
static ffmpeg_kit_flutter::FFprobeSessionCompleteCallback ffprobeSessionCompleteCallback;
static ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback mediaInformationSessionCompleteCallback;

static ffmpeg_kit_flutter::LogRedirectionStrategy globalLogRedirectionStrategy;

/** Redirection control variables */
static int redirectionEnabled;
static std::recursive_mutex callbackDataMutex;
static std::mutex callbackMutex;
static std::condition_variable callbackMonitor;

class CallbackData;

static std::list<CallbackData *> callbackDataList;

/** Fields that control the handling of SIGNALs */
volatile int handleSIGQUIT = 1;
volatile int handleSIGINT = 1;
volatile int handleSIGTERM = 1;
volatile int handleSIGXCPU = 1;
volatile int handleSIGPIPE = 1;

/** Holds the id of the current execution */
//__thread long globalSessionId = 0;

/** Holds the default log level */
int configuredLogLevel = ffmpeg_kit_flutter::LevelAVLogInfo;

#ifdef __cplusplus
extern "C" {
#endif

/** Forward declaration for function defined in fftools_ffmpeg.c */
int ffmpeg_execute(int argc, char **argv);

/** Forward declaration for function defined in fftools_ffprobe.c */
int ffprobe_execute(int argc, char **argv);

void ffmpegkit_log_callback_function(void *ptr, int level, const char *format, va_list vargs);

#ifdef __cplusplus
}
#endif

static std::once_flag ffmpegKitInitializerFlag;
//static pthread_t callbackThread;

void *ffmpegKitInitialize();

const void *_ffmpegKitConfigInitializer{ffmpegKitInitialize()};

enum CallbackType {
    LogType,
    StatisticsType
};

//static bool fs_exists(const std::string& s, const bool isFile, const bool isDirectory) {
//    //struct stat dir_info;
//
//  /*  if (stat(s.c_str(), &dir_info) == 0) {
//        if (isFile && S_ISREG(dir_info.st_mode)) {
//            return true;
//        }
//        if (isDirectory && S_ISDIR(dir_info.st_mode)) {
//            return true;
//        }
//    }*/
//
//    return false;
//}

//static bool fs_create_dir(const std::string& s) {
//   /* if (!fs_exists(s, false, true)) {
//        if (mkdir(s.c_str(), S_IRWXU | S_IRWXG | S_IROTH) != 0) {
//            std::cout << "Failed to create directory: " << s << ". Operation failed with " << errno << "." << std::endl;
//            return false;
//        }
//    }*/
//    return true;
//}

void deleteExpiredSessions() {
    while (sessionHistoryList.size() > sessionHistorySize) {
        auto first = sessionHistoryList.front();
        if (first != nullptr) {
            sessionHistoryList.pop_front();
            sessionHistoryMap.erase(first->getSessionId());
        }
    }
}

void addSessionToSessionHistory(const std::shared_ptr <ffmpeg_kit_flutter::Session> session) {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);

    const long sessionId = session->getSessionId();

    lock.lock();

    /*
     * ASYNC SESSIONS CALL THIS METHOD TWICE
     * THIS CHECK PREVENTS ADDING THE SAME SESSION AGAIN
     */
    if (sessionHistoryMap.count(sessionId) == 0) {
        sessionHistoryMap.insert({sessionId, session});
        sessionHistoryList.push_back(session);
        deleteExpiredSessions();
    }

    lock.unlock();
}

/**
 * Callback data class.
 */
class CallbackData {
public:
    //CallbackData(const long sessionId, const  int logLevel, const AVBPrint* data) :
    //    _type{LogType}, _sessionId{sessionId}, _logLevel{logLevel} {
    //     /*   av_bprint_init(&_logData, 0, AV_BPRINT_SIZE_UNLIMITED);
    //        av_bprintf(&_logData, "%s", data->str);*/
    //}

    CallbackData(const long sessionId,
                 const int videoFrameNumber,
                 const float videoFps,
                 const float videoQuality,
                 const int64_t size,
                 const double time,
                 const double bitrate,
                 const double speed) :
            _type{StatisticsType},
            _sessionId{sessionId},
            _statisticsFrameNumber{videoFrameNumber},
            _statisticsFps{videoFps},
            _statisticsQuality{videoQuality},
            _statisticsSize{size},
            _statisticsTime{time},
            _statisticsBitrate{bitrate},
            _statisticsSpeed{speed} {
    }

    CallbackType getType() {
        return _type;
    }

    long getSessionId() {
        return _sessionId;
    }

    int getLogLevel() {
        return _logLevel;
    }

    /*  AVBPrint* getLogData() {
          return &_logData;
      }*/

    int getStatisticsFrameNumber() {
        return _statisticsFrameNumber;
    }

    float getStatisticsFps() {
        return _statisticsFps;
    }

    float getStatisticsQuality() {
        return _statisticsQuality;
    }

    int64_t getStatisticsSize() {
        return _statisticsSize;
    }

    double getStatisticsTime() {
        return _statisticsTime;
    }

    double getStatisticsBitrate() {
        return _statisticsBitrate;
    }

    double getStatisticsSpeed() {
        return _statisticsSpeed;
    }

private:
    CallbackType _type;
    long _sessionId;                    // session id

    int _logLevel;                      // log level
    //AVBPrint _logData;                  // log data

    int _statisticsFrameNumber;         // statistics frame number
    float _statisticsFps;               // statistics fps
    float _statisticsQuality;           // statistics quality
    int64_t _statisticsSize;            // statistics size
    double _statisticsTime;             // statistics time
    double _statisticsBitrate;          // statistics bitrate
    double _statisticsSpeed;            // statistics speed
};

/**
 * Waits on the callback semaphore for the given time.
 *
 * @param milliSeconds wait time in milliseconds
 */
//static void callbackWait(int milliSeconds) {
//    std::unique_lock<std::mutex> callbackLock{callbackMutex};
//    callbackMonitor.wait_for(callbackLock, std::chrono::milliseconds(milliSeconds));
//}

/**
 * Notifies threads waiting on callback semaphore.
 */
//static void callbackNotify() {
//    callbackMonitor.notify_one();
//}

//static const char *avutil_log_get_level_str(int level) {
//    switch (level) {
//    case AV_LOG_STDERR:
//        return "stderr";
//    case AV_LOG_QUIET:
//        return "quiet";
//    case AV_LOG_DEBUG:
//        return "debug";
//    case AV_LOG_VERBOSE:
//        return "verbose";
//    case AV_LOG_INFO:
//        return "info";
//    case AV_LOG_WARNING:
//        return "warning";
//    case AV_LOG_ERROR:
//        return "error";
//    case AV_LOG_FATAL:
//        return "fatal";
//    case AV_LOG_PANIC:
//        return "panic";
//    default:
//        return "";
//    }
//}

//static void avutil_log_format_line(void *avcl, int level, const char *fmt, va_list vl, AVBPrint part[4], int *print_prefix) {
//    int flags = av_log_get_flags();
//    AVClass* avc = avcl ? *(AVClass **) avcl : NULL;
//    av_bprint_init(part+0, 0, 1);
//    av_bprint_init(part+1, 0, 1);
//    av_bprint_init(part+2, 0, 1);
//    av_bprint_init(part+3, 0, 65536);
//
//    if (*print_prefix && avc) {
//        if (avc->parent_log_context_offset) {
//            AVClass** parent = *(AVClass ***) (((uint8_t *) avcl) +
//                                   avc->parent_log_context_offset);
//            if (parent && *parent) {
//                av_bprintf(part+0, "[%s @ %p] ",
//                         (*parent)->item_name(parent), parent);
//            }
//        }
//        av_bprintf(part+1, "[%s @ %p] ",
//                 avc->item_name(avcl), avcl);
//    }
//
//    if (*print_prefix && (level > AV_LOG_QUIET) && (flags & AV_LOG_PRINT_LEVEL))
//        av_bprintf(part+2, "[%s] ", avutil_log_get_level_str(level));
//
//    av_vbprintf(part+3, fmt, vl);
//
//    if(*part[0].str || *part[1].str || *part[2].str || *part[3].str) {
//        char lastc = part[3].len && part[3].len <= part[3].size ? part[3].str[part[3].len - 1] : 0;
//        *print_prefix = lastc == '\n' || lastc == '\r';
//    }
//}

//static void avutil_log_sanitize(char *line) {
//    while(*line){
//        if(*line < 0x08 || (*line > 0x0D && *line < 0x20))
//            *line='?';
//        line++;
//    }
//}

/**
 * Adds log data to the end of callback data list.
 *
 * @param level log level
 * @param data log data
 */
//static void logCallbackDataAdd(int level, AVBPrint *data) {
//    std::unique_lock<std::recursive_mutex> lock(callbackDataMutex, std::defer_lock);
//    CallbackData* callbackData = new CallbackData(globalSessionId, level, data);
//
//    lock.lock();
//    callbackDataList.push_back(callbackData);
//    lock.unlock();
//
//    callbackNotify();
//
//    std::atomic_fetch_add(&sessionInTransitMessageCountMap[globalSessionId % SESSION_MAP_SIZE], 1);
//}

/**
 * Adds statistics data to the end of callback data list.
 */
//static void statisticsCallbackDataAdd(int frameNumber, float fps, float quality, int64_t size, int time, double bitrate, double speed) {
//    std::unique_lock<std::recursive_mutex> lock(callbackDataMutex, std::defer_lock);
//    CallbackData* callbackData = new CallbackData(globalSessionId, frameNumber, fps, quality, size, time, bitrate, speed);
//
//    lock.lock();
//    callbackDataList.push_back(callbackData);
//    lock.unlock();
//
//    callbackNotify();
//
//    std::atomic_fetch_add(&sessionInTransitMessageCountMap[globalSessionId % SESSION_MAP_SIZE], 1);
//}

/**
 * Removes head of callback data list.
 */
//static CallbackData *callbackDataRemove() {
//    std::unique_lock<std::recursive_mutex> lock(callbackDataMutex, std::defer_lock);
//    CallbackData* newData = nullptr;
//
//    lock.lock();
//    if (callbackDataList.size() > 0) {
//        newData = callbackDataList.front();
//        callbackDataList.pop_front();
//    }
//    lock.unlock();
//
//    return newData;
//}

/**
 * Registers a session id to the session map.
 *
 * @param sessionId session id
 */
//static void registerSessionId(long sessionId) {
//    std::atomic_store(&sessionMap[sessionId % SESSION_MAP_SIZE], (short)1);
//}

/**
 * Removes a session id from the session map.
 *
 * @param sessionId session id
 */
//static void removeSession(long sessionId) {
//    std::atomic_store(&sessionMap[sessionId % SESSION_MAP_SIZE], (short)0);
//}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Adds a cancel session request to the session map.
 *
 * @param sessionId session id
 */
void cancelSession(long sessionId) {
    std::atomic_store(&sessionMap[sessionId % SESSION_MAP_SIZE], (short) 2);
}

/**
 * Checks whether a cancel request for the given session id exists in the session map.
 *
 * @param sessionId session id
 * @return 1 if exists, false otherwise
 */
int cancelRequested(long sessionId) {
    if (std::atomic_load(&sessionMap[sessionId % SESSION_MAP_SIZE]) == 2) {
        return 1;
    } else {
        return 0;
    }
}

#ifdef __cplusplus
}
#endif

/**
 * Resets the number of messages in transmit for this session.
 *
 * @param sessionId session id
 */
//static void resetMessagesInTransmit(long sessionId) {
//    std::atomic_store(&sessionInTransitMessageCountMap[sessionId % SESSION_MAP_SIZE], 0);
//}

/**
 * Callback function for FFmpeg/FFprobe logs.
 *
 * @param ptr pointer to AVClass struct
 * @param level log level
 * @param format format string
 * @param vargs arguments
 */
//void ffmpegkit_log_callback_function(void *ptr, int level, const char* format, va_list vargs) {
//    AVBPrint fullLine;
//    AVBPrint part[4];
//    int print_prefix = 1;
//
//    // DO NOT PROCESS UNWANTED LOGS
//    if (level >= 0) {
//        level &= 0xff;
//    }
//    int activeLogLevel = av_log_get_level();
//
//    // LevelAVLogStdErr logs are always redirected
//    if ((activeLogLevel == ffmpeg_kit_flutter::LevelAVLogQuiet && level != ffmpeg_kit_flutter::LevelAVLogStdErr) || (level > activeLogLevel)) {
//        return;
//    }
//
//    av_bprint_init(&fullLine, 0, AV_BPRINT_SIZE_UNLIMITED);
//
//    avutil_log_format_line(ptr, level, format, vargs, part, &print_prefix);
//    avutil_log_sanitize(part[0].str);
//    avutil_log_sanitize(part[1].str);
//    avutil_log_sanitize(part[2].str);
//    avutil_log_sanitize(part[3].str);
//
//    // COMBINE ALL 4 LOG PARTS
//    av_bprintf(&fullLine, "%s%s%s%s", part[0].str, part[1].str, part[2].str, part[3].str);
//
//    if (fullLine.len > 0) {
//        logCallbackDataAdd(level, &fullLine);
//    }
//
//    av_bprint_finalize(part, NULL);
//    av_bprint_finalize(part+1, NULL);
//    av_bprint_finalize(part+2, NULL);
//    av_bprint_finalize(part+3, NULL);
//    av_bprint_finalize(&fullLine, NULL);
//}

/**
 * Callback function for FFmpeg statistics.
 *
 * @param frameNumber last processed frame number
 * @param fps frames processed per second
 * @param quality quality of the output stream (video only)
 * @param size size in bytes
 * @param time processed output duration
 * @param bitrate output bit rate in kbits/s
 * @param speed processing speed = processed duration / operation duration
 */
//void ffmpegkit_statistics_callback_function(int frameNumber, float fps, float quality, int64_t size, double time, double bitrate, double speed) {
//    statisticsCallbackDataAdd(frameNumber, fps, quality, size, time, bitrate, speed);
//}
//
//static void process_log(long sessionId, int levelValueInt, AVBPrint* logMessage) {
//    int activeLogLevel = av_log_get_level();
//    ffmpeg_kit_flutter::Level levelValue = static_cast<ffmpeg_kit_flutter::Level>(levelValueInt);
//    std::shared_ptr<ffmpeg_kit_flutter::Log> log = std::make_shared<ffmpeg_kit_flutter::Log>(sessionId, levelValue, logMessage->str);
//    bool globalCallbackDefined = false;
//    bool sessionCallbackDefined = false;
//    ffmpeg_kit_flutter::LogRedirectionStrategy activeLogRedirectionStrategy = globalLogRedirectionStrategy;
//
//    // LevelAVLogStdErr logs are always redirected
//    if ((activeLogLevel == ffmpeg_kit_flutter::LevelAVLogQuiet && levelValue != ffmpeg_kit_flutter::LevelAVLogStdErr) || (levelValue > activeLogLevel)) {
//        // LOG NEITHER PRINTED NOR FORWARDED
//        return;
//    }
//
//    auto session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(sessionId);
//    if (session != nullptr) {
//        activeLogRedirectionStrategy = session->getLogRedirectionStrategy();
//        session->addLog(log);
//
//        ffmpeg_kit_flutter::LogCallback sessionLogCallback = session->getLogCallback();
//        if (sessionLogCallback != nullptr) {
//            sessionCallbackDefined = true;
//
//            try {
//                // NOTIFY SESSION CALLBACK DEFINED
//                sessionLogCallback(log);
//            } catch(const std::exception& exception) {
//                std::cout << "Exception thrown inside session log callback. " << exception.what() << std::endl;
//            }
//        }
//    }
//
//    ffmpeg_kit_flutter::LogCallback globalLogCallback = logCallback;
//    if (globalLogCallback != nullptr) {
//        globalCallbackDefined = true;
//
//        try {
//            // NOTIFY GLOBAL CALLBACK DEFINED
//            globalLogCallback(log);
//        } catch(const std::exception& exception) {
//            std::cout << "Exception thrown inside global log callback. " << exception.what() << std::endl;
//        }
//    }
//
//    // EXECUTE THE LOG STRATEGY
//    switch (activeLogRedirectionStrategy) {
//        case ffmpeg_kit_flutter::LogRedirectionStrategyNeverPrintLogs: {
//            return;
//        }
//        case ffmpeg_kit_flutter::LogRedirectionStrategyPrintLogsWhenGlobalCallbackNotDefined: {
//            if (globalCallbackDefined) {
//                return;
//            }
//        }
//        break;
//        case ffmpeg_kit_flutter::LogRedirectionStrategyPrintLogsWhenSessionCallbackNotDefined: {
//            if (sessionCallbackDefined) {
//                return;
//            }
//        }
//        break;
//        case ffmpeg_kit_flutter::LogRedirectionStrategyPrintLogsWhenNoCallbacksDefined: {
//            if (globalCallbackDefined || sessionCallbackDefined) {
//                return;
//            }
//        }
//        break;
//        case ffmpeg_kit_flutter::LogRedirectionStrategyAlwaysPrintLogs: {
//        }
//        break;
//    }
//
//    // PRINT LOGS
//    switch (levelValue) {
//        case ffmpeg_kit_flutter::LevelAVLogQuiet:
//            // PRINT NO OUTPUT
//            break;
//        default:
//            // WRITE TO STDOUT
//            std::cout << ffmpeg_kit_flutter::FFmpegKitConfig::logLevelToString(levelValue) << ": " << logMessage->str;
//            break;
//    }
//}

void process_statistics(long sessionId, int videoFrameNumber, float videoFps, float videoQuality,
                        long size, double time, double bitrate, double speed) {
    std::shared_ptr <ffmpeg_kit_flutter::Statistics> statistics = std::make_shared<ffmpeg_kit_flutter::Statistics>(
            sessionId, videoFrameNumber, videoFps, videoQuality, size, time, bitrate, speed);

    auto session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(sessionId);
    if (session != nullptr && session->isFFmpeg()) {
        std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> ffmpegSession = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
                session);
        ffmpegSession->addStatistics(statistics);

        ffmpeg_kit_flutter::StatisticsCallback sessionStatisticsCallback = ffmpegSession->getStatisticsCallback();
        if (sessionStatisticsCallback != nullptr) {
            try {
                sessionStatisticsCallback(statistics);
            } catch (const std::exception &exception) {
                std::cout << "Exception thrown inside session statistics callback. "
                          << exception.what() << std::endl;
            }
        }
    }

    ffmpeg_kit_flutter::StatisticsCallback globalStatisticsCallback = statisticsCallback;
    if (globalStatisticsCallback != nullptr) {
        try {
            globalStatisticsCallback(statistics);
        } catch (const std::exception &exception) {
            std::cout << "Exception thrown inside global statistics callback. " << exception.what()
                      << std::endl;
        }
    }
}

/**
 * Forwards asynchronous messages to Callbacks.
 */
//void *callbackThreadFunction(void *pointer) {
//    int activeLogLevel = av_log_get_level();
//    if ((activeLogLevel != ffmpeg_kit_flutter::LevelAVLogQuiet) && (ffmpeg_kit_flutter::LevelAVLogDebug <= activeLogLevel)) {
//        std::cout << "Async callback block started." << std::endl;
//    }
//
//    while(redirectionEnabled) {
//        try {
//            CallbackData* callbackData = callbackDataRemove();
//
//            if (callbackData != nullptr) {
//
//                if (callbackData->getType() == LogType) {
//                    process_log(callbackData->getSessionId(), callbackData->getLogLevel(), callbackData->getLogData());
//                    av_bprint_finalize(callbackData->getLogData(), NULL);
//                } else {
//                    process_statistics(callbackData->getSessionId(),
//                                       callbackData->getStatisticsFrameNumber(),
//                                       callbackData->getStatisticsFps(),
//                                       callbackData->getStatisticsQuality(),
//                                       callbackData->getStatisticsSize(),
//                                       callbackData->getStatisticsTime(),
//                                       callbackData->getStatisticsBitrate(),
//                                       callbackData->getStatisticsSpeed());
//                }
//
//                std::atomic_fetch_sub(&sessionInTransitMessageCountMap[callbackData->getSessionId() % SESSION_MAP_SIZE], 1);
//
//            } else {
//                callbackWait(100);
//            }
//
//        } catch(const std::exception& exception) {
//            activeLogLevel = av_log_get_level();
//            if ((activeLogLevel != ffmpeg_kit_flutter::LevelAVLogQuiet) && (ffmpeg_kit_flutter::LevelAVLogWarning <= activeLogLevel)) {
//                std::cout << "Async callback block received error: " << exception.what() << std::endl;
//            }
//        }
//    }
//
//    activeLogLevel = av_log_get_level();
//    if ((activeLogLevel != ffmpeg_kit_flutter::LevelAVLogQuiet) && (ffmpeg_kit_flutter::LevelAVLogDebug <= activeLogLevel)) {
//        std::cout << "Async callback block stopped." << std::endl;
//    }
//
//    return NULL;
//}

std::wstring ConvertToWString(const std::string &str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int) str.length(), nullptr,
                                          0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int) str.length(), &wstr[0], size_needed);
    return wstr;
}


static int
executeFFmpeg(const long sessionId, const std::shared_ptr <std::list<std::string>> arguments) {
    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
            sessionId);

    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> ffmpegSession = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
            session);

    std::string command = ffmpeg_kit_flutter::FFmpegKitConfig::argumentsToString(arguments);
    // Get the path of the executable
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(NULL, path, MAX_PATH)) {
        std::cerr << "Failed to get module path: " << GetLastError() << std::endl;
        return 1;
    }

    std::wstring exePath = path;
    size_t lastSlash = exePath.find_last_of(L"\\/");
    std::wstring directoryPath = exePath.substr(0, lastSlash);
    std::wstring executablePath = directoryPath + L"\\ffmpeg.exe ";
    std::wstring fullCommand = executablePath + ConvertToWString(command);

    std::wcout << L"FFmpeg command: " << fullCommand << std::endl;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    HANDLE hInputRead = NULL, hInputWrite = NULL;

    // Create a pipe to capture process output
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        std::cerr << "Failed to create output pipe: " << GetLastError() << std::endl;
        return 1;
    }

    // Create a pipe for process input
    if (!CreatePipe(&hInputRead, &hInputWrite, &sa, 0)) {
        std::cerr << "Failed to create input pipe: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return 1;
    }

    // Ensure the read handle is not inherited
    if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "Failed to set handle information: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        CloseHandle(hInputRead);
        CloseHandle(hInputWrite);
        return 1;
    }

    // Ensure the write handle to the input pipe is not inherited
    if (!SetHandleInformation(hInputWrite, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "Failed to set input handle information: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        CloseHandle(hInputRead);
        CloseHandle(hInputWrite);
        return 1;
    }

    // Set up process startup info
    STARTUPINFOW startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdOutput = hWritePipe;
    startupInfo.hStdError = hWritePipe;
    startupInfo.hStdInput = hInputRead;  // Set the input pipe as stdin

    PROCESS_INFORMATION ffmpegProcessInfo;
    ZeroMemory(&ffmpegProcessInfo, sizeof(ffmpegProcessInfo));

    // Create the FFmpeg process
    BOOL processCreated = CreateProcessW(
            NULL,                                   // No module name (use command line)
            const_cast<wchar_t *>(fullCommand.c_str()),  // Command line
            NULL,                                   // Process handle not inheritable
            NULL,                                   // Thread handle not inheritable
            TRUE,                                   // Inherit handles
            CREATE_NO_WINDOW,                       // Creation flags
            NULL,                                   // Use parent's environment block
            NULL,                                   // Use parent's starting directory
            &startupInfo,                           // Pointer to STARTUPINFO
            &ffmpegProcessInfo                      // Pointer to PROCESS_INFORMATION
    );

    // Close the write end of the pipe in the parent process
    // This is crucial to prevent deadlocks when reading
    CloseHandle(hWritePipe);
    hWritePipe = NULL;

    // Close InputRead in the parent process
    CloseHandle(hInputRead);
    hInputRead = NULL;

    // Close InputWrite in the parent process too - this makes the child process realize
    // that there is no data to read from stdin and it should not wait for user input
    CloseHandle(hInputWrite);
    hInputWrite = NULL;

    if (!processCreated) {
        std::cerr << "Failed to start FFmpeg: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        return 1;
    }



    // Read output from the child process
    //std::string fullLogOutput;
    char buffer[4096];
    DWORD bytesRead;
    BOOL readSuccess;
    BOOL hasError = false;
    BOOL canceled = false;

    while ((readSuccess = ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) &&
           bytesRead > 0) {
        buffer[bytesRead] = '\0';
        //fullLogOutput.append(buffer, bytesRead);

        std::string bufferStr(buffer, bytesRead); // Convert buffer to std::string

        // Check if "Error" is in the output
        if (bufferStr.find("Error") != std::string::npos) {
            hasError = true;
        }

        if (ffmpegSession->canceled) {
            canceled = true;
            TerminateProcess(ffmpegProcessInfo.hProcess, 0);

            break;
        }

        std::shared_ptr <ffmpeg_kit_flutter::Log> log = std::make_shared<ffmpeg_kit_flutter::Log>(
                sessionId, ffmpeg_kit_flutter::LevelAVLogStdErr, buffer
        );


        if (bufferStr.length() > 1) {
            int frameNumber = 0;
            float fps = 0.0f;
            float videoQuality = 0.0f;
            long size = 0;
            double time = 0.0f;
            double bitrate = 0.0f;
            double speed = 0.0f;

            std::vector <std::string> tokens;
            std::istringstream stream(bufferStr);
            std::string token;

            while (stream >> token) {
                tokens.push_back(token);
            }

            for (size_t i = 0; i < tokens.size(); ++i) {
                if (tokens[i].find("frame=") == 0) {
                    if (tokens[i] == "frame=" && i + 1 < tokens.size()) {
                        frameNumber = std::stoi(tokens[i + 1]);
                    }
                    else {
                        frameNumber = std::stoi(tokens[i].substr(6));
                    }
                }
                else if (tokens[i].find("fps=") == 0 && tokens[i].length() > 4) {
                    std::string fpsStr = tokens[i].substr(4);
                    if (fpsStr != "N/A") {
                        fps = std::stof(fpsStr);
                    }
                }
                else if (tokens[i].find("q=") == 0 && tokens[i].length() > 2) {
                    std::string qStr = tokens[i].substr(2);
                    if (qStr != "N/A") {
                        videoQuality = std::stof(qStr);
                    }
                }
                else if (tokens[i] == "size=" && i + 1 < tokens.size()) {
                    std::string rawSize = tokens[i + 1];
                    rawSize.erase(0, rawSize.find_first_not_of(' '));
                    if (rawSize != "N/A") {
                        size = std::stol(rawSize.substr(0, rawSize.find("KiB")));
                    }
                }
                else if (tokens[i].find("time=") == 0 && tokens[i].length() > 5) {
                    std::string timeStr = tokens[i].substr(5);
                    int hours = std::stoi(timeStr.substr(0, 2));
                    int minutes = std::stoi(timeStr.substr(3, 2));
                    double seconds = std::stod(timeStr.substr(6));
                    time = (hours * 3600 + minutes * 60 + seconds) * 1000.0;
                }
                else if (tokens[i].find("bitrate=") == 0 && tokens[i].length() > 8) {
                    std::string bitrateStr = tokens[i].substr(8);
                    if (bitrateStr != "N/A") {
                        bitrate = std::stof(bitrateStr);
                    }
                }
                else if (tokens[i].find("speed=") == 0 && tokens[i].length() > 6) {
                    std::string speedStr = tokens[i].substr(6);
                    if (speedStr != "N/A") {
                        speed = std::stof(speedStr);
                    }
                }
            }

            if (frameNumber != 0) {
                process_statistics(sessionId, frameNumber, fps, videoQuality, size, time, bitrate,
                                   speed);
            }
        }

        session->addLog(log);
    }

    if (!readSuccess && GetLastError() != ERROR_BROKEN_PIPE) {
        std::cerr << "Error reading from pipe: " << GetLastError() << std::endl;
    }

    // Close the read end of the pipe
    CloseHandle(hReadPipe);

    // Wait for the process to exit
    DWORD exitCode = 0;
    if (WaitForSingleObject(ffmpegProcessInfo.hProcess, INFINITE) == WAIT_FAILED) {
        std::cerr << "Failed to wait for FFmpeg process: " << GetLastError() << std::endl;
        exitCode = 1;
    } else {
        if (!GetExitCodeProcess(ffmpegProcessInfo.hProcess, &exitCode)) {
            std::cerr << "Failed to get FFmpeg process exit code: " << GetLastError() << std::endl;
            exitCode = 1;
        }
    }

    // Clean up process handles
    CloseHandle(ffmpegProcessInfo.hProcess);
    CloseHandle(ffmpegProcessInfo.hThread);

    if (canceled) {
        return 255;
    }

    // Check for errors in the output log
    if (hasError) {
        return 1;
    }

    return exitCode;
}

int executeFFprobe(const long sessionId, const std::shared_ptr <std::list<std::string>> arguments) {
    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
        sessionId);

    std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession> mediaInformationSession = std::static_pointer_cast<ffmpeg_kit_flutter::MediaInformationSession>(
        session);

    std::string command = ffmpeg_kit_flutter::FFmpegKitConfig::argumentsToString(arguments);
    // Get the path of the executable
    wchar_t path[MAX_PATH];
    if (!GetModuleFileNameW(NULL, path, MAX_PATH)) {
        std::cerr << "Failed to get module path: " << GetLastError() << std::endl;
        return 1;
    }

    std::wstring exePath = path;
    size_t lastSlash = exePath.find_last_of(L"\\/");
    std::wstring directoryPath = exePath.substr(0, lastSlash);
    std::wstring executablePath = directoryPath + L"\\ffprobe.exe ";
    std::wstring fullCommand = executablePath + ConvertToWString(command);

    std::wcout << L"FFprobe command: " << fullCommand << std::endl;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    HANDLE hInputRead = NULL, hInputWrite = NULL;

    // Create a pipe to capture process output
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        std::cerr << "Failed to create output pipe: " << GetLastError() << std::endl;
        return 1;
    }

    // Create a pipe for process input
    if (!CreatePipe(&hInputRead, &hInputWrite, &sa, 0)) {
        std::cerr << "Failed to create input pipe: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return 1;
    }

    // Ensure the read handle is not inherited
    if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "Failed to set handle information: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        CloseHandle(hInputRead);
        CloseHandle(hInputWrite);
        return 1;
    }

    // Ensure the write handle to the input pipe is not inherited
    if (!SetHandleInformation(hInputWrite, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "Failed to set input handle information: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        CloseHandle(hInputRead);
        CloseHandle(hInputWrite);
        return 1;
    }

    // Set up process startup info
    STARTUPINFOW startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdOutput = hWritePipe;
    startupInfo.hStdError = hWritePipe;
    startupInfo.hStdInput = hInputRead;  // Set the input pipe as stdin

    PROCESS_INFORMATION ffmpegProcessInfo;
    ZeroMemory(&ffmpegProcessInfo, sizeof(ffmpegProcessInfo));

    // Create the FFmpeg process
    BOOL processCreated = CreateProcessW(
        NULL,                                   // No module name (use command line)
        const_cast<wchar_t*>(fullCommand.c_str()),  // Command line
        NULL,                                   // Process handle not inheritable
        NULL,                                   // Thread handle not inheritable
        TRUE,                                   // Inherit handles
        CREATE_NO_WINDOW,                       // Creation flags
        NULL,                                   // Use parent's environment block
        NULL,                                   // Use parent's starting directory
        &startupInfo,                           // Pointer to STARTUPINFO
        &ffmpegProcessInfo                      // Pointer to PROCESS_INFORMATION
    );

    // Close the write end of the pipe in the parent process
    // This is crucial to prevent deadlocks when reading
    CloseHandle(hWritePipe);
    hWritePipe = NULL;

    // Close InputRead in the parent process
    CloseHandle(hInputRead);
    hInputRead = NULL;

    // Close InputWrite in the parent process too - this makes the child process realize
    // that there is no data to read from stdin and it should not wait for user input
    CloseHandle(hInputWrite);
    hInputWrite = NULL;

    if (!processCreated) {
        std::cerr << "Failed to start FFmpeg: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        return 1;
    }



    // Read output from the child process
    //std::string fullLogOutput;
    char buffer[4096];
    DWORD bytesRead;
    BOOL readSuccess;
    BOOL hasError = false;
    BOOL canceled = false;

    while ((readSuccess = ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) &&
        bytesRead > 0) {
        buffer[bytesRead] = '\0';
        //fullLogOutput.append(buffer, bytesRead);

        std::string bufferStr(buffer, bytesRead); // Convert buffer to std::string


        // Check if "Error" is in the output
        if (bufferStr.find("Error") != std::string::npos) {
            hasError = true;
        }

        std::shared_ptr <ffmpeg_kit_flutter::Log> log = std::make_shared<ffmpeg_kit_flutter::Log>(
            sessionId, ffmpeg_kit_flutter::LevelAVLogStdErr, buffer
        );


        session->addLog(log);
    }

    if (!readSuccess && GetLastError() != ERROR_BROKEN_PIPE) {
        std::cerr << "Error reading from pipe: " << GetLastError() << std::endl;
    }

    // Close the read end of the pipe
    CloseHandle(hReadPipe);

    // Wait for the process to exit
    DWORD exitCode = 0;
    if (WaitForSingleObject(ffmpegProcessInfo.hProcess, INFINITE) == WAIT_FAILED) {
        std::cerr << "Failed to wait for FFmpeg process: " << GetLastError() << std::endl;
        exitCode = 1;
    }
    else {
        if (!GetExitCodeProcess(ffmpegProcessInfo.hProcess, &exitCode)) {
            std::cerr << "Failed to get FFmpeg process exit code: " << GetLastError() << std::endl;
            exitCode = 1;
        }
    }

    // Clean up process handles
    CloseHandle(ffmpegProcessInfo.hProcess);
    CloseHandle(ffmpegProcessInfo.hThread);

    if (canceled) {
        return 255;
    }

    // Check for errors in the output log
    if (hasError) {
        return 1;
    }

    return exitCode;
}

void *ffmpegKitInitialize() {
    std::call_once(ffmpegKitInitializerFlag, []() {
        std::cout << "Loading ffmpeg-kit." << std::endl;

        sessionHistorySize = 10;

        for (int i = 0; i < SESSION_MAP_SIZE; i++) {
            std::atomic_init(&sessionMap[i], (short) 0);
            std::atomic_init(&sessionInTransitMessageCountMap[i], 0);
        }

        logCallback = nullptr;
        statisticsCallback = nullptr;
        ffmpegSessionCompleteCallback = nullptr;
        ffprobeSessionCompleteCallback = nullptr;
        mediaInformationSessionCompleteCallback = nullptr;

        globalLogRedirectionStrategy = ffmpeg_kit_flutter::LogRedirectionStrategyPrintLogsWhenNoCallbacksDefined;

        redirectionEnabled = 0;

        ffmpeg_kit_flutter::FFmpegKitConfig::enableRedirection();

        std::cout << "Loaded ffmpeg-kit-" << ffmpeg_kit_flutter::Packages::getPackageName() << "-"
                  << ffmpeg_kit_flutter::ArchDetect::getArch() << "-"
                  << ffmpeg_kit_flutter::FFmpegKitConfig::getVersion() << "-"
                  << ffmpeg_kit_flutter::FFmpegKitConfig::getBuildDate() << "." << std::endl;
    });

    return NULL;
}

void ffmpeg_kit_flutter::FFmpegKitConfig::enableRedirection() {
    std::unique_lock <std::recursive_mutex> lock(callbackDataMutex, std::defer_lock);
    lock.lock();

    if (redirectionEnabled != 0) {
        lock.unlock();
        return;
    }
    redirectionEnabled = 1;

    lock.unlock();

    //int rc = pthread_create(&callbackThread, NULL, callbackThreadFunction, NULL);

    /* if (rc != 0) {
         std::cout << "Failed to create async callback block: %d" << rc << std::endl;
         lock.unlock();
         return;
     }*/

    /* av_log_set_callback(ffmpegkit_log_callback_function);
     set_report_callback(ffmpegkit_statistics_callback_function);*/

    return;
}
//
//void ffmpeg_kit_flutter::FFmpegKitConfig::disableRedirection() {
//    std::unique_lock<std::recursive_mutex> lock(callbackDataMutex, std::defer_lock);
//
//    lock.lock();
//
//    if (redirectionEnabled != 1) {
//        lock.unlock();
//        return;
//    }
//    redirectionEnabled = 0;
//
//    lock.unlock();
//
//    callbackNotify();
//
//    pthread_detach(callbackThread);
//
//    av_log_set_callback(av_log_default_callback);
//    set_report_callback(NULL);
//}

int ffmpeg_kit_flutter::FFmpegKitConfig::setFontconfigConfigurationPath(const std::string &path) {
    return ffmpeg_kit_flutter::FFmpegKitConfig::setEnvironmentVariable("FONTCONFIG_PATH", path);
}

void ffmpeg_kit_flutter::FFmpegKitConfig::setFontDirectory(const std::string &fontDirectoryPath,
                                                           const std::map <std::string, std::string> &fontNameMapping) {
    ffmpeg_kit_flutter::FFmpegKitConfig::setFontDirectoryList(
            std::list < std::string > {fontDirectoryPath}, fontNameMapping);
}

void ffmpeg_kit_flutter::FFmpegKitConfig::setFontDirectoryList(
        const std::list <std::string> &fontDirectoryList,
        const std::map <std::string, std::string> &fontNameMapping) {
    //int validFontNameMappingCount = 0;

    //const char *parentDirectory = std::getenv("HOME");
    //if (parentDirectory == NULL) {
    //    parentDirectory = std::getenv("TMPDIR");
    //    if (parentDirectory == NULL) {
    //        parentDirectory = ".";
    //    }
    //}

    //std::string cacheDir = std::string(parentDirectory) + "/.cache";
    //std::string ffmpegKitDir = cacheDir + "/ffmpegkit";
    //auto tempConfigurationDirectory = ffmpegKitDir + "/fontconfig";
    //auto fontConfigurationFile = std::string(tempConfigurationDirectory) + "/fonts.conf";

    //if (!fs_create_dir(cacheDir) || !fs_create_dir(ffmpegKitDir) || !fs_create_dir(tempConfigurationDirectory)) {
    //    return;
    //}
    //std::cout << "Created temporary font conf directory: TRUE." << std::endl;

    //if (fs_exists(fontConfigurationFile, true, false)) {
    //    bool fontConfigurationDeleted = std::remove(fontConfigurationFile.c_str());
    //    std::cout << "Deleted old temporary font configuration: " << (fontConfigurationDeleted == 0?"TRUE":"FALSE") << "." << std::endl;
    //}

    ///* PROCESS MAPPINGS FIRST */
    //std::string fontNameMappingBlock = "";
    //for (auto const& pair : fontNameMapping) {
    //    if ((pair.first.size() > 0) && (pair.second.size() > 0)) {

    //        fontNameMappingBlock += "    <match target=\"pattern\">\n";
    //        fontNameMappingBlock += "        <test qual=\"any\" name=\"family\">\n";
    //        fontNameMappingBlock += "                <string>";
    //        fontNameMappingBlock += pair.first;
    //        fontNameMappingBlock += "</string>\n";
    //        fontNameMappingBlock += "        </test>\n";
    //        fontNameMappingBlock += "        <edit name=\"family\" mode=\"assign\" binding=\"same\">\n";
    //        fontNameMappingBlock += "            <string>";
    //        fontNameMappingBlock += pair.second;
    //        fontNameMappingBlock += "</string>\n";
    //        fontNameMappingBlock += "        </edit>\n";
    //        fontNameMappingBlock += "    </match>\n";

    //        validFontNameMappingCount++;
    //    }
    //}

    //std::string fontConfiguration;
    //fontConfiguration += "<?xml version=\"1.0\"?>\n";
    //fontConfiguration += "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">\n";
    //fontConfiguration += "<fontconfig>\n";
    //fontConfiguration += "    <dir prefix=\"cwd\">.</dir>\n";

    //for (const auto& fontDirectoryPath : fontDirectoryList) {
    //    fontConfiguration += "    <dir>";
    //    fontConfiguration += fontDirectoryPath;
    //    fontConfiguration += "</dir>\n";
    //}
    //fontConfiguration += fontNameMappingBlock;
    //fontConfiguration += "</fontconfig>\n";

    //std::ofstream fontConfigurationStream(fontConfigurationFile, std::ios::out | std::ios::trunc);
    //if (fontConfigurationStream) {
    //    fontConfigurationStream << fontConfiguration;
    //}
    //if (fontConfigurationStream.bad()) {
    //    std::cout << "Failed to set font directory. Error received while saving font configuration: " << fontConfigurationStream.rdbuf() << "." << std::endl;
    //}
    //fontConfigurationStream.close();

    //std::cout << "Saved new temporary font configuration with " << validFontNameMappingCount << " font name mappings." << std::endl;

    //ffmpeg_kit_flutter::FFmpegKitConfig::setFontconfigConfigurationPath(tempConfigurationDirectory.c_str());

    //for (const auto& fontDirectoryPath : fontDirectoryList) {
    //    std::cout << "Font directory " << fontDirectoryPath << " registered successfully." << std::endl;
    //}
}

//std::shared_ptr<std::string> ffmpeg_kit_flutter::FFmpegKitConfig::registerNewFFmpegPipe() {
//    const char *parentDirectory = std::getenv("HOME");
//    if (parentDirectory == NULL) {
//        parentDirectory = std::getenv("TMPDIR");
//        if (parentDirectory == NULL) {
//            parentDirectory = ".";
//        }
//    }
//
//    // PIPES ARE CREATED UNDER THE PIPES DIRECTORY
//    std::string cacheDir = std::string(parentDirectory) + "/.cache";
//    std::string ffmpegKitDir = cacheDir + "/ffmpegkit";
//    std::string pipesDir = ffmpegKitDir + "/pipes";
//
//    if (!fs_create_dir(cacheDir) || !fs_create_dir(ffmpegKitDir) || !fs_create_dir(pipesDir)) {
//        return nullptr;
//    }
//
//    std::shared_ptr<std::string> newFFmpegPipePath = std::make_shared<std::string>(pipesDir + "/" + FFmpegKitNamedPipePrefix + std::to_string(pipeIndexGenerator++));
//
//    // FIRST CLOSE OLD PIPES WITH THE SAME NAME
//    ffmpeg_kit_flutter::FFmpegKitConfig::closeFFmpegPipe(newFFmpegPipePath->c_str());
//
//    int rc = mkfifo(newFFmpegPipePath->c_str(), S_IRWXU | S_IRWXG | S_IROTH);
//    if (rc == 0) {
//        return newFFmpegPipePath;
//    } else {
//        std::cout << "Failed to register new FFmpeg pipe " << newFFmpegPipePath << ". Operation failed with rc=" << rc << "." << std::endl;
//        return nullptr;
//    }
//}
//
//void ffmpeg_kit_flutter::FFmpegKitConfig::closeFFmpegPipe(const std::string& ffmpegPipePath) {
//    std::remove(ffmpegPipePath.c_str());
//}
//
//std::string ffmpeg_kit_flutter::FFmpegKitConfig::getFFmpegVersion() {
//    return FFMPEG_VERSION;
//}
//

std::shared_ptr<std::string> ffmpeg_kit_flutter::FFmpegKitConfig::registerNewFFmpegPipe() {
    // Convert FFmpegKitNamedPipePrefix from const char* to std::wstring
    std::wstring ffmpegKitNamedPipePrefixW(FFmpegKitNamedPipePrefix, FFmpegKitNamedPipePrefix + strlen(FFmpegKitNamedPipePrefix));

    // Create new pipe path in the correct format
    std::shared_ptr<std::wstring> newFFmpegPipePath = std::make_shared<std::wstring>(L"\\\\.\\pipe\\");
    *newFFmpegPipePath += ffmpegKitNamedPipePrefixW + std::to_wstring(pipeIndexGenerator++);
    std::wcout << "newFFmpegPipePath: " << *newFFmpegPipePath << std::endl;

    // Close old pipes with the same name
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string pipePathString = converter.to_bytes(*newFFmpegPipePath);
    std::cout << "Closing old pipe at: " << pipePathString << std::endl;
    ffmpeg_kit_flutter::FFmpegKitConfig::closeFFmpegPipe(pipePathString);

    // Create new pipe
    HANDLE pipe = CreateNamedPipeW(newFFmpegPipePath->c_str(), PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE,
        1, 65536, 65536, 0, NULL);

    if (pipe == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::cerr << "Failed to create named pipe. GetLastError=" << error << "." << std::endl;
        return nullptr;
    }

    // Start a thread to wait for client connection
    std::thread connectThread([pipe]() mutable {
        std::cout << "Waiting for client to connect to pipe..." << std::endl;
        if (ConnectNamedPipe(pipe, NULL)) {
            std::cout << "Client connected to pipe." << std::endl;
        }
        else {
            DWORD error = GetLastError();
            if (error != ERROR_PIPE_CONNECTED) {
                std::cerr << "Failed to connect pipe. GetLastError=" << error << "." << std::endl;
            }
            else {
                std::cerr << "Pipe already connected." << std::endl;
            }
        }
        // Close the pipe handle when done
        //CloseHandle(pipe);
        });
    connectThread.detach();

    // Return pipe path as string
    std::string result = converter.to_bytes(*newFFmpegPipePath);
    auto resultSharedPtr = std::make_shared<std::string>(result);
    return resultSharedPtr;
}


void ffmpeg_kit_flutter::FFmpegKitConfig::closeFFmpegPipe(const std::string& ffmpegPipePath) {
    std::wstring wPipePath(ffmpegPipePath.begin(), ffmpegPipePath.end());

    HANDLE hPipe = CreateFile(
        wPipePath.c_str(),
        GENERIC_READ | GENERIC_WRITE, 
        0,              
        NULL,         
        OPEN_EXISTING,  
        0,                   
        NULL); 

    if (hPipe == INVALID_HANDLE_VALUE) {
        return;
    }


    if (CloseHandle(hPipe)) {
        std::cout << "Pipe closed successfully." << std::endl;
    }
    else {
        std::cerr << "Error closing pipe handle: " << GetLastError() << std::endl;
    }
}

std::string ffmpeg_kit_flutter::FFmpegKitConfig::getVersion() {
    if (FFmpegKitConfig::isLTSBuild()) {
        return std::string("").append(FFmpegKitVersion).append("-lts");
    } else {
        return FFmpegKitVersion;
    }
}

bool ffmpeg_kit_flutter::FFmpegKitConfig::isLTSBuild() {
#if defined(FFMPEG_KIT_LTS)
    return true;
#else
    return false;
#endif
}


std::string ffmpeg_kit_flutter::FFmpegKitConfig::getBuildDate() {
    char buildDate[10];
    sprintf_s(buildDate, "%d", FFMPEG_KIT_BUILD_DATE);
    return std::string(buildDate);
}

int ffmpeg_kit_flutter::FFmpegKitConfig::setEnvironmentVariable(const std::string &variableName,
                                                                const std::string &variableValue) {
    //return setenv(variableName.c_str(), variableValue.c_str(), true);

    return 0;
}

//void ffmpeg_kit_flutter::FFmpegKitConfig::ignoreSignal(const ffmpeg_kit_flutter::Signal signal) {
//    if (signal == ffmpeg_kit_flutter::SignalQuit) {
//        handleSIGQUIT = 0;
//    } else if (signal == ffmpeg_kit_flutter::SignalInt) {
//        handleSIGINT = 0;
//    } else if (signal == ffmpeg_kit_flutter::SignalTerm) {
//        handleSIGTERM = 0;
//    } else if (signal == ffmpeg_kit_flutter::SignalXcpu) {
//        handleSIGXCPU = 0;
//    } else if (signal == ffmpeg_kit_flutter::SignalPipe) {
//        handleSIGPIPE = 0;
//    }
//}

void ffmpeg_kit_flutter::FFmpegKitConfig::ffmpegExecute(
        const std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> ffmpegSession) {
    ffmpegSession->startRunning();

    try {
        int returnCode = executeFFmpeg(ffmpegSession->getSessionId(),
                                       ffmpegSession->getArguments());
        ffmpegSession->complete(std::make_shared<ffmpeg_kit_flutter::ReturnCode>(returnCode));
    }
    catch (const std::exception &exception) {
        ffmpegSession->fail(exception.what());
        std::cout << "FFmpeg execute failed: "
                  << ffmpeg_kit_flutter::FFmpegKitConfig::argumentsToString(
                          ffmpegSession->getArguments()) << "." << exception.what() << std::endl;
    }
}

void ffmpeg_kit_flutter::FFmpegKitConfig::ffprobeExecute(
        const std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession> ffprobeSession) {
    ffprobeSession->startRunning();

    try {
        int returnCode = executeFFprobe(ffprobeSession->getSessionId(),
                                        ffprobeSession->getArguments());
        ffprobeSession->complete(std::make_shared<ffmpeg_kit_flutter::ReturnCode>(returnCode));
    } catch (const std::exception &exception) {
        ffprobeSession->fail(exception.what());
        std::cout << "FFprobe execute failed: "
                  << ffmpeg_kit_flutter::FFmpegKitConfig::argumentsToString(
                          ffprobeSession->getArguments()) << "." << exception.what() << std::endl;
    }
}

void ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationExecute(
        const std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession> mediaInformationSession,
        const int waitTimeout) {
    mediaInformationSession->startRunning();

    try {
        int returnCodeValue = executeFFprobe(mediaInformationSession->getSessionId(),
                                             mediaInformationSession->getArguments());
        auto returnCode = std::make_shared<ffmpeg_kit_flutter::ReturnCode>(returnCodeValue);
        mediaInformationSession->complete(returnCode);
        if (returnCode->isValueSuccess()) {
            auto allLogs = mediaInformationSession->getAllLogsWithTimeout(waitTimeout);
            std::string ffprobeJsonOutput;
            std::for_each(allLogs->cbegin(), allLogs->cend(),
                          [&](std::shared_ptr <ffmpeg_kit_flutter::Log> log) {
                              if (log->getLevel() == LevelAVLogStdErr) {
                                  ffprobeJsonOutput.append(log->getMessage());
                              }
                          });
            auto mediaInformation = ffmpeg_kit_flutter::MediaInformationJsonParser::fromWithError(
                    ffprobeJsonOutput.c_str());
            mediaInformationSession->setMediaInformation(mediaInformation);
        }
    } catch (const std::exception &exception) {
        mediaInformationSession->fail(exception.what());
        std::cout << "Get media information execute failed: "
                  << ffmpeg_kit_flutter::FFmpegKitConfig::argumentsToString(
                          mediaInformationSession->getArguments()) << "." << exception.what()
                  << std::endl;
    }
}

void ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFmpegExecute(
        const std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> ffmpegSession) {
    auto thread = std::thread([ffmpegSession]() {
        ffmpeg_kit_flutter::FFmpegKitConfig::ffmpegExecute(ffmpegSession);

        ffmpeg_kit_flutter::FFmpegSessionCompleteCallback completeCallback = ffmpegSession->getCompleteCallback();
        if (completeCallback != nullptr) {
            try {
                // NOTIFY SESSION CALLBACK DEFINED
                completeCallback(ffmpegSession);
            } catch (const std::exception &exception) {
                std::cout << "Exception thrown inside session complete callback. "
                          << exception.what() << std::endl;
            }
        }

        ffmpeg_kit_flutter::FFmpegSessionCompleteCallback globalFFmpegSessionCompleteCallback = ffmpeg_kit_flutter::FFmpegKitConfig::getFFmpegSessionCompleteCallback();
        if (globalFFmpegSessionCompleteCallback != nullptr) {
            try {
                // NOTIFY SESSION CALLBACK DEFINED
                globalFFmpegSessionCompleteCallback(ffmpegSession);
            } catch (const std::exception &exception) {
                std::cout << "Exception thrown inside global complete callback. "
                          << exception.what() << std::endl;
            }
        }
    });

    thread.detach();
}

void ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFprobeExecute(
        const std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession> ffprobeSession) {
    auto thread = std::thread([ffprobeSession]() {
        ffmpeg_kit_flutter::FFmpegKitConfig::ffprobeExecute(ffprobeSession);

        ffmpeg_kit_flutter::FFprobeSessionCompleteCallback completeCallback = ffprobeSession->getCompleteCallback();
        if (completeCallback != nullptr) {
            try {
                // NOTIFY SESSION CALLBACK DEFINED
                completeCallback(ffprobeSession);
            } catch (const std::exception &exception) {
                std::cout << "Exception thrown inside session complete callback. "
                          << exception.what() << std::endl;
            }
        }

        ffmpeg_kit_flutter::FFprobeSessionCompleteCallback globalFFprobeSessionCompleteCallback = ffmpeg_kit_flutter::FFmpegKitConfig::getFFprobeSessionCompleteCallback();
        if (globalFFprobeSessionCompleteCallback != nullptr) {
            try {
                // NOTIFY SESSION CALLBACK DEFINED
                globalFFprobeSessionCompleteCallback(ffprobeSession);
            } catch (const std::exception &exception) {
                std::cout << "Exception thrown inside global complete callback. "
                          << exception.what() << std::endl;
            }
        }
    });

    thread.detach();
}

void ffmpeg_kit_flutter::FFmpegKitConfig::asyncGetMediaInformationExecute(
        const std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession> mediaInformationSession,
        const int waitTimeout) {
    auto thread = std::thread([mediaInformationSession, waitTimeout]() {
        ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationExecute(mediaInformationSession,
                                                                        waitTimeout);

        ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback completeCallback = mediaInformationSession->getCompleteCallback();
        if (completeCallback != nullptr) {
            try {
                // NOTIFY SESSION CALLBACK DEFINED
                completeCallback(mediaInformationSession);
            } catch (const std::exception &exception) {
                std::cout << "Exception thrown inside session complete callback. "
                          << exception.what() << std::endl;
            }
        }

        ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback globalMediaInformationSessionCompleteCallback = ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationSessionCompleteCallback();
        if (globalMediaInformationSessionCompleteCallback != nullptr) {
            try {
                // NOTIFY SESSION CALLBACK DEFINED
                globalMediaInformationSessionCompleteCallback(mediaInformationSession);
            } catch (const std::exception &exception) {
                std::cout << "Exception thrown inside global complete callback. "
                          << exception.what() << std::endl;
            }
        }
    });

    thread.detach();
}

void ffmpeg_kit_flutter::FFmpegKitConfig::enableLogCallback(
        const ffmpeg_kit_flutter::LogCallback callback) {
    logCallback = callback;
}

void ffmpeg_kit_flutter::FFmpegKitConfig::enableStatisticsCallback(
        const ffmpeg_kit_flutter::StatisticsCallback callback) {
    statisticsCallback = callback;
}

void ffmpeg_kit_flutter::FFmpegKitConfig::enableFFmpegSessionCompleteCallback(
        const FFmpegSessionCompleteCallback completeCallback) {
    ffmpegSessionCompleteCallback = completeCallback;
}

ffmpeg_kit_flutter::FFmpegSessionCompleteCallback
ffmpeg_kit_flutter::FFmpegKitConfig::getFFmpegSessionCompleteCallback() {
    return ffmpegSessionCompleteCallback;
}

void ffmpeg_kit_flutter::FFmpegKitConfig::enableFFprobeSessionCompleteCallback(
        const FFprobeSessionCompleteCallback completeCallback) {
    ffprobeSessionCompleteCallback = completeCallback;
}

ffmpeg_kit_flutter::FFprobeSessionCompleteCallback
ffmpeg_kit_flutter::FFmpegKitConfig::getFFprobeSessionCompleteCallback() {
    return ffprobeSessionCompleteCallback;
}

void ffmpeg_kit_flutter::FFmpegKitConfig::enableMediaInformationSessionCompleteCallback(
        const MediaInformationSessionCompleteCallback completeCallback) {
    mediaInformationSessionCompleteCallback = completeCallback;
}

ffmpeg_kit_flutter::MediaInformationSessionCompleteCallback
ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationSessionCompleteCallback() {
    return mediaInformationSessionCompleteCallback;
}

ffmpeg_kit_flutter::Level ffmpeg_kit_flutter::FFmpegKitConfig::getLogLevel() {
    return static_cast<ffmpeg_kit_flutter::Level>(configuredLogLevel);
}

void ffmpeg_kit_flutter::FFmpegKitConfig::setLogLevel(const ffmpeg_kit_flutter::Level level) {
    configuredLogLevel = level;
}

std::string
ffmpeg_kit_flutter::FFmpegKitConfig::logLevelToString(const ffmpeg_kit_flutter::Level level) {
    switch (level) {
        case ffmpeg_kit_flutter::LevelAVLogStdErr:
            return "STDERR";
        case ffmpeg_kit_flutter::LevelAVLogTrace:
            return "TRACE";
        case ffmpeg_kit_flutter::LevelAVLogDebug:
            return "DEBUG";
        case ffmpeg_kit_flutter::LevelAVLogVerbose:
            return "VERBOSE";
        case ffmpeg_kit_flutter::LevelAVLogInfo:
            return "INFO";
        case ffmpeg_kit_flutter::LevelAVLogWarning:
            return "WARNING";
        case ffmpeg_kit_flutter::LevelAVLogError:
            return "ERROR";
        case ffmpeg_kit_flutter::LevelAVLogFatal:
            return "FATAL";
        case ffmpeg_kit_flutter::LevelAVLogPanic:
            return "PANIC";
        case ffmpeg_kit_flutter::LevelAVLogQuiet:
            return "QUIET";
        default:
            return "";
    }
}

int ffmpeg_kit_flutter::FFmpegKitConfig::getSessionHistorySize() {
    return sessionHistorySize;
}

void ffmpeg_kit_flutter::FFmpegKitConfig::setSessionHistorySize(const int newSessionHistorySize) {
    if (newSessionHistorySize >= SESSION_MAP_SIZE) {

        /*
         * THERE IS A HARD LIMIT ON THE NATIVE SIDE. HISTORY SIZE MUST BE SMALLER THAN SESSION_MAP_SIZE
         */
        throw std::runtime_error("Session history size must not exceed the hard limit!");
    } else if (newSessionHistorySize > 0) {
        sessionHistorySize = newSessionHistorySize;
        deleteExpiredSessions();
    }
}

std::shared_ptr <ffmpeg_kit_flutter::Session>
ffmpeg_kit_flutter::FFmpegKitConfig::getSession(const long sessionId) {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);
    lock.lock();

    auto session = sessionHistoryMap.find(sessionId);
    if (session != sessionHistoryMap.end()) {
        return session->second;
    } else {
        return nullptr;
    }
}

std::shared_ptr <ffmpeg_kit_flutter::Session>
ffmpeg_kit_flutter::FFmpegKitConfig::getLastSession() {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);
    lock.lock();

    return sessionHistoryList.front();
}

std::shared_ptr <ffmpeg_kit_flutter::Session>
ffmpeg_kit_flutter::FFmpegKitConfig::getLastCompletedSession() {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);

    lock.lock();

    for (auto rit = sessionHistoryList.rbegin(); rit != sessionHistoryList.rend(); ++rit) {
        auto session = *rit;
        if (session->getState() == SessionStateCompleted) {
            return session;
        }
    }

    return nullptr;
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Session>>>

ffmpeg_kit_flutter::FFmpegKitConfig::getSessions() {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);
    lock.lock();

    auto sessionHistoryListCopy = std::make_shared < std::list < std::shared_ptr <
    ffmpeg_kit_flutter::Session>>>(sessionHistoryList);

    lock.unlock();

    return sessionHistoryListCopy;
}

void ffmpeg_kit_flutter::FFmpegKitConfig::clearSessions() {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);
    lock.lock();

    sessionHistoryList.clear();
    sessionHistoryMap.clear();

    lock.unlock();
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::FFmpegSession>>>

ffmpeg_kit_flutter::FFmpegKitConfig::getFFmpegSessions() {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);
    const auto ffmpegSessions =
    std::make_shared < std::list < std::shared_ptr < ffmpeg_kit_flutter::FFmpegSession>>>();

    lock.lock();

    for (auto it = sessionHistoryList.begin(); it != sessionHistoryList.end(); ++it) {
        auto session = *it;
        if (session->isFFmpeg()) {
            ffmpegSessions->push_back(
                    std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(session));
        }
    }

    lock.unlock();

    return ffmpegSessions;
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::FFprobeSession>>>

ffmpeg_kit_flutter::FFmpegKitConfig::getFFprobeSessions() {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);
    const auto ffprobeSessions = std::make_shared < std::list < std::shared_ptr <
    ffmpeg_kit_flutter::FFprobeSession>>>();

    lock.lock();

    for (auto it = sessionHistoryList.begin(); it != sessionHistoryList.end(); ++it) {
        auto session = *it;
        if (session->isFFprobe()) {
            ffprobeSessions->push_back(
                    std::static_pointer_cast<ffmpeg_kit_flutter::FFprobeSession>(session));
        }
    }

    lock.unlock();

    return ffprobeSessions;
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::MediaInformationSession>>>

ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationSessions() {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);
    const auto mediaInformationSessions = std::make_shared < std::list < std::shared_ptr <
    ffmpeg_kit_flutter::MediaInformationSession>>>();

    lock.lock();

    for (auto it = sessionHistoryList.begin(); it != sessionHistoryList.end(); ++it) {
        auto session = *it;
        if (session->isMediaInformation()) {
            mediaInformationSessions->push_back(
                    std::static_pointer_cast<ffmpeg_kit_flutter::MediaInformationSession>(session));
        }
    }

    lock.unlock();

    return mediaInformationSessions;
}

std::shared_ptr <std::list<std::shared_ptr < ffmpeg_kit_flutter::Session>>>

ffmpeg_kit_flutter::FFmpegKitConfig::getSessionsByState(const SessionState state) {
    std::unique_lock <std::recursive_mutex> lock(sessionMutex, std::defer_lock);
    auto sessions =
    std::make_shared < std::list < std::shared_ptr < ffmpeg_kit_flutter::Session>>>();

    lock.lock();

    for (auto it = sessionHistoryList.begin(); it != sessionHistoryList.end(); ++it) {
        auto session = *it;
        if (session->getState() == state) {
            sessions->push_back(session);
        }
    }

    lock.unlock();

    return sessions;
}

ffmpeg_kit_flutter::LogRedirectionStrategy
ffmpeg_kit_flutter::FFmpegKitConfig::getLogRedirectionStrategy() {
    return globalLogRedirectionStrategy;
}

void ffmpeg_kit_flutter::FFmpegKitConfig::setLogRedirectionStrategy(
        const LogRedirectionStrategy logRedirectionStrategy) {
    globalLogRedirectionStrategy = logRedirectionStrategy;
}

int ffmpeg_kit_flutter::FFmpegKitConfig::messagesInTransmit(const long sessionId) {
    return std::atomic_load(&sessionInTransitMessageCountMap[sessionId % SESSION_MAP_SIZE]);
}

std::string ffmpeg_kit_flutter::FFmpegKitConfig::sessionStateToString(SessionState state) {
    switch (state) {
        case SessionStateCreated:
            return "CREATED";
        case SessionStateRunning:
            return "RUNNING";
        case SessionStateFailed:
            return "FAILED";
        case SessionStateCompleted:
            return "COMPLETED";
        default:
            return "";
    }
}

std::list <std::string>
ffmpeg_kit_flutter::FFmpegKitConfig::parseArguments(const std::string &command) {
    std::list <std::string> argumentList;
    std::string currentArgument;

    bool singleQuoteStarted = false;
    bool doubleQuoteStarted = false;

    for (int i = 0; i < command.size(); i++) {
        char previousChar;
        if (i > 0) {
            previousChar = command[i - 1];
        } else {
            previousChar = 0;
        }
        char currentChar = command[i];

        if (currentChar == ' ') {
            if (singleQuoteStarted || doubleQuoteStarted) {
                currentArgument += currentChar;
            } else if (currentArgument.size() > 0) {
                argumentList.push_back(currentArgument);
                currentArgument = "";
            }
        } else if (currentChar == '\'' && (previousChar == 0 || previousChar != '\\')) {
            if (singleQuoteStarted) {
                singleQuoteStarted = false;
            } else if (doubleQuoteStarted) {
                currentArgument += currentChar;
            } else {
                singleQuoteStarted = true;
            }
        } else if (currentChar == '\"' && (previousChar == 0 || previousChar != '\\')) {
            if (doubleQuoteStarted) {
                doubleQuoteStarted = false;
            } else if (singleQuoteStarted) {
                currentArgument += currentChar;
            } else {
                doubleQuoteStarted = true;
            }
        } else {
            currentArgument += currentChar;
        }
    }

    if (currentArgument.size() > 0) {
        argumentList.push_back(currentArgument);
    }

    return argumentList;
}

std::string ffmpeg_kit_flutter::FFmpegKitConfig::argumentsToString(
        std::shared_ptr <std::list<std::string>> arguments) {
    if (arguments == nullptr) {
        return "null";
    }

    std::string string;
    for (auto it = arguments->begin(); it != arguments->end(); ++it) {
        auto argument = *it;
        if (it != arguments->begin()) {
            string += " ";
        }
        string += argument;
    }

    return string;
}