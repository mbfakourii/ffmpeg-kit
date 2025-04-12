import 'dart:io';
import 'package:ffmpeg_kit_flutter/return_code.dart';
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

  Future<void> convertVideoToWav() async {
    setState(() {
      status = "Requesting access...";
    });

    var permission = await Permission.storage.request();
    if (!permission.isGranted) {
      setState(() {
        status = "Access denied!";
      });

      return;
    }

    setState(() {
      status = "Access granted. Starting conversion...";
    });

    String inputPath = "/storage/emulated/0/Download/aa.mp4";

    Directory appDir = await getApplicationDocumentsDirectory();
    String outputPath = join(appDir.path, "output.wav");

    String command = '-i "$inputPath" "$outputPath"';

    await FFmpegKit.execute(command).then((session) async {
      final returnCode = await session.getReturnCode();
      if (ReturnCode.isSuccess(returnCode)) {
        setState(() {
          status = "Conversion was successful!\n$outputPath";
        });
      } else {
        setState(() {
          status = "Conversion failed.";
        });

        debugPrint(await session.getAllLogsAsString());
      }
    });
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
                child: Text("Start conversion"),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
