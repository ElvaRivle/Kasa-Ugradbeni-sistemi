import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_barcode_scanner/flutter_barcode_scanner.dart';

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

  @override
  void initState() {
    super.initState();
  }

  Future<void> startBarcodeScanStream() async {
    FlutterBarcodeScanner.getBarcodeStreamReceiver(
        '#ff6666', 'Otkazi', true, ScanMode.BARCODE)!
        .listen((barcode) => print(barcode));
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> scanBarcodeNormal() async {
    String barcodeScanRes;
    // Platform messages may fail, so we use a try/catch PlatformException.
    try {
      barcodeScanRes = await FlutterBarcodeScanner.scanBarcode(
          '#ff6666', 'Otkazi', true, ScanMode.BARCODE);
      print(barcodeScanRes);
    } on PlatformException {
      barcodeScanRes = 'Failed to get platform version.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _scanBarcode = barcodeScanRes;
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
                              onPressed: () => scanBarcodeNormal(),
                              child: Text('Skeniranje pojedinacnih artikala')),
                          ElevatedButton(
                              onPressed: () => startBarcodeScanStream(),
                              child: Text('Neprekidno skeniranje artikala')),
                          Text('Barkod: $_scanBarcode\n',
                              style: TextStyle(fontSize: 20))
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
