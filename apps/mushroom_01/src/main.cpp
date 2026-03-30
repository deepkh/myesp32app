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
    g_espconfig.switch2_config.mqtt_set,
    g_espconfig.switch3_config.mqtt_set,

};
static const char *mqttSwtichStates[] = {
    g_espconfig.switch2_config.mqtt_status,
    g_espconfig.switch3_config.mqtt_status,
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

static int fanPrevStatus = -1;
static unsigned long lastFanTriggered = 0;

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
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch2_config.mqtt_set, lampPrevStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch2_config.mqtt_set, lampPrevStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.hcsr501_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      digitalWrite(g_espconfig.switch2_config.pin, lampPrevStatus ? HIGH : LOW);
      break;
    case 1:
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch3_config.mqtt_set, fanPrevStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch3_config.mqtt_set, fanPrevStatus ? "ON" : "OFF");
      digitalWrite(g_espconfig.switch3_config.pin, fanPrevStatus ? HIGH : LOW);
      break;
    }
  }
  else
  {
    switch (index)
    {
    case 0:
      digitalWrite(g_espconfig.switch2_config.pin, status ? HIGH : LOW);
      lampPrevStatus = status;
      if (status)
      {
        lastLampTriggered = now;
      }
      break;
    case 1:
      digitalWrite(g_espconfig.switch3_config.pin, status ? HIGH : LOW);
      fanPrevStatus = status;
      if (status)
      {
        lastFanTriggered = now;
      }
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
}

static void handlePirStatus(unsigned int now, int status)
{
  if (pirPrevStatus == 0 && status == 0)
  {
  }
  else if (pirPrevStatus == 0 && status == 1)
  {
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.hcsr501_config.mqtt_set, "ON");
    pirPrevStatus = status;
    lastPirTriggered = now;
  }
  else if (pirPrevStatus == 1 && status == 0)
  {
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.hcsr501_config.mqtt_set, "OFF");
    pirPrevStatus = status;
  }
  else if (pirPrevStatus == 1 && status == 1)
  {
    lastPirTriggered = now;
  }
}

static void handleLampStatus(unsigned int now, int status)
{
  if (lampPrevStatus == 0 && status == 0)
  {
  }
  else if (lampPrevStatus == 0 && status == 1)
  {
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch2_config.mqtt_set, "ON");
    lampPrevStatus = status;
    lastLampTriggered = now;
  }
  else if (lampPrevStatus == 1 && status == 0)
  {
    // Turn lamp off after 300 seconds
    if ((now - lastLampTriggered) < 300000)
    {
      return;
    }
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch2_config.mqtt_set, "OFF");
    lampPrevStatus = status;
  }
  else if (lampPrevStatus == 1 && status == 1)
  {
    lastLampTriggered = now;
  }
}

static void handleFanStatus(unsigned int now, int status)
{
  if (fanPrevStatus == 0 && status == 0)
  {
  }
  else if (fanPrevStatus == 0 && status == 1)
  {
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch3_config.mqtt_set, "ON");
    fanPrevStatus = status;
    lastFanTriggered = now;
  }
  else if (fanPrevStatus == 1 && status == 0)
  {
    // Turn fan off after 25200 seconds
    if ((now - lastFanTriggered) < 25200000)
    {
      return;
    }
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch3_config.mqtt_set, "OFF");
    fanPrevStatus = status;
  }
  else if (fanPrevStatus == 1 && status == 1)
  {
    lastFanTriggered = now;
  }
}

static void storePermanetlyConfig()
{
  if (hasRestoreFromPermanetlyConfig)
  {
    Serial.printf("=== main: %s stored: SKIPED (sould be clear before store): lampPrevStatus:%u fanPrevStatus:%u\n", __func__, lampPrevStatus, fanPrevStatus);
    return;
  }

  preferences.begin("main_cfg", false);
  preferences.putULong("stroed", 1);
  preferences.putULong("lampPrevStatus", lampPrevStatus);
  preferences.putULong("fanPrevStatus", fanPrevStatus);
  preferences.end();
  Serial.printf("=== main: %s stored: lampPrevStatus:%u fanPrevStatus:%u\n", __func__, lampPrevStatus, fanPrevStatus);
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
    lampPrevStatus = preferences.getULong("lampPrevStatus", lampPrevStatus);
    fanPrevStatus = preferences.getULong("fanPrevStatus", fanPrevStatus);
    Serial.printf("=== main: %s restored: lampPrevStatus:%u fanPrevStatus:%u\n", __func__, lampPrevStatus, fanPrevStatus);
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

  pirCurrStatus = 0;
  pirPrevStatus = pirCurrStatus;
  lampPrevStatus = pirCurrStatus;
  fanPrevStatus = pirCurrStatus;

  restorePermanetlyConfig();

  if (lampPrevStatus || fanPrevStatus)
  {
    lastLampTriggered = millis();
    lastFanTriggered = lastLampTriggered;
    Serial.printf("=== main: %s continue to trigger pir fan lamp after wifi recoveryed\n", __func__);
  }

  if (!espApp.setup())
  {
    Serial.printf("failed to espApp.setup()\n");
    return;
  }

  espApp.mqtt_.RegisterMqttSwitchSets(mqttSwtichSet, mqttSwtichStates, mqttSwitchHandler, mqttSwtichLen);
  // esp.RegisterMqttConnectedCallback(handleMqttConnected);
  // espApp.dht_.RegisterDhtCallback(handleDhtCallback);
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
    handleLampStatus(now, pirCurrStatus);
    handleFanStatus(now, pirCurrStatus);
  }
}
