import 'dart:typed_data';
import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:http/http.dart' as http;
import 'package:shared_preferences/shared_preferences.dart';
import 'package:wifi_info_flutter/wifi_info_flutter.dart';
class DeviceSettingsScreen extends StatefulWidget {
  final BluetoothDevice device;
  DeviceSettingsScreen({required this.device});

  @override
  State<DeviceSettingsScreen> createState() => _DeviceSettingsScreenState();
}

class _DeviceSettingsScreenState extends State<DeviceSettingsScreen> {
  bool _isConnected = false;
  double _powerValue = 0;
  bool _switchValue = false;
  

    Future<void> _showInputDialog(BuildContext context, Function(String) onSubmitted) async {
    TextEditingController _textFieldController = TextEditingController();

    await showDialog<void>(
      context: context,
      barrierDismissible: false, // пользователь должен нажать на кнопку!
      builder: (BuildContext context) {
        return AlertDialog(
          title: Text('Enter wifi password'),
          content: TextField(
            controller: _textFieldController,
            decoration: InputDecoration(hintText: "Enter data here"),
          ),
          actions: <Widget>[
            TextButton(
              child: Text('Save'),
              onPressed: () {
                String enteredData = _textFieldController.text;
                Navigator.of(context).pop(); // Закрываем диалог
                onSubmitted(enteredData); // Передаем введенные данные
              },
            ),
          ],
        );
      },
    );
  }
  //get запрос мощность
  Future<void> _powerQuery(int value, String macAddr) async {

    try {
      SharedPreferences prefs = await SharedPreferences.getInstance();
      String? ipAddr = prefs.getString(macAddr);
      final url = Uri.https(ipAddr.toString(),"/power_$value");
      final response = await http.get(url);
      
    }
    catch(e){
      print(e);
    }
  }

Future<void> _ledQuery(bool state,String macAddr) async {

    try{
      SharedPreferences prefs = await SharedPreferences.getInstance();
      String? ipAddr = prefs.getString(macAddr);
      if (state == true){
        final url = Uri.https(ipAddr.toString(),"/led_on");
        final response = await http.get(url);
      }
      else {
        final url =Uri.https(ipAddr.toString(),"/led_off");
        final response = await http.get(url);
      }
    }
    catch(e){
      print(e);
    }


}

// функция представляет собой коннект с платой, передача и принятие ascii кодов, передача пароля , ssid , пароля от wifi
  void _connectToDevice(BluetoothDevice device, String ssid) async {
    try {
      
      final connection = await BluetoothConnection.toAddress(device.address);
      print('Connected to the device');

      connection.input!.listen((data) async {
        String receivedData = ascii.decode(data);

        // После получения первого сообщения отправляем пароль
        String password = "1641";
        connection.output.add(ascii.encode(password + "\r\n"));
        await connection.output.allSent;
        
        // Слушаем последующие данные от устройства
        connection.input!.listen((data) async {
          // true password
          String receivedData = ascii.decode(data);
          
          connection.input!.listen((data) async {
            // ssid
            String receivedData = ascii.decode(data);
            String ssid = WifiInfo().getWifiName().toString();
            connection.output.add(ascii.encode(ssid + "\r\n"));
            await connection.output.allSent;
            
            connection.input!.listen((data) async {
              // wifi password
              String receivedData = ascii.decode(data);
              SharedPreferences prefs = await SharedPreferences.getInstance();
              
              String? wifiPassword = await prefs.getString(ssid);
              connection.output.add(ascii.encode(wifiPassword.toString() + "\r\n"));
              await connection.output.allSent;
              
              connection.input!.listen((data) async{
                // connected
                String receivedData = ascii.decode(data);

                connection.input!.listen((data) async {
                  // ip
                  String receivedData  = ascii.decode(data);
                  
                  connection.input!.listen((data) async {
                    String receivedData = ascii.decode(data);
                      // Сохраняем данные в SharedPreferences MAC-addr : ipLocalhost
                      SharedPreferences prefs = await SharedPreferences.getInstance();
                      await prefs.setString(device.address, receivedData);
                      setState(() {
                        _isConnected = true;
                      });
                      print('Data saved to SharedPreferences');
                      connection.finish(); // Закрываем соединение
                   }).onDone(() { });
                 }).onDone(() { });
               }).onDone(() { });
             }).onDone(() {});
           }).onDone(() {});
        }).onDone(() {});
      }).onDone(() {});
    } catch (e) {
      print('Error: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.device.name ?? 'Device Settings'),
      ),
      body: Padding(
        padding: EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            Text(
              'Device Name: ${widget.device.name} ',
              style: TextStyle(fontSize: 18.0),
            ),
            SizedBox(height: 10),
            Text(
              'Device Address: ${widget.device.address}',
              style: TextStyle(fontSize: 18.0),
            ),
            SizedBox(height: 10),
            // при нажатии нужно сделать подключение 
            TextButton(onPressed: () async {
              await _showInputDialog(context, (enteredData) async {
                  // Если пользователь ввел данные, выполняем подключение
                  if (enteredData.isNotEmpty) {
                     _connectToDevice(widget.device, enteredData);
                  }
                });
            }, child: Text(_isConnected ? 'подключение произведено' : 'connect')),
            // Add more settings or actions here
            if(_isConnected)
              Column(
                children: [
                  Text("Мощность аппарата"),
                  SizedBox(height: 10),
                  Slider(
                    value: _powerValue,
                    min:0,
                    max:100,
                    divisions: 5,
                    onChanged:(double value) {
                          setState(() {
                            _powerValue = value;
                          }); 
                          _powerQuery(value.round(),widget.device.address);
                    },
                    ),
                  SizedBox(height: 10),
                  Switch(
                    value: _switchValue,
                    onChanged: (bool value){
                      setState(() {
                        _switchValue = value;
                      });
                      _ledQuery(value, widget.device.address);
                    })



                ],
              )

          ],
        ),
      ),
    );
  }
}
