# My ESP32 APP

![GitHub License](https://img.shields.io/github/license/deepkh/myesp32app?color=green) [![Build firmware](https://github.com/deepkh/myesp32app/actions/workflows/release.yml/badge.svg)](https://github.com/deepkh/myesp32app/actions) ![GitHub release (latest by date)](https://img.shields.io/github/v/release/deepkh/myesp32app?color=blue) ![PlatformIO](https://img.shields.io/badge/PlatformIO-orange?logo=platformio&logoColor=white) ![ESP32](https://img.shields.io/badge/Hardware-ESP32--C3-blue?logo=espressif&logoColor=white)
![ESP8266](https://img.shields.io/badge/Hardware-ESP8266-lightgrey?logo=espressif&logoColor=white)


Some helper functions to initialize the related modules of ESP32-IDF for ESP32 and ESP8266 using VSCode and PlatformIO quickly.

## Purposes
1. Briing up multiple of `ESP32` and `ESP32 8266` devices with only do the fews line of code in `main.cpp` is requried.
1. The YAML description file (`EspConfigDefinition.yml`) are good to `enable` and `disable` services and sensors on various devices.
1. The `ESP32-IDF` services and sensors initial code are in `EspApp.h`
1. The `main.cpp` are mostly responsible for the bussiness logic.
1. The `EspConfig.h` and `EspConfig.cpp` can be generated automaticly by using EspConfigGenerator.py

## Services
There are some services initial code shows as below. 
1. WifiService wifi_;
2. NtpClientService ntp_;
3. WebServerService web_;
4. OtaUpdaterService ota_;
5. MqttService mqtt_;

## Sensors
There are some services initial code shows as below. 
1. DhtSensor dht_;
2. Mq135Sensor mq135_;
3. Hchr501Sensor hcsr501_;

## My build environment 	
1. VSCode 1.108
2. PlatformIO Core 6.1.19·Home 3.4.4
3. Debian 13 Trixie

