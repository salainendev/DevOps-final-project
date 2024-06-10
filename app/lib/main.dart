import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:permission_handler/permission_handler.dart';
import 'bluetooth_devices_list.dart';

void main() {
  runApp( MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});
  @override
  Widget build(BuildContext context) {
    return const MaterialApp(
      home: MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key});


  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> with SingleTickerProviderStateMixin {




  @override
  void initState() {
    super.initState();
    var status = Permission.bluetoothScan.request();
  }



  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('умная вытяжка',style: TextStyle(color: Colors.white),),
        backgroundColor: const Color.fromARGB(255, 48, 48, 47),
      ),
     
      body:
          Container(
            child: Scaffold(
              
              extendBody: true,
              backgroundColor: const Color.fromARGB(255, 126, 126, 126),
              floatingActionButtonLocation: FloatingActionButtonLocation.endFloat,
              floatingActionButton:FloatingActionButton(
                onPressed:() {
                  
                },
                backgroundColor:const Color.fromARGB(255, 167, 207, 169),
                child: const Icon(Icons.add),
                
                ),
                    
              body: Column(
                  children:<Widget>[
                    Expanded(
                      child: BluetoothDeviceList()
                    ),
                  ]  
              ),
            ),
          ),
    );
  }
}