// DO NOT EDIT THE FILE MANUALLY. 
// THE FILE IS GENERATED AUTOMATICLY BY EspConfigGenerator.py
#pragma once

namespace MyEsp { 
struct BoardConfig {
  char name[64];
  char family[64];
};
struct WifiConfig {
  int enable;
  char name[64];
  char ssid[64];
  char password[64];
  int ledpin;
  int reboot_secs;
};
struct WebServerConfig {
  int enable;
  int listening_port;
};
struct MqttBrokerConfig {
  int enable;
  char name[64];
  char ipaddr[64];
  char account[64];
  char password[64];
};
struct NtpClientConfig {
  int enable;
  char ipaddr[64];
};
struct OtpUpdaterConfig {
  int enable;
};
struct DhtConfig {
  char describe[64];
  int enable;
  int dht_type;
  int pin;
  char mqtt_temperature_set[64];
  char mqtt_humidity_set[64];
  int update_interval;
};
struct Mq135Config {
  char describe[64];
  int enable;
  int pin;
  char mqtt_set[64];
  int update_interval;
};
struct Hcsr501Config {
  char describe[64];
  int enable;
  int pin;
  char mqtt_set[64];
  int update_interval;
};
struct Switch1Config {
  int enable;
  char mqtt_set[64];
  char mqtt_status[64];
  int pin;
  int default_value;
};
struct Switch2Config {
  int enable;
  char mqtt_set[64];
  char mqtt_status[64];
  int pin;
  int default_value;
};
struct Switch3Config {
  int enable;
  char mqtt_set[64];
  char mqtt_status[64];
  int pin;
  int default_value;
};
struct Switch4Config {
  int enable;
  char mqtt_set[64];
  char mqtt_status[64];
  int pin;
  int default_value;
};
struct EspConfig {
  BoardConfig board_config;
  WifiConfig wifi_config;
  WebServerConfig web_server_config;
  MqttBrokerConfig mqtt_broker_config;
  NtpClientConfig ntp_client_config;
  OtpUpdaterConfig otp_updater_config;
  DhtConfig dht_config;
  Mq135Config mq135_config;
  Hcsr501Config hcsr501_config;
  Switch1Config switch1_config;
  Switch2Config switch2_config;
  Switch3Config switch3_config;
  Switch4Config switch4_config;
};
}

extern const MyEsp::EspConfig g_espconfig;
