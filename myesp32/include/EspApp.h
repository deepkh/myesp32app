/*
 * MIT License
 *
 * Copyright (c) 2026 Gary Huang (deepkh@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction...
 */
#pragma once
#include "EspConfig.h"
#include "EspConfigPrivate.h"
#include <Preferences.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#define WIFI_DISCONNECTED WIFI_EVENT_STAMODE_DISCONNECTED
#define WIFI_GOT_IP WIFI_EVENT_STAMODE_GOT_IP
#define ESP_RESTART ESP.restart
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#define WIFI_DISCONNECTED ARDUINO_EVENT_WIFI_STA_DISCONNECTED
#define WIFI_GOT_IP ARDUINO_EVENT_WIFI_STA_GOT_IP
#define WIFI_LOST_IP ARDUINO_EVENT_WIFI_STA_LOST_IP
#define ESP_RESTART esp_restart
#endif

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <ElegantOTA.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

namespace MyEsp
{

  class WifiService
  {
  public:
    explicit WifiService(const WifiConfig &cfg)
        : cfg_(cfg)
    {
      instance_ = this;
#if defined(ESP32)
      wifiQueue_ = xQueueCreate(30, sizeof(WiFiEvent_t));
#endif
    }

    /**
     * Stores the SSID and Password into a namespace called "wifi-config"
     */
    void StorePermanetlyConfig(const String &ssid, const String &password)
    {
      //
      // Only store ssid, password permanetly when the FW is NOT built by server
      //
      if (strcmp(ESPCONFIG_PRIVATE_WIFI_SSID, "SSID") != 0 &&
          strcmp(ESPCONFIG_PRIVATE_WIFI_PASSWORD, "PASSWORD") != 0)
      {
        preferences_.begin("wifi-config", false);

        preferences_.putString("ssid", ssid.c_str());
        preferences_.putString("password", password.c_str());

        // Close the preferences
        preferences_.end();
        Serial.printf("*** Wifi Config Saved Permanently. '%s' '%s' \n", ssid.c_str(), password.c_str());
      }
      else
      {
        Serial.printf("*** Wifi Config WITHOUT Saved Permanently. '%s' \n", ssid.c_str(), password.c_str());
      }
    }

    /**
     * Loads the SSID and Password from NVS memory
     */
    void LoadPermanetlyConfig(String &ssid, String &password)
    {
      //
      // Only load from PermanetlyConfig when the FW is built by server
      //
      if (strcmp(ESPCONFIG_PRIVATE_WIFI_SSID, "SSID") == 0 &&
          strcmp(ESPCONFIG_PRIVATE_WIFI_PASSWORD, "PASSWORD") == 0)
      {
        // Open the "wifi-config" namespace in read-only mode (true)
        preferences_.begin("wifi-config", true);

        // Get the values, providing a default empty string if they don't exist
        ssid = preferences_.getString("ssid", cfg_.ssid);
        password = preferences_.getString("password", cfg_.password);

        preferences_.end();

        Serial.printf("*** Wifi Config Load Permanently. '%s' '%s' \n", ssid.c_str(), password.c_str());
      }
      else
      {
        ssid = cfg_.ssid;
        password = cfg_.password;

        Serial.printf("*** Wifi Config Load WITHOUT Permanently. '%s' '%s' \n", ssid.c_str(), password.c_str());
      }
    }

