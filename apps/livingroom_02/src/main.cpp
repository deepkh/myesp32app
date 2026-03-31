/*
 * MIT License
 *
 * Copyright (c) 2026 Gary Huang (deepkh@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction...
 */
#include "EspApp.h"
#include "EspConfig.h"

static const char *mqttSwtichSet[] = {
    g_espconfig.switch1_config.mqtt_set,
    g_espconfig.switch2_config.mqtt_set,
};
static const char *mqttSwtichStates[] = {
    g_espconfig.switch1_config.mqtt_status,
    g_espconfig.switch2_config.mqtt_status,
};
static const int mqttSwtichLen = sizeof(mqttSwtichSet) / sizeof(mqttSwtichSet[0]);

static int mqttSwitchCallbackCounter[] = {
    0,
    0,
};

static unsigned long now = 0;

static int pirCurrStatus = -1;
static int pirPrevStatus = -1;
static unsigned long lastPirTriggered = 0;

static int lampPrevStatus = -1;
static unsigned long lastLampTriggered = 0;

static int lamp2PrevStatus = -1;

Preferences preferences;
static bool hasRestoreFromPermanetlyConfig = false;

static void mqttSwitchHandler(unsigned int index, const char *topic, const char *msg)
{
  int status = strcmp(msg, "ON") == 0;
  Serial.printf("mqttSwitchHandler %d '%s' -> '%s'\n", index, topic, msg);

  // Sync up in the initial stage
  if (mqttSwitchCallbackCounter[index] == 0)
  {
    switch (index)
    {
    case 0:
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch1_config.mqtt_set, lampPrevStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch1_config.mqtt_set, lampPrevStatus ? "ON" : "OFF");
      espApp.switch1_.writeValue(lampPrevStatus);
      break;
    case 1:
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch2_config.mqtt_set, lamp2PrevStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch2_config.mqtt_set, lamp2PrevStatus ? "ON" : "OFF");
      espApp.switch2_.writeValue(lamp2PrevStatus);
      break;
    }
  }
  else
  {
    switch (index)
    {
    case 0:
      lampPrevStatus = status;
      espApp.switch1_.writeValue(lampPrevStatus);
      if (lampPrevStatus == 0)
      {
        lastLampTriggered = 0;
      }
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch1_config.mqtt_set, lampPrevStatus ? "ON" : "OFF");

      break;
    case 1:
      lamp2PrevStatus = status;
      espApp.switch2_.writeValue(lamp2PrevStatus);
      break;
    }
  }

  mqttSwitchCallbackCounter[index]++;
}

/**
 *    Temp & Humidity
 */
static void handleDhtCallback(float temperature, float humidity)
{
  espApp.mqtt_.GetMqttClient().publish(g_espconfig.dht_config.mqtt_temperature_set, String(temperature).c_str(), true);
  espApp.mqtt_.GetMqttClient().publish(g_espconfig.dht_config.mqtt_humidity_set, String(humidity).c_str(), true);
  Serial.printf("HI %f %f\n", temperature, humidity);
}

/**
 *    Air Quality
 */
static void handleMq135Callback(float value)
{
}

/**
 *    PR Motion Sensor
 */
static void handleHchr501Callback(int status)
{
  pirCurrStatus = status;
  // static int c = 0;
  // Serial.printf("pirCurrStatus %d  %d\n", pirCurrStatus, c++);
}

static void handlePirStatus(unsigned int now, int status)
{
  if (pirPrevStatus == 0 && status == 0)
  {
  }
  else if (pirPrevStatus == 0 && status == 1)
  {
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.hcsr501_config.mqtt_set, "ON");
    Serial.printf("PIR on %d '%s'\n", status, g_espconfig.hcsr501_config.mqtt_set);

    pirPrevStatus = status;
    lastPirTriggered = now;
  }
  else if (pirPrevStatus == 1 && status == 0)
  {
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.hcsr501_config.mqtt_set, "OFF");
    Serial.printf("PIR off %d '%s'\n", status, g_espconfig.hcsr501_config.mqtt_set);

    pirPrevStatus = status;
  }
  else if (pirPrevStatus == 1 && status == 1)
  {
    lastPirTriggered = now;
  }
}

