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

bool initalSucceeded = false;

const char *mqttSwtichSet[] = {
    g_espconfig.switch1_config.mqtt_set,
    g_espconfig.switch2_config.mqtt_set,
};
const char *mqttSwtichStates[] = {
    g_espconfig.switch1_config.mqtt_status,
    g_espconfig.switch2_config.mqtt_status,
};
const int mqttSwtichLen = sizeof(mqttSwtichSet) / sizeof(mqttSwtichSet[0]);

static int mqttSwitchCallbackCounter[] = {
    0,
    0,
};

static int pirCurrStatus = -1;
static int pirPrevStatus = -1;
unsigned long lastPirTriggered = 0;

static int lamp1PrevStatus = -1;
unsigned long lastLamp1Triggered = 0;

static int lamp2PrevStatus = -1;
unsigned long lastLamp2Triggered = 0;

static void mqttSwitchHandler(unsigned int index, const char *topic, const char *msg)
{
  Serial.printf("mqttSwitchHandler %d '%s' -> '%s'\n", index, topic, msg);

  // Sync up in the initial stage
  if (mqttSwitchCallbackCounter[index] == 0)
  {
    switch (index)
    {
    case 0:
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch1_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch1_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.hcsr501_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      break;
    case 1:
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch2_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch2_config.mqtt_set, pirCurrStatus ? "ON" : "OFF");
      break;
    }
  }
  else
  {
    switch (index)
    {
    case 0:
      digitalWrite(g_espconfig.switch1_config.pin, strcmp(msg, "ON") == 0 ? HIGH : LOW);
      break;
    case 1:
      digitalWrite(g_espconfig.switch2_config.pin, strcmp(msg, "ON") == 0 ? HIGH : LOW);
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

static void handleLamp1Status(unsigned int now, int status)
{
  if (lamp1PrevStatus == 0 && status == 0)
  {
  }
  else if (lamp1PrevStatus == 0 && status == 1)
  {
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch1_config.mqtt_set, "ON");
    lamp1PrevStatus = status;
    lastLamp1Triggered = now;
  }
  else if (lamp1PrevStatus == 1 && status == 0)
  {
    // Turn lamp1 off after 30 seconds
    if ((now - lastLamp1Triggered) < 30000)
    {
      return;
    }
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch1_config.mqtt_set, "OFF");
    lamp1PrevStatus = status;
  }
  else if (lamp1PrevStatus == 1 && status == 1)
  {
    lastLamp1Triggered = now;
  }
}

static void handleLamp2Status(unsigned int now, int status)
{
  if (lamp2PrevStatus == 0 && status == 0)
  {
  }
  else if (lamp2PrevStatus == 0 && status == 1)
  {
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch2_config.mqtt_set, "ON");
    lamp2PrevStatus = status;
    lastLamp2Triggered = now;
  }
  else if (lamp2PrevStatus == 1 && status == 0)
  {
    // Turn lamp2 off after 20 seconds
    if ((now - lastLamp2Triggered) < 20000)
    {
      return;
    }
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch2_config.mqtt_set, "OFF");
    lamp2PrevStatus = status;
  }
  else if (lamp2PrevStatus == 1 && status == 1)
  {
    lastLamp2Triggered = now;
  }
}

void setup()
{
  Serial.printf("a\n");

  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(g_espconfig.switch1_config.pin, OUTPUT);
  pinMode(g_espconfig.switch2_config.pin, OUTPUT);

  // set default switchs value
  delay(10);
  digitalWrite(g_espconfig.switch1_config.pin, g_espconfig.switch1_config.default_value);
  digitalWrite(g_espconfig.switch2_config.pin, g_espconfig.switch2_config.default_value);

  if (g_espconfig.switch1_config.default_value)
  {
    lastPirTriggered = millis();
    lastLamp1Triggered = lastPirTriggered;
    lastLamp2Triggered = lastPirTriggered;
  }

  pirCurrStatus = g_espconfig.switch1_config.default_value;
  pirPrevStatus = pirCurrStatus;
  lamp1PrevStatus = pirCurrStatus;
  lamp2PrevStatus = pirCurrStatus;

  if (!espApp.setup())
  {
    Serial.printf("failed to espApp.setup()\n");
    return;
  }

  espApp.mqtt_.RegisterMqttSwitchSets(mqttSwtichSet, mqttSwtichStates, mqttSwitchHandler, mqttSwtichLen);
  // esp.RegisterMqttConnectedCallback(HandleMqttConnected);
  espApp.dht_.RegisterDhtCallback(handleDhtCallback);
  espApp.mq135_.RegisterMq135Callback(handleMq135Callback);
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

  unsigned long now = millis();
  espApp.loop(now);
  handlePirStatus(now, pirCurrStatus);
  handleLamp1Status(now, pirCurrStatus);
  handleLamp2Status(now, pirCurrStatus);
}