    void HandleWiFiEvent(WiFiEvent_t event /*, arduino_event_info_t info*/)
    {
      Serial.printf("WiFi event %d ip %s\n", event, WiFi.localIP().toString().c_str());

      unsigned long now = millis();
      switch (event)
      {
      case WIFI_DISCONNECTED:
        if (disconnectedStart_ == 0)
        {
          disconnectedStart_ = now;
        }
        isConnected_ = 0;
        delay(500);

        // if disconnection time is over 1200 seconds, than reboot. Sometimes may help ?
        if (cfg_.reboot_secs && disconnectedDuration_)
        {
          if (now - disconnectedDuration_ >= cfg_.reboot_secs * 1000)
          {
            Serial.println("WiFi continous disconnected... rebooting ");
            ESP_RESTART();
            return;
          }
        }

        Serial.printf("WiFi disconnected, reconnect ... Elapsed Secs: %d\n", (now - disconnectedStart_) / 1000);
        ledInvert();
        if (disconnectedDuration_ == 0)
        {
          disconnectedDuration_ = now;
        }

        break;
      case WIFI_GOT_IP:
        Serial.printf("WiFi is connected! IP: %s. Elapsed Secs: %d\n", WiFi.localIP().toString().c_str(), (now - disconnectedStart_) / 1000);
        disconnectedDuration_ = 0;
        isConnected_ = 1;
        disconnectedStart_ = 0;
        StorePermanetlyConfig(ssid_, passowrd_);
        ledOn();
        break;
#if defined(ESP32)
      case WIFI_LOST_IP:
        Serial.println("Lost IP → force reconnect");
        isConnected_ = 0;
        WiFi.reconnect();
        break;
#endif
      }
    }

    void ledOff()
    {
      digitalWrite(cfg_.ledpin, HIGH);
      ledPinStatus = 0;
    }

    void ledOn()
    {
      digitalWrite(cfg_.ledpin, LOW);
      ledPinStatus = 1;
    }

    void ledInvert()
    {
      if (ledPinStatus)
      {
        ledOff();
      }
      else
      {
        ledOn();
      }
    }

    bool setup()
    {
      pinMode(cfg_.ledpin, OUTPUT);
      ledOff();

      WiFi.hostname(cfg_.name);
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true); // save credentials in flash
#if defined(ESP8266)
      WiFi.onEvent([](WiFiEvent_t cbEvent)
                   { WifiService::instance_->HandleWiFiEvent(cbEvent); });
#elif defined(ESP32)
      WiFi.onEvent([](WiFiEvent_t cbEvent)
                   { xQueueSend(WifiService::instance_->wifiQueue_, &cbEvent, 0); });

#endif
      WiFi.mode(WIFI_STA);
      LoadPermanetlyConfig(ssid_, passowrd_);
      WiFi.begin(ssid_.c_str(), passowrd_.c_str());

      disconnectedStart_ = millis();
      delay(5000);
      return true;
    }

    bool begin() { return true; }
    bool loop(unsigned long now)
    {
#if defined(ESP32)
      WiFiEvent_t event;
      while (xQueueReceive(wifiQueue_, &event, 0) == pdTRUE)
      {
        HandleWiFiEvent(event);
      }
#endif
      return true;
    }

