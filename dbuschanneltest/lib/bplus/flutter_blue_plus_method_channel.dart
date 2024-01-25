import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'flutter_blue_plus_platform_interface.dart';

/// An implementation of [FlutterBluePlusPlatform] that uses method channels.
class MethodChannelFlutterBluePlus extends FlutterBluePlusPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('samples.flutter.dev/dbuschannel');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
