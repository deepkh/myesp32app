// DO NOT EDIT THE FILE MANUALLY. 
// THE FILE IS GENERATED AUTOMATICLY BY EspConfigGenerator.py
#include "EspConfig.h"
#include "EspConfigPrivate.h"

const MyEsp::EspConfig g_espconfig = {  
  //.board_config = 
  {
    /*.name = */ "mushroom_01_lamp1_fan2_irmo4",
    /*.family = */ "ESP32C3", 
  },
  //.wifi_config = 
  {
    /*.enable = */ 1,
    /*.name = */ "mushroom_01_lamp1_fan2_irmo4",
    /*.ssid = */ ESPCONFIG_PRIVATE_WIFI_SSID,
    /*.password = */ ESPCONFIG_PRIVATE_WIFI_PASSWORD,
    /*.ledpin = */ 8, 
  },
  //.web_server_config = 
  {
    /*.enable = */ 1,
    /*.listening_port = */ 80, 
  },
  //.mqtt_broker_config = 
  {
    /*.enable = */ 1,
    /*.name = */ "mushroom_01_lamp1_fan2_irmo4",
    /*.ipaddr = */ ESPCONFIG_PRIVATE_MQTT_BROKER_IPADDR,
    /*.account = */ ESPCONFIG_PRIVATE_MQTT_BROKER_ACCOUNT,
    /*.password = */ ESPCONFIG_PRIVATE_MQTT_BROKER_PASSWORD, 
  },
  //.ntp_client_config = 
  {
    /*.enable = */ 1,
    /*.ipaddr = */ "tw.pool.ntp.org", 
  },
  //.otp_updater_config = 
  {
    /*.enable = */ 1, 
  },
  //.dht_config = 
  {
    /*.describe = */ "Temp & Humidity",
    /*.enable = */ 0,
    /*.dht_type = */ 22,
    /*.pin = */ 14,
    /*.mqtt_temperature_set = */ "home/mushroom/dht/temperature/set",
    /*.mqtt_humidity_set = */ "home/mushroom/dht/humidity/set",
    /*.update_interval = */ 5000, 
  },
  //.mq135_config = 
  {
    /*.describe = */ "Air Quality",
    /*.enable = */ 0,
    /*.pin = */ 17,
    /*.mqtt_set = */ "home/mushroom/mq135/set",
    /*.update_interval = */ 5000, 
  },
  //.hcsr501_config = 
  {
    /*.describe = */ "Ir Motion Detector",
    /*.enable = */ 1,
    /*.pin = */ 4,
    /*.mqtt_set = */ "home/mushroom/hcsr501/set",
    /*.update_interval = */ 200, 
  },
  //.switch1_config = 
  {
    /*.enable = */ 0,
    /*.mqtt_set = */ "home/mushroom/switch_pc/set",
    /*.mqtt_status = */ "home/mushroom/switch_pc/status",
    /*.pin = */ 0,
    /*.default_value = */ 0, 
  },
  //.switch2_config = 
  {
    /*.enable = */ 1,
    /*.mqtt_set = */ "home/mushroom/switch_lamp/set",
    /*.mqtt_status = */ "home/mushroom/switch_lamp/status",
    /*.pin = */ 1,
    /*.default_value = */ 0, 
  },
  //.switch3_config = 
  {
    /*.enable = */ 1,
    /*.mqtt_set = */ "home/mushroom/switch_fan/set",
    /*.mqtt_status = */ "home/mushroom/switch_fan/status",
    /*.pin = */ 2,
    /*.default_value = */ 0, 
  },
  //.switch4_config = 
  {
    /*.enable = */ 0,
    /*.mqtt_set = */ "home/mushroom/lamp4/set",
    /*.mqtt_status = */ "home/mushroom/lamp4/status",
    /*.pin = */ 3,
    /*.default_value = */ 0, 
  },
};
