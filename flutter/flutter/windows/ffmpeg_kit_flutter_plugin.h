#ifndef FLUTTER_PLUGIN_FFMPEG_KIT_FLUTTER_PLUGIN_H_
#define FLUTTER_PLUGIN_FFMPEG_KIT_FLUTTER_PLUGIN_H_
#define NOMINMAX // This undef for fix conflict with <windows.h> in rapidjson
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>

namespace ffmpeg_kit_flutter {

class FfmpegKitFlutterPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FfmpegKitFlutterPlugin();

  virtual ~FfmpegKitFlutterPlugin();

  // Disallow copy and assign.
  FfmpegKitFlutterPlugin(const FfmpegKitFlutterPlugin&) = delete;
  FfmpegKitFlutterPlugin& operator=(const FfmpegKitFlutterPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

}  // namespace ffmpeg_kit_flutter

#endif  // FLUTTER_PLUGIN_FFMPEG_KIT_FLUTTER_PLUGIN_H_
