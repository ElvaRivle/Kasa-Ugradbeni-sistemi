import 'dart:async';
import 'dart:typed_data';
import 'dart:ffi';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_barcode_scanner/flutter_barcode_scanner.dart';
import 'package:flutter_beep/flutter_beep.dart';
import 'package:mqtt_client/mqtt_server_client.dart' as mqtt;
import 'package:mqtt_client/mqtt_client.dart' as mqttClient;

void main() {
  runApp(MaterialApp(
    title: "Kasa - US",
    home: HomeScreenWidget()
  ));
}

class HomeScreenWidget extends StatelessWidget {
  HomeScreenWidget({super.key});

  final mqtt.MqttServerClient client = mqtt.MqttServerClient("broker.hivemq.com", 'US_Projekat_Kasa_18702');

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
        .withClientIdentifier('US_Projekat_Kasa_18702')
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
                  onPressed: () {
                    if (client.connectionStatus!.state != mqttClient.MqttConnectionState.connected) {
                      _connect();
                    }
                    Navigator.push(context, MaterialPageRoute(builder: (context) => NormalModeWidget(client: client)));
                  },
                  child: const Text('Rezim normalnog rada')
              ),
              ElevatedButton(
                  onPressed: () {
                    if (client.connectionStatus!.state != mqttClient.MqttConnectionState.connected) {
                      _connect();
                    }
                    Navigator.push(context, MaterialPageRoute(builder: (context) => InsertModeWidget(client: client)));
                  },
                  child: Text('Rezim unosa artikala')
              ),
              ElevatedButton(
                  onPressed: () {
                    if (client.connectionStatus!.state != mqttClient.MqttConnectionState.connected) {
                      _connect();
                    }
                    Navigator.push(context, MaterialPageRoute(builder: (context) => DeleteModeWidget(client: client)));
                  },
                  child: const Text("Rezim brisanja artikala")
              )
            ]
          )
        )
      );
  }

}

class NormalModeWidget extends StatefulWidget {
  const NormalModeWidget({Key? key, required this.client}) : super(key: key);

  final mqtt.MqttServerClient client;

  @override
  NormalModeState createState() => NormalModeState();
}

class NormalModeState extends State<NormalModeWidget> {
  String _scanBarcode = '';
  String successMessage = '';


  //ne treba ovdje future jer nista nije async
  Future<void> SendBarCode() async {
    try {

      const pubTopic = 'projekatkasa/kasa/kod';
      final builder = mqttClient.MqttClientPayloadBuilder();
      builder.addString(_scanBarcode);

      /// Subscribe to it

      widget.client.subscribe(pubTopic, mqttClient.MqttQos.atMostOnce);

      /// Publish it

      widget.client.publishMessage(
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
        .listen((barcode) {
          if (barcode != _scanBarcode && barcode.toString().length > 5 && barcode.toString().length < 15) {
            //sleep(Duration(seconds: 1));
            //FlutterBeep.beep();

            setState(() {
              _scanBarcode = barcode.toString();
              successMessage = "";
            });
            SendBarCode();
          }
    });
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
                              child: const Text("Slanje skeniranog barkoda")
                          ),
                          Text('$successMessage\n',
                          style: const TextStyle(fontSize: 20))
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

///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class InsertModeWidget extends StatefulWidget {
  const InsertModeWidget({Key? key, required this.client}) : super(key: key);

  final mqtt.MqttServerClient client;

  @override
  InsertModeState createState() => InsertModeState();
}

class InsertModeState extends State<InsertModeWidget> {
  String _scanBarcode = '';
  String successMessage = '';

  final textController = TextEditingController();
  final priceController = TextEditingController();

  @override
  void dispose() {
    textController.dispose();
    priceController.dispose();
    //smijemo li pozvati super.dispose() da se ne ukine konekcija na broker
    //mada sam se osigurao u home ruti da ako konekcija nije uspostavljena da je uspostavi
  }

  Future<void> SendAll() async {
    try {

      String pubTopic = 'projekatkasa/kasa/unos';
      final builder = mqttClient.MqttClientPayloadBuilder();
      builder.addString('$_scanBarcode,${textController.text},${priceController.text}');
      /// Subscribe to it
      widget.client.subscribe(pubTopic, mqttClient.MqttQos.atMostOnce);
      /// Publish it
      widget.client.publishMessage(
          pubTopic, mqttClient.MqttQos.atMostOnce, builder.payload!);

      setState(() {
        successMessage = "Uspjesno slanje artikla.";
      });
    }
    on Exception {
      setState(() {
        successMessage = "Neuspjesno slanje artikla.";
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
                appBar: AppBar(title: const Text('Unos artikla')),
                body: Builder(builder: (BuildContext context) {
                  return Container(
                      alignment: Alignment.center,
                      child: Flex(
                          direction: Axis.vertical,
                          mainAxisAlignment: MainAxisAlignment.center,
                          children: <Widget>[
                            TextField(
                              controller: textController,
                              autofocus: true,
                              decoration: const InputDecoration(
                                hintText: "Unesite naziv artikla"
                              ),
                            ),
                            TextField(
                              controller: priceController,
                              autofocus: false,
                              decoration: const InputDecoration(
                                  hintText: "Unesite cijenu artikla"
                              ),
                            ),
                            ElevatedButton(
                                onPressed: () => scanBarcodeNormal(),
                                child: const Text('Unesite barkod artikla')),
                            Text('Barkod: $_scanBarcode\n',
                                style: const TextStyle(fontSize: 20)),
                            ElevatedButton(
                                onPressed: () => SendAll(),
                                child: const Text("Slanje artikla")
                            ),
                            Text('$successMessage\n',
                                style: const TextStyle(fontSize: 20))
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

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

class DeleteModeWidget extends StatefulWidget {
  const DeleteModeWidget({Key? key, required this.client}) : super(key: key);

  final mqtt.MqttServerClient client;

  @override
  DeleteModeState createState() => DeleteModeState();
}

class DeleteModeState extends State<DeleteModeWidget> {
  String _scanBarcode = '';
  String successMessage = '';


  //ne treba ovdje future jer nista nije async
  Future<void> SendBarCode() async {
    try {

      const pubTopic = 'projekatkasa/kasa/brisanje';
      final builder = mqttClient.MqttClientPayloadBuilder();
      builder.addString(_scanBarcode);

      /// Subscribe to it

      widget.client.subscribe(pubTopic, mqttClient.MqttQos.atMostOnce);

      /// Publish it

      widget.client.publishMessage(
          pubTopic, mqttClient.MqttQos.atMostOnce, builder.payload!);

      setState(() {
        successMessage = "Uspjesno brisanje artikla.";
      });
    }
    on Exception {
      setState(() {
        successMessage = "Neuspjesno brisanje artikla.";
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
    return WillPopScope(
        child:
        MaterialApp(
            home: Scaffold(
                appBar: AppBar(title: const Text('Brisanje artikla')),
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
                                child: const Text('Skeniranje barkoda')),
                            Text('Barkod: $_scanBarcode\n',
                                style: const TextStyle(fontSize: 20)),
                            ElevatedButton(
                                onPressed: () => SendBarCode(),
                                child: const Text("Slanje skeniranog barkoda")
                            ),
                            Text('$successMessage\n',
                                style: const TextStyle(fontSize: 20))
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