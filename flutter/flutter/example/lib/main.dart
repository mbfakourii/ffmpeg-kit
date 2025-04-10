import 'dart:io';
import 'package:ffmpeg_kit_flutter/return_code.dart';
import 'package:flutter/material.dart';
import 'package:ffmpeg_kit_flutter/ffmpeg_kit.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:path_provider/path_provider.dart';
import 'package:path/path.dart' as p;

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: FFmpegExample(),
    );
  }
}

class FFmpegExample extends StatefulWidget {
  @override
  _FFmpegExampleState createState() => _FFmpegExampleState();
}

class _FFmpegExampleState extends State<FFmpegExample> {
  String status = "منتظر شروع عملیات...";

  Future<void> convertVideoToWav() async {
    setState(() {
      status = "درخواست دسترسی‌ها...";
    });

    // // // درخواست دسترسی
    // // var permission = await Permission.storage.request();
    // // if (!permission.isGranted) {
    // //   setState(() {
    // //     status = "دسترسی رد شد!";
    // //   });
    // //   return;
    // // }
    // //
    // // setState(() {
    //   status = "دسترسی پذیرفته شد. شروع تبدیل...";
    // });

    // مسیر فایل ورودی
    String inputPath = "/storage/emulated/0/Download/aa.mp4";
    //
    // // مسیر خروجی
    // Directory appDir = await getApplicationDocumentsDirectory();
    String outputPath = "";

    // اجرای ffmpeg
    String command = '-i "$inputPath" "$outputPath"';

    await FFmpegKit.execute(command).then((session) async {
      final returnCode = await session.getReturnCode();
      if (ReturnCode.isSuccess(returnCode)) {
        setState(() {
          status = "تبدیل با موفقیت انجام شد!\n$outputPath";
        });
      } else {
        setState(() {
          status = "تبدیل ناموفق بود.";
        });
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('تبدیل ویدیو به WAV')),
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
                child: Text("شروع تبدیل"),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
