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

static bool initalSucceeded = false;

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
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch2_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch2_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.hcsr501_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      break;
    case 1:
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch3_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch3_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
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
  espApp.mqtt_.GetMqttClient().publish(g_espconfig.dht_config.mqtt_temperature_set, String(temperature).c_str(), true);
  espApp.mqtt_.GetMqttClient().publish(g_espconfig.dht_config.mqtt_humidity_set, String(humidity).c_str(), true);
  // Serial.printf("HI %f %f\n", temperature, humidity);
}

/**
 *    Air Quality
 */
static void handleMq135Callback(float value)
{
  espApp.mqtt_.GetMqttClient().publish(g_espconfig.mq135_config.mqtt_set, String(value).c_str(), true);
  // Serial.printf("mq135 %f \n", value);
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
    // Turn fan off after 10800 seconds
    if ((now - lastFanTriggered) < 10800000)
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

void setup()
{
  Serial.printf("a\n");

  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(g_espconfig.switch2_config.pin, OUTPUT);
  pinMode(g_espconfig.switch3_config.pin, OUTPUT);

  // set default switchs value
  delay(10);
  digitalWrite(g_espconfig.switch2_config.pin, g_espconfig.switch2_config.default_value);
  digitalWrite(g_espconfig.switch3_config.pin, g_espconfig.switch2_config.default_value);

  if (g_espconfig.switch2_config.default_value)
  {
    lastPirTriggered = millis();
    lastFanTriggered = lastPirTriggered;
    lastLampTriggered = lastPirTriggered;
  }

  pirCurrStatus = g_espconfig.switch2_config.default_value;
  pirPrevStatus = pirCurrStatus;
  lampPrevStatus = pirCurrStatus;
  fanPrevStatus = pirCurrStatus;

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

  if (!espApp.begin())
  {
    Serial.printf("failed to esp.Begin()\n");
    return;
  }

  initalSucceeded = true;
}

void loop()
{
  if (!initalSucceeded)
  {
    return;
  }

  now = millis();
  espApp.loop(now);
  handlePirStatus(now, pirCurrStatus);
  handleLampStatus(now, pirCurrStatus);
  handleFanStatus(now, pirCurrStatus);
}