  public:
    int ledPinStatus = 0;
    static WifiService *instance_;
    int disconnectedDuration_ = 0;
    int isConnected_ = 0;
    unsigned long disconnectedStart_ = 0;
#if defined(ESP32)
    QueueHandle_t wifiQueue_ = nullptr;
#endif
  private:
    String ssid_;
    String passowrd_;
    const WifiConfig &cfg_;
    Preferences preferences_;
  };

  class NtpClientService
  {
  public:
    explicit NtpClientService(const NtpClientConfig &cfg)
        : cfg_(cfg), ntpUDP_(), client_(ntpUDP_)
    {
      instance_ = this;
    }

    bool setup()
    {
      client_.setPoolServerName(cfg_.ipaddr);
      client_.setTimeOffset(3600 * 8);
      client_.setUpdateInterval(21600000);
      Serial.printf("NTPClient is setted!\n");
      return true;
    }

    bool begin()
    {
      client_.begin();
      if (!client_.update())
      {
        Serial.printf("Failed to client_.update() \n");
        return true;
      }
      Serial.printf("NTP Update succeeded! %s \n", client_.getFormattedTime().c_str());
      return true;
    }

    bool loop(unsigned long now)
    {
      client_.update();
      return true;
    }

  public:
    static NtpClientService *instance_;

  private:
    const NtpClientConfig &cfg_;
    WiFiUDP ntpUDP_;
    NTPClient client_;
  };

  class WebServerService
  {
  public:
    explicit WebServerService(const WebServerConfig cfg)
        : cfg_(cfg),
#if defined(ESP8266)
          webServer_(cfg.listening_port)
#elif defined(ESP32)
          webServer_(cfg.listening_port)
#endif
    {
      instance_ = this;
    }

    bool setup()
    {
      webServer_.on("/", [this]()
                    { webServer_.send(200, "text/plain", "ESP WebServer Running"); });
      Serial.printf("WebServer is setted!\n");
      return true;
    }

    bool begin()
    {
      webServer_.begin();
      return true;
    }

    bool loop(unsigned long now)
    {
      webServer_.handleClient();
      return true;
    }

#if defined(ESP8266)
    ESP8266WebServer &GetWebServer() { return webServer_; }
#elif defined(ESP32)
    WebServer &GetWebServer() { return webServer_; }
#endif

  public:
    static WebServerService *instance_;

  private:
    const WebServerConfig &cfg_;
#if defined(ESP8266)
    ESP8266WebServer webServer_;
#elif defined(ESP32)
    WebServer webServer_;
#endif
    unsigned long lastUpdate = 0;
  };

  class OtaUpdaterService
  {
  public:
    explicit OtaUpdaterService(const OtpUpdaterConfig cfg)
        : cfg_(cfg)
    {
      instance_ = this;
    }

    bool setup()
    {
      Serial.printf("OtaUpdaterService is setted!\n");
      return true;
    }

    bool begin()
    {
      ElegantOTA.begin(&WebServerService::instance_->GetWebServer());
      return true;
    }

    bool loop(unsigned long now)
    {
      ElegantOTA.loop();
      return true;
    }

  public:
    static OtaUpdaterService *instance_;

  private:
    const OtpUpdaterConfig &cfg_;
  };

  class MqttService
  {
  public:
    typedef std::function<void(void)> MqttConnectedCallback;
    typedef std::function<void(unsigned int, const char *topic, const char *msg)> MqttSwitchHandler;

    explicit MqttService(const MqttBrokerConfig &cfg)
        : cfg_(cfg), client_(wifi_)
    {
      MqttService::instance_ = this;
    }

    /**
     * Stores the SSID and Password into a namespace called "wifi-config"
     */
    void StorePermanetlyConfig(const String &ipaddr, const String &account, const String &password)
    {
      //
      // Only store ssid, password permanetly when the FW is NOT built by server
      //
      if (strcmp(ESPCONFIG_PRIVATE_MQTT_BROKER_IPADDR, "0.0.0.0") != 0)
      {
        preferences_.begin("mqtt-config", false);
        preferences_.putString("mqtt_ipaddr", ipaddr.c_str());
        preferences_.putString("mqtt_account", account.c_str());
        preferences_.putString("mqtt_password", password.c_str());

        // Close the preferences
        preferences_.end();
        Serial.printf("*** Mqtt Config Saved Permanently. '%s' '%s' '%s'\n", ipaddr.c_str(), account.c_str(), password.c_str());
      }
      else
      {
        Serial.printf("*** Mqtt Config WITHOUT Saved Permanently.  '%s' '%s' '%s'\n", ipaddr.c_str(), account.c_str(), password.c_str());
      }
    }

    /**
     * Loads the SSID and Password from NVS memory
     */
    void LoadPermanetlyConfig(String &ipaddr, String &account, String &password)
    {
      //
      // Only load from PermanetlyConfig when the FW is built by server
      //
      if (strcmp(ESPCONFIG_PRIVATE_MQTT_BROKER_IPADDR, "0.0.0.0") == 0)
      {
        // Open the "wifi-config" namespace in read-only mode (true)
        preferences_.begin("mqtt-config", true);

        // Get the values, providing a default empty string if they don't exist
        ipaddr = preferences_.getString("mqtt_ipaddr", cfg_.ipaddr);
        account = preferences_.getString("mqtt_account", cfg_.account);
        password = preferences_.getString("mqtt_password", cfg_.password);

        Serial.printf("*** Mqtt Config Load from PermanetlyConfig: '%s' '%s' '%s'  \n", ipaddr.c_str(), account.c_str(), password.c_str());

        preferences_.end();
      }
      else
      {
        ipaddr = cfg_.ipaddr;
        account = cfg_.account;
        password = cfg_.password;

        Serial.printf("*** Mqtt Config Load WITHOUT from PermanetlyConfig: '%s' '%s' '%s'  \n", ipaddr.c_str(), account.c_str(), password.c_str());
      }
    }

    void RegisterMqttSwitchSets(const char **mqttSwitchSets, const char **mqttSwitchStates, const MqttSwitchHandler mqttSwitchHandler, int mqttSwitchSetsLen)
    {
      mqttSwitchSets_ = mqttSwitchSets;
      mqttSwitchStates_ = mqttSwitchStates;
      mqttSwitchHandler_ = mqttSwitchHandler;
      mqttSwitchSetsLen_ = mqttSwitchSetsLen;
    }

    void RegisterMqttConnectedCallback(const MqttConnectedCallback cb)
    {
      mqttConnectedCallback_ = cb;
    }

    bool setup()
    {
      LoadPermanetlyConfig(ipaddr_, account_, password_);
      client_.setServer(ipaddr_.c_str(), 1883);
      return true;
    }

    bool begin()
    {
      client_.setCallback([](char *topic, uint8_t *payload, unsigned int length)
                          {
      String message;
      for (int i = 0; i < length; i++)
      {
          message += (char)payload[i];
      }

      // check switch
      if (message == "ON" || message == "OFF")
      {
          bool isOn = message == "ON";
          for (int i = 0; i < MqttService::instance_->mqttSwitchSetsLen_; i++)
          {
              if (strcmp(topic, MqttService::instance_->mqttSwitchSets_[i]) == 0)
              {
                  //Serial.printf("Mqtt received: '%s' -> '%s'\n", MqttService::instance_->mqttSwitchSets_[i], message.c_str());
                  MqttService::instance_->GetMqttClient().publish(
                    MqttService::instance_->mqttSwitchStates_[i], message.c_str(), true); // Send confirmation back to HA
                  MqttService::instance_->mqttSwitchHandler_(i, MqttService::instance_->mqttSwitchSets_[i], message.c_str());
                  break;
              }
          }
      } });

      return true;
    }

    void EspHandleMqttConnected()
    {
      for (int i = 0; i < mqttSwitchSetsLen_; i++)
      {
        Serial.printf("Mqtt do subscribe: %s\n", mqttSwitchSets_[i]);
        client_.subscribe(mqttSwitchSets_[i]);
      }

      if (mqttConnectedCallback_)
      {
        mqttConnectedCallback_();
      }
    }

    void doConnect()
    {
      if (WifiService::instance_->isConnected_ == 0)
      {
        return;
      }

      if (!client_.connected())
      {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = cfg_.name;
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client_.connect(clientId.c_str(), account_.c_str(), password_.c_str()))
        {
          Serial.println("connected");
          EspHandleMqttConnected();
          StorePermanetlyConfig(ipaddr_, account_, password_);
          failedCount_ = 0;
        }
        else
        {
          Serial.print("failed, rc=");
          Serial.print(client_.state());
          Serial.printf(" try again in 5 seconds. failedCount :%d\n", failedCount_);
          // Wait 5 seconds before retrying
          delay(500);
          failedCount_++;
        }
      }
    }

    bool loop(unsigned long now)
    {
      doConnect();
      client_.loop();
      return true;
    }

    PubSubClient &GetMqttClient()
    {
      return client_;
    }

  public:
    static MqttService *instance_;

  private:
    const MqttBrokerConfig &cfg_;
    WiFiClient wifi_;
    PubSubClient client_;
    String ipaddr_;
    String account_;
    String password_;

  public:
    MqttConnectedCallback mqttConnectedCallback_ = nullptr;
    const char **mqttSwitchSets_ = nullptr;
    const char **mqttSwitchStates_ = nullptr;
    MqttSwitchHandler mqttSwitchHandler_ = nullptr;
    int mqttSwitchSetsLen_ = 0;
    Preferences preferences_;
    int failedCount_ = 0;
  };

  class DhtSensor
  {
  public:
    explicit DhtSensor(const DhtConfig &cfg)
        : cfg_(cfg), dht_(cfg.pin, cfg.dht_type)
    {
    }

    bool setup() { return true; }

    bool begin()
    {
      // Initialize device.
      dht_.begin();
      Serial.println(F("DHTxx Unified Sensor Example"));
      // Print temperature sensor details.
      sensor_t sensor;
      dht_.temperature().getSensor(&sensor);
      Serial.println(F("------------------------------------"));
      Serial.println(F("Temperature Sensor"));
      Serial.print(F("Sensor Type: "));
      Serial.println(sensor.name);
      Serial.print(F("Driver Ver:  "));
      Serial.println(sensor.version);
      Serial.print(F("Unique ID:   "));
      Serial.println(sensor.sensor_id);
      Serial.print(F("Max Value:   "));
      Serial.print(sensor.max_value);
      Serial.println(F("°C"));
      Serial.print(F("Min Value:   "));
      Serial.print(sensor.min_value);
      Serial.println(F("°C"));
      Serial.print(F("Resolution:  "));
      Serial.print(sensor.resolution);
      Serial.println(F("°C"));
      Serial.println(F("------------------------------------"));
      // Print humidity sensor details.
      dht_.humidity().getSensor(&sensor);
      Serial.println(F("Humidity Sensor"));
      Serial.print(F("Sensor Type: "));
      Serial.println(sensor.name);
      Serial.print(F("Driver Ver:  "));
      Serial.println(sensor.version);
      Serial.print(F("Unique ID:   "));
      Serial.println(sensor.sensor_id);
      Serial.print(F("Max Value:   "));
      Serial.print(sensor.max_value);
      Serial.println(F("%"));
      Serial.print(F("Min Value:   "));
      Serial.print(sensor.min_value);
      Serial.println(F("%"));
      Serial.print(F("Resolution:  "));
      Serial.print(sensor.resolution);
      Serial.println(F("%"));
      Serial.println(F("------------------------------------"));
      // Set delay between sensor readings based on sensor details.
      // delayMS = sensor.min_delay / 1000;
      return true;
    }

    bool loop(unsigned long now)
    {
      sensors_event_t event;
      float temperature = -1;
      float humidity = -1;

      if (now - lastUpdate_ <= cfg_.update_interval)
      {
        return false;
      }
      lastUpdate_ = now;
      dht_.temperature().getEvent(&event);
      if (isnan(event.temperature))
      {
        Serial.println(F("Error reading temperature!"));
      }
      else
      {
        // Serial.print(F("Temperature: "));
        temperature = event.temperature;
        // Serial.print(event.temperature);
        // Serial.println(F("°C"));
      }

      // Get humidity event and print its value.
      dht_.humidity().getEvent(&event);
      if (isnan(event.relative_humidity))
      {
        Serial.println(F("Error reading humidity!"));
      }
      else
      {
        // Serial.print(F("Humidity: "));
        humidity = event.relative_humidity;
        // Serial.print(event.relative_humidity);
        // Serial.println(F("%"));
      }

      dhtCallBack_(temperature, humidity);
      return true;
    }

    typedef std::function<void(float, float)> DhtCallback;
    void RegisterDhtCallback(const DhtCallback cb)
    {
      dhtCallBack_ = cb;
    }

  private:
    const DhtConfig &cfg_;
    DHT_Unified dht_;
    DhtCallback dhtCallBack_;
    unsigned int lastUpdate_ = 0;
  };

  class Hchr501Sensor
  {
  public:
    explicit Hchr501Sensor(const Hcsr501Config &cfg)
        : cfg_(cfg)
    {
    }

    bool setup()
    {
      pinMode(cfg_.pin, INPUT);
      Serial.printf("HCHR501 is setted!\n");
      return true;
    }

    bool begin()
    {
      return true;
    }

    bool loop(unsigned long now)
    {
      if (now - lastUpdate <= cfg_.update_interval)
      {
        return false;
      }
      lastUpdate = now;

      int currentState = digitalRead(cfg_.pin);
      hchr501CallBack_(currentState);

      return true;
    }

    typedef std::function<void(int)> Hchr501Callback;
    void RegisterHchr501Callback(const Hchr501Callback cb)
    {
      hchr501CallBack_ = cb;
    }

  private:
    const Hcsr501Config &cfg_;
    Hchr501Callback hchr501CallBack_;
    unsigned long lastUpdate = 0;
  };

  class Mq135Sensor
  {
  public:
    explicit Mq135Sensor(const Mq135Config &cfg)
        : cfg_(cfg)
    {
    }
    bool setup()
    {
      return true;
    }

    bool begin()
    {
      return true;
    }

    bool loop(unsigned long now)
    {
      if (now - lastUpdate <= cfg_.update_interval)
      {
        return false;
      }
      lastUpdate = now;

      // Read the analog value (0 to 1023)
      int sensorValue = analogRead(cfg_.pin);
      // Serial.printf("mq135 %d\n", sensorValue);

      // Convert to a rough "Quality" percentage
      // 1023 is max saturation, 0 is clean air (ideally)
      float air_quality = (sensorValue / 1024.0) * 100.0;
      mq135CallBack_(air_quality);
      return true;
    }

    typedef std::function<void(float)> Mq135Callback;
    void RegisterMq135Callback(const Mq135Callback cb)
    {
      mq135CallBack_ = cb;
    }

  private:
    const Mq135Config &cfg_;
    Mq135Callback mq135CallBack_;
    unsigned long lastUpdate = 0;
  };

  class Switch1
  {
  public:
    explicit Switch1(const Switch1Config *cfg)
        : cfg_(cfg)
    {
    }

    bool setup()
    {
      pinMode(cfg_->pin, OUTPUT);
      delay(10);
      digitalWrite(cfg_->pin, cfg_->default_value);
      return true;
    }

  private:
    const Switch1Config *cfg_;
  };

  class Switch2 : public Switch1
  {
  public:
    explicit Switch2(const Switch2Config *cfg)
        : Switch1((const Switch1Config *)cfg) {}
  };

  class Switch3 : public Switch1
  {
  public:
    explicit Switch3(const Switch3Config *cfg)
        : Switch1((const Switch1Config *)cfg) {}
  };

  class Switch4 : public Switch1
  {
  public:
    explicit Switch4(const Switch4Config *cfg)
        : Switch1((const Switch1Config *)cfg) {}
  };

  class SimpleWatchDog
  {
  public:

    typedef std::function<void(void)> WDTTimeoutCallback;

    explicit SimpleWatchDog()
    {
    }
    
      void RegisterWDTRebootCallback(const WDTTimeoutCallback wdtTimeoutCallback)
    {
      wdtTimeoutCallback_ = wdtTimeoutCallback;
    }

    bool setup()
    {
      return true;
    }

    bool begin()
    {
      return true;
    }

    bool loop(unsigned long now)
    {
      if (!MyEsp::WifiService::instance_->isConnected_)
      {
        if (lastDisconnectedTime_ == 0)
        {
          Serial.printf("WDT: WiFI is disconnected! %p %p %u %u \n", this, &lastDisconnectedTime_, lastDisconnectedTime_, now);
          lastDisconnectedTime_ = now;
        }

        if ((now - lastDisconnectedTime_) > 10000)
        {
          Serial.printf("WDT:  WiFI recovery timeout! Reboot! \n");
          if (wdtTimeoutCallback_) {
            wdtTimeoutCallback_();
          }
          ESP_RESTART();
        }
        return true;
      }

      lastDisconnectedTime_ = 0;
      return true;
    }

  private:
    WDTTimeoutCallback wdtTimeoutCallback_ = nullptr;
    unsigned long lastDisconnectedTime_ = 0;
  };

  class EspApp
  {
  public:
    explicit EspApp(const EspConfig &cfg)
        : cfg_(cfg),
          wifi_(cfg.wifi_config),
          ntp_(cfg.ntp_client_config),
          web_(cfg.web_server_config),
          ota_(cfg.otp_updater_config),
          mqtt_(cfg.mqtt_broker_config),
          wdt_(),

          dht_(cfg.dht_config),
          mq135_(cfg.mq135_config),
          hcsr501_(cfg.hcsr501_config),

          switch1_(&cfg.switch1_config),
          switch2_(&cfg.switch2_config),
          switch3_(&cfg.switch3_config),
          switch4_(&cfg.switch4_config)
    {
    }

    bool setup()
    {
      if (cfg_.wifi_config.enable && !wifi_.setup())
        return false;
      if (cfg_.ntp_client_config.enable && !ntp_.setup())
        return false;
      if (cfg_.web_server_config.enable && !web_.setup())
        return false;
      if (cfg_.otp_updater_config.enable && !ota_.setup())
        return false;
      if (cfg_.mqtt_broker_config.enable && !mqtt_.setup())
        return false;

      if (cfg_.dht_config.enable && !dht_.setup())
        return false;
      if (cfg_.mq135_config.enable && !mq135_.setup())
        return false;
      if (cfg_.hcsr501_config.enable && !hcsr501_.setup())
        return false;

      if (cfg_.switch1_config.enable && !switch1_.setup())
        return false;
      if (cfg_.switch2_config.enable && !switch2_.setup())
        return false;
      if (cfg_.switch3_config.enable && !switch3_.setup())
        return false;
      if (cfg_.switch4_config.enable && !switch4_.setup())
        return false;

      return true;
    }

    bool begin()
    {
      if (cfg_.wifi_config.enable && !wifi_.begin())
        return false;
      if (cfg_.ntp_client_config.enable && !ntp_.begin())
        return false;
      if (cfg_.web_server_config.enable && !web_.begin())
        return false;
      if (cfg_.otp_updater_config.enable && !ota_.begin())
        return false;
      if (cfg_.mqtt_broker_config.enable && !mqtt_.begin())
        return false;

      if (cfg_.dht_config.enable && !dht_.begin())
        return false;
      if (cfg_.mq135_config.enable && !mq135_.begin())
        return false;
      if (cfg_.hcsr501_config.enable && !hcsr501_.begin())
        return false;

      return true;
    }
    bool loop(unsigned long now)
    {
      wdt_.loop(now);
      if (cfg_.wifi_config.enable)
      {
        wifi_.loop(now);
      }
      if (cfg_.ntp_client_config.enable)
      {
        ntp_.loop(now);
      }
      if (cfg_.web_server_config.enable)
      {
        web_.loop(now);
      }
      if (cfg_.otp_updater_config.enable)
      {
        ota_.loop(now);
      }
      if (cfg_.mqtt_broker_config.enable)
      {
        mqtt_.loop(now);
      }
      if (cfg_.dht_config.enable)
      {
        dht_.loop(now);
      }
      if (cfg_.mq135_config.enable)
      {
        mq135_.loop(now);
      }
      if (cfg_.hcsr501_config.enable)
      {
        hcsr501_.loop(now);
      }

      return true;
    }

  private:
    const EspConfig &cfg_;

  public:
    // Services
    WifiService wifi_;
    NtpClientService ntp_;
    WebServerService web_;
    OtaUpdaterService ota_;
    MqttService mqtt_;
    SimpleWatchDog wdt_;

    // Sensors
    DhtSensor dht_;
    Mq135Sensor mq135_;
    Hchr501Sensor hcsr501_;

    // Switchs
    Switch1 switch1_;
    Switch2 switch2_;
    Switch3 switch3_;
    Switch4 switch4_;
  };
};

extern MyEsp::EspApp espApp;