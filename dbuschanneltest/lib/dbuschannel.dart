import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class DBusChannel extends StatefulWidget {

  @override
  State<DBusChannel> createState() => _DBusChannelState();
}

class _DBusChannelState extends State<DBusChannel>{
  static const platform = MethodChannel('samples.flutter.dev/dbuschannel');

  String _batteryLevel = 'Unknown battery level.';
  Future<void> _getBatteryLevel() async {
    String batteryLevel;
    try {
      final result = await platform.invokeMethod<int>('getBatteryLevel');
      batteryLevel = 'Battery level at $result % .';
    } on PlatformException catch (e) {
      batteryLevel = "Failed to get battery level: '${e.message}'.";
    }

    setState(() {
      _batteryLevel = batteryLevel;
    });
  }
  @override
  Widget build(BuildContext c){
    return Row(
      children: [
        Text("$_batteryLevel"),
        FloatingActionButton(onPressed: (){
          _getBatteryLevel();
        })
      ],
    );
  }
}