static void handleLamp1Status(unsigned int now, int status)
{
  if (lampPrevStatus == 0 && status == 0)
  {
  }
  else if (lampPrevStatus == 0 && status == 1)
  {
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch1_config.mqtt_set, "ON");
    lampPrevStatus = status;
    lastLampTriggered = now;
  }
  else if (lampPrevStatus == 1 && status == 0 && lastLampTriggered > 0)
  {
    if ((now - lastLampTriggered) < 30000)
    {
      return;
    }
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch1_config.mqtt_set, "OFF");
    lampPrevStatus = status;
    lastLampTriggered = 0;
  }
  else if (lampPrevStatus == 1 && status == 1 && lastLampTriggered > 0)
  {
    lastLampTriggered = now;
  }
}

static void storePermanetlyConfig()
{
  if (hasRestoreFromPermanetlyConfig)
  {
    Serial.printf("=== main: %s stored: SKIPED (sould be clear before store): pirCurrStatus:%u pirPrevStatus:%u lampPrevStatus:%u lamp2PrevStatus:%u \n", __func__, pirCurrStatus, pirPrevStatus, lampPrevStatus, lamp2PrevStatus);
    return;
  }
  preferences.begin("main_cfg", false);
  preferences.putULong("stroed", 1);
  preferences.putULong("pirCurrStatus", pirCurrStatus);
  preferences.putULong("pirPrevStatus", pirPrevStatus);
  preferences.putULong("lampPrevStatus", lampPrevStatus);
  preferences.putULong("lamp2PrevStatus", lamp2PrevStatus);
  preferences.end();
  Serial.printf("=== main: %s stored: pirCurrStatus:%u pirPrevStatus:%u lampPrevStatus:%u lamp2PrevStatus:%u \n", __func__, pirCurrStatus, pirPrevStatus, lampPrevStatus, lamp2PrevStatus);
}

static void restorePermanetlyConfig()
{
  preferences.begin("main_cfg", true);
  uint32_t stroed = preferences.getULong("stroed", 0);
  if (!stroed)
  {
    Serial.printf("=== main: %s no need to restore PermanetlyConfig.\n", __func__);
  }
  else
  {
    hasRestoreFromPermanetlyConfig = true;
    pirCurrStatus = preferences.getULong("pirCurrStatus", pirCurrStatus);
    pirPrevStatus = preferences.getULong("pirPrevStatus", pirPrevStatus);
    lampPrevStatus = preferences.getULong("lampPrevStatus", lampPrevStatus);
    lamp2PrevStatus = preferences.getULong("lamp2PrevStatus", lamp2PrevStatus);
    Serial.printf("=== main: %s restored: pirCurrStatus:%u pirPrevStatus:%u lampPrevStatus:%u lamp2PrevStatus:%u \n", __func__, pirCurrStatus, pirPrevStatus, lampPrevStatus, lamp2PrevStatus);
  }

  preferences.end();
}

static void clearPermanetlyConfig()
{
  preferences.begin("main_cfg", false);
  preferences.putULong("stroed", 0);
  preferences.end();
  hasRestoreFromPermanetlyConfig = false;
  Serial.printf("=== main: %s \n", __func__);
}

static void handleWDTTimeout()
{
  storePermanetlyConfig();
}

void setup()
{
  Serial.printf("a\n");

  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  if (!espApp.setup())
  {
    Serial.printf("failed to espApp.setup()\n");
    return;
  }

  if (g_espconfig.switch1_config.default_value == 0)
  {
    lastPirTriggered = millis();
  }

  lastLampTriggered = 0;
  pirCurrStatus = 0;
  pirPrevStatus = pirCurrStatus;
  lampPrevStatus = g_espconfig.switch1_config.default_value;
  lamp2PrevStatus = g_espconfig.switch2_config.default_value;

  restorePermanetlyConfig();

  espApp.mqtt_.RegisterMqttSwitchSets(mqttSwtichSet, mqttSwtichStates, mqttSwitchHandler, mqttSwtichLen);
  // esp.RegisterMqttConnectedCallback(handleMqttConnected);
  espApp.dht_.RegisterDhtCallback(handleDhtCallback);
  // espApp.mq135_.RegisterMq135Callback(handleMq135Callback);
  espApp.hcsr501_.RegisterHchr501Callback(handleHchr501Callback);
  espApp.wdt_.RegisterWDTRebootCallback(handleWDTTimeout);

  if (!espApp.begin())
  {
    Serial.printf("failed to esp.Begin()\n");
    return;
  }
}

void loop()
{
  now = millis();
  espApp.loop(now);

  if (MyEsp::WifiService::instance_->isConnected_)
  {
    if (hasRestoreFromPermanetlyConfig)
    {
      clearPermanetlyConfig();
    }
    handlePirStatus(now, pirCurrStatus);
    handleLamp1Status(now, pirCurrStatus);
  }
}
