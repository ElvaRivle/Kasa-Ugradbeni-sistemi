import 'dart:async';
import 'dart:typed_data';
import 'dart:ffi';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_barcode_scanner/flutter_barcode_scanner.dart';
import 'package:mqtt_client/mqtt_server_client.dart' as mqtt;
import 'package:mqtt_client/mqtt_client.dart' as mqttClient;
import 'dart:developer';

void main() {
  runApp(MaterialApp(
    title: "Kasa",
    home: HomeScreenWidget()
  ));
}

class HomeScreenWidget extends StatefulWidget {
  HomeScreenWidget();

  @override
  HomeScreenState createState() => HomeScreenState();
}

class HomeScreenState extends State<HomeScreenWidget> {
  String _scanBarcode = '';
  String successMessage = '';
  final mqtt.MqttServerClient client;
  
  HomeScreenState() : client = mqtt.MqttServerClient("192.168.0.6", "Telefon") {
    _connect();
  }

    //client.connect je async
  //mada cemo je sinhrono pozivati!
  void _connect() {
    client.useWebSocket = false;
    client.port = 1883;
    client.logging(on: false);
    client.keepAlivePeriod = 60;
    client.onDisconnected = onDisconnected;
    client.onConnected = onConnected;
    client.secure = false;
    client.autoReconnect = true;
    client.setProtocolV311();
    client.resubscribeOnAutoReconnect = true;

    final connMess = mqttClient.MqttConnectMessage()
        .withClientIdentifier("Telefon")
        .startClean() // Non persistent session for testing
        .withWillQos(mqttClient.MqttQos.atMostOnce);
    client.connectionMessage = connMess;

    try {
      client.connect();
    } catch (e) {
      client.disconnect();
    }

    //if (client.connectionStatus!.state != mqttClient.MqttConnectionState.connected) {
    //
    //  client.disconnect();
    //}
  }

  /// The unsolicited disconnect callback
  void onDisconnected() {

  }

  /// The successful connect callback
  void onConnected() {

  }


  //ne treba ovdje future jer nista nije async
  Future<void> SendBarCode() async {
    try {

      const pubTopic = 'tema';
      final builder = mqttClient.MqttClientPayloadBuilder();
      builder.addString(_scanBarcode);

      /// Subscribe to it

      this.client.subscribe(pubTopic, mqttClient.MqttQos.atMostOnce);

      /// Publish it

      this.client.publishMessage(
          pubTopic, mqttClient.MqttQos.atMostOnce, builder.payload!);

      setState(() {
        successMessage = "Uspjesno slanje barkoda.";
      });
    }
    on Exception catch(e) {
      setState(() {
        successMessage = "Neuspjesno slanje barkoda.";
      });
    }
  }


  @override
  void initState() {
    super.initState();
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
    return Scaffold(
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
                                scanBarcodeNormal();
                              },
                              style: ElevatedButton.styleFrom(
                                  backgroundColor: Colors.lightBlue.shade400
                              ),
                              child: const Text('Skeniranje barkoda')),
                          Text('Barkod: $_scanBarcode\n',
                              style: const TextStyle(fontSize: 20)),
                          ElevatedButton(
                              onPressed: () => SendBarCode(),
                              style: ElevatedButton.styleFrom(
                                //backgroundColor: Colors.green
                              ),
                              child: const Text("Slanje barkoda", style: TextStyle(fontWeight: FontWeight.bold),)
                          ),
                          Text('$successMessage\n',
                          style: const TextStyle(fontSize: 20))
                        ]));
              }));
  }
}