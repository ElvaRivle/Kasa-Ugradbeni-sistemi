import 'dart:async';
import 'dart:typed_data';
import 'dart:ffi';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_barcode_scanner/flutter_barcode_scanner.dart';
import 'package:mqtt_client/mqtt_server_client.dart' as mqtt;
import 'package:mqtt_client/mqtt_client.dart' as mqttClient;

void main() {
  runApp(const MaterialApp(
    title: "Kasa - US",
    home: HomeScreenWidget()
  ));
}

class HomeScreenWidget extends StatelessWidget {
  const HomeScreenWidget({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("Kasa - US")
      ),
      body: Container(
          alignment: Alignment.center,
          child: Flex(
            direction: Axis.vertical,
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              ElevatedButton(
                  onPressed: () => {
                    Navigator.push(context, MaterialPageRoute(builder: (context) => NormalModeWidget()))
                  },
                  child: const Text('Rezim normalnog rada')
              ),
              ElevatedButton(
                  onPressed: () => {},
                  child: Text('Rezim unosa artikala')
              ),
              ElevatedButton(
                  onPressed: () => {}, 
                  child: const Text("Rezim brisanja artikla")
              )
            ]
          )
        )
      );
  }

}

class NormalModeWidget extends StatefulWidget {
  const NormalModeWidget({Key? key}) : super(key: key);

  @override
  NormalModeState createState() => NormalModeState();
}

class NormalModeState extends State<NormalModeWidget> {
  String _scanBarcode = '';
  String successMessage = '';

  int port                = 1883;
  String username         = '';
  String passwd           = '';

  final mqtt.MqttServerClient client = mqtt.MqttServerClient("broker.hivemq.com", 'US_Projekat_Kasa_18702');

  StreamSubscription? subscription;

  //client.connect je async
  //mada cemo je sinhrono pozivati!
  void _connect() {
    client.useWebSocket = false;
    client.port = 1883;
    client.logging(on: true);
    client.keepAlivePeriod = 60;
    client.onDisconnected = onDisconnected;
    client.onConnected = onConnected;
    client.secure = false;
    client.autoReconnect = true;
    client.setProtocolV311();
    client.resubscribeOnAutoReconnect = true;

    final connMess = mqttClient.MqttConnectMessage()
        .withClientIdentifier('US_Projekat_Kasa_18702')
        .startClean() // Non persistent session for testing
        .withWillQos(mqttClient.MqttQos.atMostOnce);
    client.connectionMessage = connMess;

    try {
      print("SPAJANJE!!!!");
      client.connect();
      print("USPJESNO SPAJANJE!!!!");
    } catch (e) {
      print(e);
      client.disconnect();
      print("PAO AWAIT CONNECT :( :( :( :(");
    }
    
    //if (client.connectionStatus!.state != mqttClient.MqttConnectionState.connected) {
    //  print("STATUS NIJE CONNECTED!!!!!!");
    //  client.disconnect();
    //}
  }

  /// The unsolicited disconnect callback
  void onDisconnected() {
    print(
        'ONDISCONNECT!!!!');
  }

  /// The successful connect callback
  void onConnected() {
    print(
        'ONCONNECT!!!!!');
  }
//TODO: OBRISATI PRINTOVI, STAVITI PORUKU USPJESNOG SLANJA
  Future<void> SendBarCode() async {
    try {
      print("SALJEMO!!!!!!");
      const pubTopic = 'US/Kasa/BarKod';
      final builder = mqttClient.MqttClientPayloadBuilder();
      builder.addString(_scanBarcode);

      /// Subscribe to it
      print('SUB!!!!!!!');
      client.subscribe(pubTopic, mqttClient.MqttQos.atMostOnce);

      /// Publish it
      print('PUBLISH!!!!!!!');
      client.publishMessage(
          pubTopic, mqttClient.MqttQos.atMostOnce, builder.payload!);

      setState(() {
        successMessage = "Uspjesno slanje barkoda.";
      });
    }
    on Exception {
      setState(() {
        successMessage = "Neuspjesno slanje barkoda.";
      });
    }
  }



  @override
  void initState() {
    super.initState();
  }

  Future<void> startBarcodeScanStream() async {
    FlutterBarcodeScanner.getBarcodeStreamReceiver(
        '#ff6666', 'Otkazi', true, ScanMode.BARCODE)!
        .listen((barcode) => {});
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> scanBarcodeNormal() async {
    String barcodeScanRes;
    // Platform messages may fail, so we use a try/catch PlatformException.
    try {
      barcodeScanRes = await FlutterBarcodeScanner.scanBarcode(
          '#ff6666', 'Otkazi', true, ScanMode.BARCODE);
    } on PlatformException {
      barcodeScanRes = 'Failed to get platform version.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _scanBarcode = barcodeScanRes;
      successMessage = "";
    });
  }

  @override
  Widget build(BuildContext context) {
    return WillPopScope(
      child:
        MaterialApp(
          home: Scaffold(
              appBar: AppBar(title: const Text('Barkod skener')),
              body: Builder(builder: (BuildContext context) {
                return Container(
                    alignment: Alignment.center,
                    child: Flex(
                        direction: Axis.vertical,
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: <Widget>[
                          ElevatedButton(
                              onPressed: () {
                                _connect();
                                scanBarcodeNormal();
                              },
                              child: const Text('Skeniranje pojedinacnih artikala')),
                          ElevatedButton(
                              onPressed: () => startBarcodeScanStream(),
                              child: const Text('Neprekidno skeniranje artikala')),
                          Text('Barkod: $_scanBarcode\n',
                              style: const TextStyle(fontSize: 20)),
                          ElevatedButton(
                              onPressed: () => SendBarCode(),
                              child: const Text("Posalji skenirani barkod")
                          ),
                        ]));
              }))
        ),
      onWillPop: () async {
        Navigator.pop(context);
        return false;
      }
    );
  }
}
