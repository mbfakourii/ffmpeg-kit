#include "ffmpeg_kit_flutter_plugin.h"
#include <windows.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <memory>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <iostream>
#include <chrono>
#include <map>
#include <thread>
#include <vector>
#include <string>
#include <typeinfo>
#include <variant>
#include <rapidjson/document.h>
#include "ReturnCode.h"
#include "AbstractSession.h"
#include "FFmpegSession.h"
#include "Packages.h"
#include "ArchDetect.h"
#include "FFmpegKitConfig.h"
#include "Statistics.h"
#include "FFmpegKit.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#define WM_PLATFORM_THREAD_EVENT WM_USER + 1

namespace ffmpeg_kit_flutter {
    using flutter::EncodableList;
    using flutter::EncodableMap;
    using flutter::EncodableValue;

    std::unique_ptr <flutter::MethodChannel<flutter::EncodableValue>> method_channel;
    std::unique_ptr <flutter::EventSink<flutter::EncodableValue>> event_sink;

    HWND platform_thread_hwnd;  // Hidden window handle for the main thread

    // Window procedure to process messages on the main thread
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_PLATFORM_THREAD_EVENT) {
            auto* sessionOut = reinterpret_cast<flutter::EncodableValue*>(lParam);
            if (event_sink) {
                event_sink->Success(*sessionOut);  // Safe to call on main thread
            }
            delete sessionOut;  // Clean up allocated memory
            return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    // Function to send events from background threads to the main thread
    void sendEventToPlatformThread(const flutter::EncodableValue& sessionOut) {
        auto* data = new flutter::EncodableValue(sessionOut);  // Allocate memory for the event
        PostMessage(platform_thread_hwnd, WM_PLATFORM_THREAD_EVENT, 0, reinterpret_cast<LPARAM>(data));
    }

    void FfmpegKitFlutterPlugin::RegisterWithRegistrar(
            flutter::PluginRegistrarWindows *registrar) {
        // Register the window class for the hidden window
        WNDCLASS wc = {0};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"PlatformThreadWindow";
        RegisterClass(&wc);

        // Create the hidden window
        platform_thread_hwnd = CreateWindowEx(0, L"PlatformThreadWindow", L"", 0, 0, 0, 0, 0,
                                              nullptr, nullptr, wc.hInstance, nullptr);

        // Initialize MethodChannel for command handling
        method_channel =
                std::make_unique < flutter::MethodChannel < flutter::EncodableValue >> (
                        registrar->messenger(), "flutter.arthenica.com/ffmpeg_kit",
                                &flutter::StandardMethodCodec::GetInstance());

        auto plugin = std::make_unique<FfmpegKitFlutterPlugin>();

        method_channel->SetMethodCallHandler(
                [plugin_pointer = plugin.get()](const auto &call, auto result) {
                    plugin_pointer->HandleMethodCall(call, std::move(result));
                });


        // Initialize EventChannel for log streaming
        auto event_channel = std::make_unique < flutter::EventChannel < flutter::EncodableValue >> (
                registrar->messenger(), "flutter.arthenica.com/ffmpeg_kit_event",
                        &flutter::StandardMethodCodec::GetInstance());

        event_channel->SetStreamHandler(
                std::make_unique < flutter::StreamHandlerFunctions < flutter::EncodableValue >> (
                        [](const flutter::EncodableValue *arguments,
                           std::unique_ptr <flutter::EventSink<flutter::EncodableValue>> &&sink) {
                            event_sink = std::move(sink);
                            return nullptr;
                        },
                                [](const flutter::EncodableValue *arguments) {
                                    event_sink.reset();
                                    return nullptr;
                                }));

        registrar->AddPlugin(std::move(plugin));
    }

    FfmpegKitFlutterPlugin::FfmpegKitFlutterPlugin() {
    }

    FfmpegKitFlutterPlugin::~FfmpegKitFlutterPlugin() {
        ffmpeg_kit_flutter::FFmpegKit::cancel();
    }

    flutter::EncodableValue convertJsonToEncodableValue(const rapidjson::Value &value) {
        if (value.IsObject()) {
            flutter::EncodableMap map;

            for (auto &m: value.GetObject()) {
                std::string key = m.name.GetString();

                if (m.value.IsString()) {
                    map[flutter::EncodableValue(key)] = flutter::EncodableValue(
                            m.value.GetString());
                } else if (m.value.IsInt()) {
                    map[flutter::EncodableValue(key)] = flutter::EncodableValue(m.value.GetInt());
                } else if (m.value.IsDouble()) {
                    map[flutter::EncodableValue(key)] = flutter::EncodableValue(
                            m.value.GetDouble());
                } else if (m.value.IsObject()) {
                    map[flutter::EncodableValue(key)] = flutter::EncodableValue(
                            convertJsonToEncodableValue(m.value));
                } else if (m.value.IsArray()) {
                    flutter::EncodableList list;
                    for (auto &item: m.value.GetArray()) {
                        if (item.IsString()) {
                            list.push_back(flutter::EncodableValue(item.GetString()));
                        } else if (item.IsInt()) {
                            list.push_back(flutter::EncodableValue(item.GetInt()));
                        } else if (item.IsDouble()) {
                            list.push_back(flutter::EncodableValue(item.GetDouble()));
                        } else if (item.IsObject() || item.IsArray()) {
                            list.push_back(convertJsonToEncodableValue(item));
                        }
                    }
                    map[flutter::EncodableValue(key)] = flutter::EncodableValue(list);
                }
            }
            return flutter::EncodableValue(map);
        }
        return flutter::EncodableValue();
    }


    void FfmpegKitFlutterPlugin::HandleMethodCall(
            const flutter::MethodCall <flutter::EncodableValue> &method_call,
            std::unique_ptr <flutter::MethodResult<flutter::EncodableValue>> result) {
        //std::cerr << method_call.method_name() << std::endl;
        if (method_call.method_name().compare("getLogLevel") == 0) {
            result->Success(
                    flutter::EncodableValue(ffmpeg_kit_flutter::FFmpegKitConfig::getLogLevel()));
        } else if (method_call.method_name().compare("setLogLevel") == 0) {
            result->Success();
        } else if (method_call.method_name().compare("getPlatform") == 0) {
            result->Success(flutter::EncodableValue("Windows"));
        } else if (method_call.method_name().compare("getPackageName") == 0) {
            result->Success(
                    flutter::EncodableValue(ffmpeg_kit_flutter::Packages::getPackageName()));
        } else if (method_call.method_name().compare("enableRedirection") == 0) {
            result->Success(flutter::EncodableValue(true));
        } else if (method_call.method_name().compare("isLTSBuild") == 0) {
            result->Success(flutter::EncodableValue(false));
        } else if (method_call.method_name().compare("getArch") == 0) {
            result->Success(flutter::EncodableValue(ffmpeg_kit_flutter::ArchDetect::getArch()));
        } else if (method_call.method_name().compare("getSession") == 0) {
            const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("sessionId"));
                if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                    long sessionId = std::get<int>(it->second);

                    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                            sessionId);

                    flutter::EncodableMap sessionMap;
                    sessionMap[flutter::EncodableValue("sessionId")] =
                            flutter::EncodableValue(session->getSessionId());
                    auto createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                            session->getCreateTime().time_since_epoch()).count();
                    sessionMap[flutter::EncodableValue("createTime")] =
                            flutter::EncodableValue(static_cast<int64_t>(createTime));
                    auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                            session->getStartTime().time_since_epoch()).count();
                    sessionMap[flutter::EncodableValue("startTime")] =
                            flutter::EncodableValue(static_cast<int64_t>(startTime));
                    sessionMap[flutter::EncodableValue("command")] =
                            flutter::EncodableValue(session->getCommand());

                    result->Success(flutter::EncodableValue(sessionMap));

                } else {
                    result->Error("INVALID_ARGUMENT", "Missing or invalid sessionId");
                    return;
                }
            } else {
                result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                return;
            }
        } else if (method_call.method_name().compare("ffmpegSession") == 0) {
            const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("arguments"));
                if (it != arguments->end()) {
                    const auto &argumentList = std::get<flutter::EncodableList>(it->second);

                    std::list <std::string> argumentsList;

                    for (const auto &value: argumentList) {
                        if (std::holds_alternative<std::string>(value)) {
                            argumentsList.push_back(std::get<std::string>(value));
                        }
                    }


                    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> session = ffmpeg_kit_flutter::FFmpegSession::create(
                            argumentsList,
                            [](const std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> session) {
                                flutter::EncodableMap sessionMap;
                                sessionMap[flutter::EncodableValue("sessionId")] =
                                        flutter::EncodableValue(session->getSessionId());

                                flutter::EncodableMap sessionOut;
                                sessionOut[flutter::EncodableValue(
                                        "FFmpegKitCompleteCallbackEvent")] = flutter::EncodableValue(
                                        sessionMap);

                                // Send event to the main thread
                                sendEventToPlatformThread(flutter::EncodableValue(sessionOut));
                            });

                    flutter::EncodableMap sessionMap;
                    sessionMap[flutter::EncodableValue("sessionId")] =
                            flutter::EncodableValue(session->getSessionId());
                    auto createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                            session->getCreateTime().time_since_epoch()).count();
                    sessionMap[flutter::EncodableValue("createTime")] =
                            flutter::EncodableValue(static_cast<int64_t>(createTime));
                    auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                            session->getStartTime().time_since_epoch()).count();
                    sessionMap[flutter::EncodableValue("startTime")] =
                            flutter::EncodableValue(static_cast<int64_t>(startTime));
                    sessionMap[flutter::EncodableValue("command")] =
                            flutter::EncodableValue(session->getCommand());

                    result->Success(flutter::EncodableValue(sessionMap));
            }
        }
    } else if (method_call.method_name().compare("ffprobeSession") == 0) {
        const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

        if (arguments) {
            auto it = arguments->find(flutter::EncodableValue("arguments"));
            if (it != arguments->end()) {
                const auto &argumentList = std::get<flutter::EncodableList>(it->second);

                std::list <std::string> argumentsList;

                for (const auto &value: argumentList) {
                    if (std::holds_alternative<std::string>(value)) {
                        argumentsList.push_back(std::get<std::string>(value));
                    }
                }

                std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession> session = ffmpeg_kit_flutter::FFprobeSession::create(
                        argumentsList,
                        [](const std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession> session) {
                            flutter::EncodableMap sessionMap;
                            sessionMap[flutter::EncodableValue("sessionId")] =
                                    flutter::EncodableValue(session->getSessionId());

                            flutter::EncodableMap sessionOut;
                            sessionOut[flutter::EncodableValue(
                                    "FFmpegKitCompleteCallbackEvent")] = flutter::EncodableValue(
                                    sessionMap);

                            // Send event to the main thread
                            sendEventToPlatformThread(flutter::EncodableValue(sessionOut));
                        });

                flutter::EncodableMap sessionMap;
                sessionMap[flutter::EncodableValue("sessionId")] =
                        flutter::EncodableValue(session->getSessionId());
                auto createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                        session->getCreateTime().time_since_epoch()).count();
                sessionMap[flutter::EncodableValue("createTime")] =
                        flutter::EncodableValue(static_cast<int64_t>(createTime));
                auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                        session->getStartTime().time_since_epoch()).count();
                sessionMap[flutter::EncodableValue("startTime")] =
                        flutter::EncodableValue(static_cast<int64_t>(startTime));
                sessionMap[flutter::EncodableValue("command")] =
                        flutter::EncodableValue(session->getCommand());

                result->Success(flutter::EncodableValue(sessionMap));
            }
        }
    } else if (method_call.method_name().compare("ffprobeSessionExecute") == 0) {
        const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

        if (arguments) {
            auto it = arguments->find(flutter::EncodableValue("sessionId"));
            if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                long sessionId = std::get<int>(it->second);

                std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                        sessionId);
                std::shared_ptr <ffmpeg_kit_flutter::FFprobeSession> ffprobeSession = std::static_pointer_cast<ffmpeg_kit_flutter::FFprobeSession>(
                        session);

                std::thread([ffprobeSession, result = std::move(result)]() mutable {
                    ffmpeg_kit_flutter::FFmpegKitConfig::ffprobeExecute(ffprobeSession);

                    result->Success();
                }).detach();
            } else {
                result->Error("INVALID_ARGUMENT", "Missing or invalid sessionId");
                return;
            }
        } else {
            result->Error("INVALID_ARGUMENT", "Arguments are not a map");
            return;
        }
    } else if (method_call.method_name().compare("mediaInformationSession") == 0) {
        const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("arguments"));
                if (it != arguments->end()) {
                    const auto &argumentList = std::get<flutter::EncodableList>(it->second);

                    std::list <std::string> argumentsList;

                    for (const auto &value: argumentList) {
                        if (std::holds_alternative<std::string>(value)) {
                            argumentsList.push_back(std::get<std::string>(value));
                        }
                    }

                    std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession> session = ffmpeg_kit_flutter::MediaInformationSession::create(
                            argumentsList);

                    flutter::EncodableMap sessionMap;
                    sessionMap[flutter::EncodableValue("sessionId")] =
                            flutter::EncodableValue(session->getSessionId());
                    auto createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                            session->getCreateTime().time_since_epoch()).count();
                    sessionMap[flutter::EncodableValue("createTime")] =
                            flutter::EncodableValue(static_cast<int64_t>(createTime));
                    auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                            session->getStartTime().time_since_epoch()).count();
                    sessionMap[flutter::EncodableValue("startTime")] =
                            flutter::EncodableValue(static_cast<int64_t>(startTime));
                    sessionMap[flutter::EncodableValue("command")] =
                            flutter::EncodableValue(session->getCommand());

                    result->Success(flutter::EncodableValue(sessionMap));
                }
            }
        } else if (method_call.method_name().compare("ffmpegSessionExecute") == 0) {
            const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("sessionId"));
                if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                    long sessionId = std::get<int>(it->second);

                    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                            sessionId);
                    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> ffmpegSession = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
                            session);

                    std::thread([ffmpegSession, result = std::move(result)]() mutable {
                        ffmpeg_kit_flutter::FFmpegKitConfig::ffmpegExecute(ffmpegSession);

                        result->Success();
                    }).detach();


                } else {
                    result->Error("INVALID_ARGUMENT", "Missing or invalid sessionId");
                    return;
                }
            } else {
                result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                return;
            }
        } else if (method_call.method_name().compare("getMediaInformation") == 0) {
            const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("sessionId"));
                if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                    long sessionId = std::get<int>(it->second);

                    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                            sessionId);
                    std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession> mediaInformationSession = std::static_pointer_cast<ffmpeg_kit_flutter::MediaInformationSession>(
                            session);
                    std::shared_ptr <ffmpeg_kit_flutter::MediaInformation> mediaInformation = mediaInformationSession->getMediaInformation();

                    auto allProperties = mediaInformation->getAllProperties();

                    // Only formatted has encoded
                    flutter::EncodableValue encodable_value = convertJsonToEncodableValue(
                            *allProperties);

                    result->Success(encodable_value);
                } else {
                    result->Error("INVALID_ARGUMENT", "Missing or invalid sessionId");
                    return;
                }
            } else {
                result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                return;
            }
        } else if (method_call.method_name().compare("mediaInformationSessionExecute") == 0) {
            const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("sessionId"));
                if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                    long sessionId = std::get<int>(it->second);

                    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                            sessionId);
                    std::shared_ptr <ffmpeg_kit_flutter::MediaInformationSession> mediaInformationSession = std::static_pointer_cast<ffmpeg_kit_flutter::MediaInformationSession>(
                            session);

                    // Monitor process completion and handle results
                    std::thread([mediaInformationSession, result = std::move(result)]() mutable {
                        ffmpeg_kit_flutter::FFmpegKitConfig::getMediaInformationExecute(
                                mediaInformationSession,
                                ffmpeg_kit_flutter::AbstractSession::DefaultTimeoutForAsynchronousMessagesInTransmit);

                        result->Success();
                    }).detach();


                } else {
                    result->Error("INVALID_ARGUMENT", "Missing or invalid sessionId");
                    return;
                }
            } else {
                result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                return;
            }
        } else if (method_call.method_name().compare("asyncFFmpegSessionExecute") == 0) {
            const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());

            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("sessionId"));
                if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                    long sessionId = std::get<int>(it->second);

                    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                            sessionId);
                    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> ffmpegSession = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
                            session);


                    ffmpeg_kit_flutter::FFmpegKitConfig::asyncFFmpegExecute(ffmpegSession);

                    result->Success();
                } else {
                    result->Error("INVALID_ARGUMENT", "Missing or invalid sessionId");
                    return;
                }
            } else {
                result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                return;
            }
        } else if (method_call.method_name().compare("abstractSessionGetReturnCode") == 0) {
            const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("sessionId"));
                if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                    long sessionId = std::get<int>(it->second);

                    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                            sessionId);
                    std::shared_ptr <ffmpeg_kit_flutter::FFmpegSession> ffmpegSession = std::static_pointer_cast<ffmpeg_kit_flutter::FFmpegSession>(
                            session);

                    if (ffmpegSession->getReturnCode() != nullptr) {
                        result->Success(
                                flutter::EncodableValue(
                                        ffmpegSession->getReturnCode()->getValue()));
                    } else {
                        result->Success(flutter::EncodableValue(-1));
                    }
                } else {
                    result->Error("INVALID_SESSION", "Invalid session id.");
                    return;
                }
            } else {
                result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                return;
            }
        } else if (method_call.method_name().compare("abstractSessionGetAllLogsAsString") == 0) {
            const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("sessionId"));
                auto waitTimeoutIt = arguments->find(flutter::EncodableValue("waitTimeout"));
                if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                    long sessionId = std::get<int>(it->second);

                    int waitTimeout = ffmpeg_kit_flutter::AbstractSession::DefaultTimeoutForAsynchronousMessagesInTransmit;
                    if (waitTimeoutIt != arguments->end() &&
                        std::holds_alternative<int>(waitTimeoutIt->second)) {
                        waitTimeout = std::get<int>(waitTimeoutIt->second);
                    }

                    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                            sessionId);

                    result->Success(flutter::EncodableValue(
                            session->getAllLogsAsStringWithTimeout(waitTimeout)));
                } else {
                    result->Error("INVALID_SESSION", "Invalid session id.");
                    return;
                }
            } else {
                result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                return;
            }
        } else if (method_call.method_name().compare("abstractSessionGetLogs") == 0) {
             const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
             if (arguments) {
                 auto it = arguments->find(flutter::EncodableValue("sessionId"));
                 auto waitTimeoutIt = arguments->find(flutter::EncodableValue("waitTimeout"));
                 if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                     long sessionId = std::get<int>(it->second);

                     int waitTimeout = ffmpeg_kit_flutter::AbstractSession::DefaultTimeoutForAsynchronousMessagesInTransmit;
                     if (waitTimeoutIt != arguments->end() &&
                         std::holds_alternative<int>(waitTimeoutIt->second)) {
                         waitTimeout = std::get<int>(waitTimeoutIt->second);
                     }

                     std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                         sessionId);

                    auto logsPtr = session->getLogs();
                    flutter::EncodableList encLogs;

                    for (const auto& log : *logsPtr) {
                        flutter::EncodableMap sessionMap;
                        sessionMap[flutter::EncodableValue("level")] = flutter::EncodableValue(log->getLevel());
                        sessionMap[flutter::EncodableValue("message")] = flutter::EncodableValue(log->getMessage());
                        sessionMap[flutter::EncodableValue("sessionId")] = flutter::EncodableValue(static_cast<int>(log->getSessionId()));

                        encLogs.push_back(flutter::EncodableValue(std::move(sessionMap)));
                    }

                    result->Success(flutter::EncodableValue(encLogs));
                 } else {
                     result->Error("INVALID_SESSION", "Invalid session id.");
                     return;
                 }
             }
             else {
                 result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                 return;
             }
        } else if (method_call.method_name().compare("enableStatistics") == 0) {
            ffmpeg_kit_flutter::FFmpegKitConfig::enableStatisticsCallback(
                    [](const std::shared_ptr <ffmpeg_kit_flutter::Statistics> statistics) {
                        flutter::EncodableMap details;
                        details[flutter::EncodableValue("sessionId")] = flutter::EncodableValue(
                                statistics->getSessionId());
                        details[flutter::EncodableValue(
                                "videoFrameNumber")] = flutter::EncodableValue(
                                statistics->getVideoFrameNumber());
                        details[flutter::EncodableValue("videoFps")] = flutter::EncodableValue(
                                statistics->getVideoFps());
                        details[flutter::EncodableValue("videoQuality")] = flutter::EncodableValue(
                                statistics->getVideoQuality());
                        details[flutter::EncodableValue("size")] = flutter::EncodableValue(
                                statistics->getSize());
                        details[flutter::EncodableValue("time")] = flutter::EncodableValue(
                                statistics->getTime());
                        details[flutter::EncodableValue("bitrate")] = flutter::EncodableValue(
                                statistics->getBitrate());
                        details[flutter::EncodableValue("speed")] = flutter::EncodableValue(
                                statistics->getSpeed());

                        flutter::EncodableMap statisticsMap;
                        statisticsMap[flutter::EncodableValue(
                                "FFmpegKitStatisticsCallbackEvent")] = flutter::EncodableValue(
                                details);

                        //event_sink->Success(flutter::EncodableValue(statisticsMap));
                        // Send event to the main thread
                        sendEventToPlatformThread(flutter::EncodableValue(statisticsMap));
                    });

            result->Success();
        } else if (method_call.method_name().compare("disableStatistics") == 0) {
            ffmpeg_kit_flutter::FFmpegKitConfig::enableStatisticsCallback(nullptr);

            result->Success();
        } else if (method_call.method_name().compare("closeFFmpegPipe") == 0) {
            const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("ffmpegPipePath"));
                if (it != arguments->end() && std::holds_alternative<std::string>(it->second)) {
                    std::string ffmpegPipePath = std::get<std::string>(it->second);

                    ffmpeg_kit_flutter::FFmpegKitConfig::closeFFmpegPipe(ffmpegPipePath);

                    result->Success();
                }
                else {
                    result->Error("INVALID_SESSION", "Invalid session id.");
                    return;
                }
            }
            else {
                result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                return;
            }
        } else if (method_call.method_name().compare("registerNewFFmpegPipe") == 0) {
            std::shared_ptr<std::string> pipePath=  ffmpeg_kit_flutter::FFmpegKitConfig::registerNewFFmpegPipe();

            result->Success(flutter::EncodableValue(*pipePath));
        } else if (method_call.method_name().compare("cancel") == 0) {
            ffmpeg_kit_flutter::FFmpegKit::cancel();

            result->Success();
        } else if (method_call.method_name().compare("cancelSession") == 0) {
            const auto *arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
            if (arguments) {
                auto it = arguments->find(flutter::EncodableValue("sessionId"));
                if (it != arguments->end() && std::holds_alternative<int>(it->second)) {
                    long sessionId = std::get<int>(it->second);

                    std::shared_ptr <ffmpeg_kit_flutter::Session> session = ffmpeg_kit_flutter::FFmpegKitConfig::getSession(
                            sessionId);

                    session->cancel();

                    result->Success();
                } else {
                    result->Error("INVALID_SESSION", "Invalid session id.");
                    return;
                }
            } else {
                result->Error("INVALID_ARGUMENT", "Arguments are not a map");
                return;
            }
        } else {
            result->NotImplemented();
        }
    }
}
