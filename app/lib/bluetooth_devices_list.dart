import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'device_settings_screen.dart';

class BluetoothDeviceList extends StatefulWidget {
  // final String filterKeyword;

  // BluetoothDeviceList({required this.filterKeyword});
  BluetoothDeviceList();

  @override
  _BluetoothDeviceListState createState() => _BluetoothDeviceListState();
}

class _BluetoothDeviceListState extends State<BluetoothDeviceList> {
  List<BluetoothDevice> _devices = [];
  bool _isLoading = true;

  @override
  void initState() {
    super.initState();
    _getBondedDevices();
  }

  Future<void> _getBondedDevices() async {
    setState(() {
      _isLoading = true;
    });

    List<BluetoothDevice> devices = [];
    try {
      devices = await FlutterBluetoothSerial.instance.getBondedDevices();
    } catch (e) {
      print('Error: $e');
    }

    setState(() {
      _devices = devices;
      _isLoading = false;
    });
  }

  void _connectToDevice(BluetoothDevice device) async {
    try {
      await BluetoothConnection.toAddress(device.address).then((_connection) {
        print('Connected to the device');
        _connection.input!.listen(null).onDone(() {
          print('Disconnected by remote request');
        });
      });
    } catch (e) {
      print('Error: $e');
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Bonded Bluetooth Devices'),
        actions: [
          IconButton(
            icon: Icon(Icons.refresh),
            onPressed: _isLoading ? null : _getBondedDevices,
          ),
        ],
      ),
      body: _isLoading
          ? Center(child: CircularProgressIndicator())
          : _devices.isEmpty
              ? Center(child: Text('No bonded devices found'))
              : ListView.builder(
                  itemCount: _devices.length,
                  itemBuilder: (context, index) {
                    final device = _devices[index];

                    if (device.name != null) {
                      return ListTile(
                        title: Text(device.name ?? 'Unknown device'),
                        subtitle: Text(device.address.toString()),
                        onTap: () {
                          Navigator.push(
                            context,
                            MaterialPageRoute(
                              builder: (context) => DeviceSettingsScreen(device: device),
                            ),
                          );
                        },
                      );
                    } else {
                      return Container();
                    }
                  },
                ),
    );
  }
}
