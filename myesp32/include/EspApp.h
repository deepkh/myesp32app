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
    }

    static void HandleWiFiEvent(WiFiEvent_t event /*, arduino_event_info_t info*/)
    {
      Serial.printf("WiFi event %d ip %s\n", event, WiFi.localIP().toString().c_str());
#if 0

        Serial.printf("Disconnected. Reason: %d\n", info.wifi_sta_disconnected.reason);
#endif
      unsigned long now = millis();
      switch (event)
      {
      case WIFI_DISCONNECTED:
        WifiService::instance_->isConnected_ = 0;

        // if disconnection time is over 1200 seconds, than reboot. Sometimes may help ?
        if (WifiService::instance_->disconnectedDuration_)
        {
          if (now - WifiService::instance_->disconnectedDuration_ > 1200000)
          {
            Serial.println("WiFi continous disconnected... rebooting ");
            ESP_RESTART();
            return;
          }
        }

        Serial.println("WiFi disconnected, reconnect...");
        WifiService::instance_->ledInvert();
        if (WifiService::instance_->disconnectedDuration_ == 0)
        {
          WifiService::instance_->disconnectedDuration_ = now;
        }
        WiFi.reconnect();

        break;
      case WIFI_GOT_IP:
        Serial.printf("WiFi is connected! IP: %s\n", WiFi.localIP().toString().c_str());
        WifiService::instance_->disconnectedDuration_ = 0;
        WifiService::instance_->isConnected_ = 1;
        WifiService::instance_->ledOn();
        break;
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

      delay(5000);

      WiFi.hostname(cfg_.name);
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true); // save credentials in flash
      WiFi.onEvent(HandleWiFiEvent);

      WiFi.mode(WIFI_STA);
      WiFi.begin(cfg_.ssid, cfg_.password);

      delay(5000);

      unsigned long start = millis();
      while (WiFi.status() != WL_CONNECTED)
      {
        ledInvert();
        delay(500);
#if 0
      if (millis() - start > 60000)
      {
        Serial.printf("WiFi.status() :%d connection timeout!\n", (int)WiFi.status());
        return false;
      }
#endif
        Serial.printf("WiFi.status() :%d\n", (int)WiFi.status());
      }

      isConnected_ = true;
      return true;
    }

    bool begin() { return true; }
    bool loop(unsigned long now) { return true; }

  public:
    int ledPinStatus = 0;
    static WifiService *instance_;
    int disconnectedDuration_ = 0;
    int isConnected_ = 0;

  private:
    const WifiConfig &cfg_;
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
      client_.setServer(cfg_.ipaddr, 1883);
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
      // Loop until we're reconnected
      int failedCount = 0;
      while (!client_.connected())
      {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = cfg_.name;
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client_.connect(clientId.c_str(), cfg_.account, cfg_.password))
        {
          Serial.println("connected");
          EspHandleMqttConnected();
          failedCount = 0;
        }
        else
        {
          Serial.print("failed, rc=");
          Serial.print(client_.state());
          Serial.printf(" try again in 5 seconds. failedCount :%d\n", failedCount);
          // Wait 5 seconds before retrying
          delay(5000);

          if (failedCount >= 60)
          {
            Serial.printf(" failed count too much :%d reboot \n", failedCount);
            ESP_RESTART();
          }
          failedCount++;
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

  public:
    MqttConnectedCallback mqttConnectedCallback_ = nullptr;
    const char **mqttSwitchSets_ = nullptr;
    const char **mqttSwitchStates_ = nullptr;
    MqttSwitchHandler mqttSwitchHandler_ = nullptr;
    int mqttSwitchSetsLen_ = 0;
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

          dht_(cfg.dht_config),
          mq135_(cfg.mq135_config),
          hcsr501_(cfg.hcsr501_config)
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

    // Sensors
    DhtSensor dht_;
    Mq135Sensor mq135_;
    Hchr501Sensor hcsr501_;
  };
};

extern MyEsp::EspApp espApp;