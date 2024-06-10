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
  String _ip = 'NaN';
  
  String _url = 'NaN';
    Future<void> _showInputDialog(BuildContext context, Function(String,String) onSubmitted) async {
    TextEditingController _textFieldController = TextEditingController();
    TextEditingController _ssidController = TextEditingController();

    await showDialog<void>(
      context: context,
      barrierDismissible: false, // пользователь должен нажать на кнопку!
      builder: (BuildContext context) {
        return AlertDialog(
          title: Text('Enter SSID and wifi password'),
          content:Column(children: [
          TextField(
            controller: _ssidController,
            decoration: InputDecoration(hintText: "Enter SSID here"),
          ),
          SizedBox(height: 10),

          TextField(
            controller: _textFieldController,
            decoration: InputDecoration(hintText: "Enter password here"),
          ),

          ],),
          


          actions: <Widget>[
            TextButton(
              child: Text('Save'),
              onPressed: () {
                String password = _textFieldController.text;
                String ssid = _ssidController.text;
                Navigator.of(context).pop(); // Закрываем диалог
                onSubmitted(password,ssid); // Передаем введенные данные
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
      setState(() {
        _url = url.toString();
        
        });
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
        setState(() {
        _url = url.toString();
        });
        final response = await http.get(url);
      }
      else {
        final url =Uri.https(ipAddr.toString(),"/led_off");
        setState(() {
        _url = url.toString();
        });
        final response = await http.get(url);
      }
    }
    catch(e){
      print(e);
    }


}

// функция представляет собой коннект с платой, передача и принятие ascii кодов, передача пароля , ssid , пароля от wifi
  void _connectToDevice(BluetoothDevice device, String password , String ssid) async {
    try {
      
      final connection = await BluetoothConnection.toAddress(device.address);
      print('Connected to the device');
        connection.input!.listen((data) async {
          await Future.delayed(Duration(milliseconds: 200));

          String receivedData = ascii.decode(data);
          
          if (receivedData.contains("Print Bluetooth")){
            String password = "1641";
            connection.output.add(utf8.encode("$password\r\n"));
            await connection.output.allSent;
            await Future.delayed(Duration(milliseconds: 200));

          }
          else if (receivedData.contains("true password")){
            await Future.delayed(Duration(milliseconds: 200));
          }
          else if (receivedData.contains("SSID")){
            connection.output.add(utf8.encode("$ssid\r\n"));
            await connection.output.allSent;
            await Future.delayed(Duration(milliseconds: 200));
          }
          else if (receivedData.contains("Print password")){
            
            String wifiPassword = password;
        
            connection.output.add(utf8.encode(wifiPassword + "\r\n"));
            await connection.output.allSent;
            await Future.delayed(Duration(milliseconds: 200));
          }
          else if (receivedData.startsWith("192.168")){
            // Сохраняем данные в SharedPreferences MAC-addr : ipLocalhost
            SharedPreferences prefs = await SharedPreferences.getInstance();
            await prefs.setString(device.address, receivedData);
            setState(() {
              _isConnected = true;
              _ip = receivedData;
            });
            connection.output.add(utf8.encode("mne prishol ip - $receivedData\r\n"));
            await connection.output.allSent;           
            connection.finish(); // Закрываем соединение
          }
          
        }).onDone(() { });
        
        
      
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
              await _showInputDialog(context, (password,ssid) async {
                  // Если пользователь ввел данные, выполняем подключение
                  if (password.isNotEmpty && ssid.isNotEmpty) {
                     _connectToDevice(widget.device, password,ssid);
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
                    }),  
                  SizedBox(height: 10,),
                  Text("последний запрос"),
                  SizedBox(height: 10,),
                  Text("$_url"),
                  SizedBox(height: 10,),
                  Text("айпи адрес - $_ip"),


                ],
              )

          ],
        ),
      ),
    );
  }
}
