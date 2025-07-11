import 'dart:io';
import 'package:ffmpeg_kit_flutter/ffmpeg_kit_config.dart';
import 'package:ffmpeg_kit_flutter/ffmpeg_session.dart';
import 'package:ffmpeg_kit_flutter/ffprobe_kit.dart';
import 'package:ffmpeg_kit_flutter/media_information.dart';
import 'package:ffmpeg_kit_flutter/media_information_session.dart';
import 'package:ffmpeg_kit_flutter/return_code.dart';
import 'package:ffmpeg_kit_flutter/session.dart';
import 'package:flutter/material.dart';
import 'package:ffmpeg_kit_flutter/ffmpeg_kit.dart';
import 'package:path/path.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:path_provider/path_provider.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(home: FFmpegExample());
  }
}

class FFmpegExample extends StatefulWidget {
  const FFmpegExample({super.key});

  @override
  State<FFmpegExample> createState() => _FFmpegExampleState();
}

class _FFmpegExampleState extends State<FFmpegExample> {
  String status = "Waiting for the operation to start...";

  Future<void> enableCallback() async {
    FFmpegKitConfig.enableStatistics();
  }

  Future<void> disableCallback() async {
    FFmpegKitConfig.disableStatistics();
  }

  Future<void> cancel() async {
    FFmpegKit.cancel();
  }

  Future<void> cancelSession() async {
    FFmpegKit.cancel(1);
  }

  Future<void> thumbnailGenerate() async {
    Session session = await FFmpegKit.execute(
      '-i "C:\\Users\\mbfakourii\\Documents\\FileTester\\aa.mp4" -ss 0.0 -frames:v 1 -q:v 3 C:\\Users\\MBFAKO~2\\AppData\\Local\\Temp/1746788523604.jpg',
    );
  }

  String exportPipePath = "";

  Future<void> pipeGenerate() async {
    exportPipePath = await FFmpegKitConfig.registerNewFFmpegPipe() ?? "";
    debugPrint("Generated pipe: $exportPipePath");
  }

  Future<void> pipeRemove() async {
    FFmpegKitConfig.closeFFmpegPipe(exportPipePath);
    exportPipePath = "";

    debugPrint("Removed pipe: $exportPipePath");
  }

  Future<void> ffprobe() async {
    MediaInformationSession session = await FFprobeKit.getMediaInformation(
      // "/storage/emulated/0/Download/aa.mp4",
      "C:\\Users\\mbfakourii\\Documents\\aa.mp4",
    );
    MediaInformation? information = session.getMediaInformation();
    if (information != null) {
      // debugPrint(double.tryParse(information.getDuration().toString()).toString());
      // debugPrint(information.getFilename());
      // debugPrint(information.getFormat());
      // debugPrint(information.getLongFormat());
      // debugPrint(information.getStartTime());
      // debugPrint(information.getSize());
      // debugPrint(information.getBitrate());
      // debugPrint(information.getTags().toString());
    }
  }

