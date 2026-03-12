// DO NOT EDIT THE FILE MANUALLY. 
// THE FILE IS GENERATED AUTOMATICLY BY EspConfigGenerator.py
#include "EspConfig.h"
#include "EspConfigPrivate.h"

const MyEsp::EspConfig g_espconfig = {  
  //.board_config = 
  {
    /*.name = */ "balcony_01_nodemcu",
    /*.family = */ "ESP8266", 
  },
  //.wifi_config = 
  {
    /*.enable = */ 1,
    /*.name = */ "balcony_01_nodemcu",
    /*.ssid = */ ESPCONFIG_PRIVATE_WIFI_SSID,
    /*.password = */ ESPCONFIG_PRIVATE_WIFI_PASSWORD,
    /*.ledpin = */ 2, 
  },
  //.web_server_config = 
  {
    /*.enable = */ 1,
    /*.listening_port = */ 80, 
  },
  //.mqtt_broker_config = 
  {
    /*.enable = */ 1,
    /*.name = */ "balcony_01_nodemcu",
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
    /*.enable = */ 1,
    /*.dht_type = */ 22,
    /*.pin = */ 14,
    /*.mqtt_temperature_set = */ "home/balcony/dht/temperature/set",
    /*.mqtt_humidity_set = */ "home/balcony/dht/humidity/set",
    /*.update_interval = */ 5000, 
  },
  //.mq135_config = 
  {
    /*.describe = */ "Air Quality",
    /*.enable = */ 1,
    /*.pin = */ 17,
    /*.mqtt_set = */ "home/balcony/mq135/set",
    /*.update_interval = */ 5000, 
  },
  //.hcsr501_config = 
  {
    /*.describe = */ "Ir Motion Detector",
    /*.enable = */ 1,
    /*.pin = */ 12,
    /*.mqtt_set = */ "home/balcony/hcsr501/set",
    /*.update_interval = */ 200, 
  },
  //.switch1_config = 
  {
    /*.enable = */ 1,
    /*.mqtt_set = */ "home/balcony/lamp1/set",
    /*.mqtt_status = */ "home/balcony/lamp1/status",
    /*.pin = */ 13,
    /*.default_value = */ 0, 
  },
  //.switch2_config = 
  {
    /*.enable = */ 1,
    /*.mqtt_set = */ "home/balcony/lamp2/set",
    /*.mqtt_status = */ "home/balcony/lamp2/status",
    /*.pin = */ 15,
    /*.default_value = */ 0, 
  },
  //.switch3_config = 
  {
    /*.enable = */ 0,
    /*.mqtt_set = */ "home/balcony/lamp3/set",
    /*.mqtt_status = */ "home/balcony/lamp3/status",
    /*.pin = */ 0,
    /*.default_value = */ 0, 
  },
  //.switch4_config = 
  {
    /*.enable = */ 0,
    /*.mqtt_set = */ "home/balcony/lamp4/set",
    /*.mqtt_status = */ "home/balcony/lamp4/status",
    /*.pin = */ 0,
    /*.default_value = */ 0, 
  },
};