  Future<void> convertVideoToWav() async {
    setState(() {
      status = "Requesting access...";
    });

    var permission = await Permission.manageExternalStorage.request();
    // var permission = await Permission.storage.request();
    if (!permission.isGranted) {
      setState(() {
        status = "Access denied!";
      });

      return;
    }

    setState(() {
      status = "Access granted. Starting conversion...";
    });

    // String inputPath = "/storage/emulated/0/Download/aa.mp4";
    String inputPath = "C:\\Users\\mbfakourii\\Documents\\aa.mp4";
    // FFmpegKitConfig.enableStatistics();
    //
    // FFmpegKitConfig.enableStatisticsCallback((callback) {
    //   debugPrint("------------------------");
    //   debugPrint("Session Id: ${callback.getSessionId()}");
    //   debugPrint("Video Frame Number: ${callback.getVideoFrameNumber()}");
    //   debugPrint("Video FPS: ${callback.getVideoFps()}");
    //   debugPrint("Video Quality: ${callback.getVideoQuality()}");
    //   debugPrint("Size: ${callback.getSize()}");
    //   debugPrint("Time: ${callback.getTime()}");
    //   debugPrint("Bitrate: ${callback.getBitrate()}");
    //   debugPrint("Speed: ${callback.getSpeed()}");
    // });

    Directory appDir = await getApplicationDocumentsDirectory();
    String outputPath = join(appDir.path, "output.avi");

    String command = '-i "$inputPath" "$outputPath" -y';

    // await FFmpegKit.execute(command).then((session) async {
    //   final returnCode = await session.getReturnCode();
    //   if (ReturnCode.isSuccess(returnCode)) {
    //     setState(() {
    //       status = "Conversion was successful!\n$outputPath";
    //     });
    //   } else if (ReturnCode.isCancel(returnCode)) {
    //     setState(() {
    //       status = "Conversion Canceled.";
    //     });
    //
    //     debugPrint(await session.getAllLogsAsString());
    //   } else {
    //     setState(() {
    //       status = "Conversion failed.";
    //     });
    //
    //     debugPrint(await session.getAllLogsAsString());
    //   }
    // });
    debugPrint("Flag 1");
    FFmpegSession aa = await FFmpegKit.executeAsync(
      command,
      (session) async {
        final returnCode = await session.getReturnCode();
        if (ReturnCode.isSuccess(returnCode)) {
          setState(() {
            status = "Conversion was successful!\n$outputPath";
          });
        } else if (ReturnCode.isCancel(returnCode)) {
          setState(() {
            status = "Conversion Canceled.";
          });

          debugPrint(await session.getAllLogsAsString());
        } else {
          setState(() {
            status = "Conversion failed.";
          });

          // debugPrint(await session.getAllLogsAsString());
          debugPrint(await session.getLogsAsString());
        }
      },
      null,
      (statistics) {
        debugPrint("------------------------");
        debugPrint("Session Id: ${statistics.getSessionId()}");
        debugPrint("Video Frame Number: ${statistics.getVideoFrameNumber()}");
        debugPrint("Video FPS: ${statistics.getVideoFps()}");
        debugPrint("Video Quality: ${statistics.getVideoQuality()}");
        debugPrint("Size: ${statistics.getSize()}");
        debugPrint("Time: ${statistics.getTime()}");
        debugPrint("Bitrate: ${statistics.getBitrate()}");
        debugPrint("Speed: ${statistics.getSpeed()}");
      },
    );

    debugPrint(aa.getSessionId().toString());
    debugPrint("Flag 2");
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Convert video to WAV')),
      body: Center(
        child: Padding(
          padding: const EdgeInsets.all(16.0),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text(status, textAlign: TextAlign.center),
              SizedBox(height: 20),
              ElevatedButton(
                onPressed: convertVideoToWav,
                child: Text("Start"),
              ),

              ElevatedButton(
                onPressed: enableCallback,
                child: Text("Enable Callback"),
              ),

              ElevatedButton(
                onPressed: disableCallback,
                child: Text("Disable Callback"),
              ),

              ElevatedButton(onPressed: cancel, child: Text("Cancel")),

              ElevatedButton(
                onPressed: cancelSession,
                child: Text("Cancel session"),
              ),
              ElevatedButton(onPressed: ffprobe, child: Text("Ffprobe")),

              ElevatedButton(
                onPressed: thumbnailGenerate,
                child: Text("Test generate thumbnail"),
              ),

              ElevatedButton(
                onPressed: thumbnailGenerate,
                child: Text("Test generate thumbnail"),
              ),

              ElevatedButton(
                onPressed: pipeGenerate,
                child: Text("Pipe Generate"),
              ),

              ElevatedButton(
                onPressed: pipeRemove,
                child: Text("Pipe Removed"),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
