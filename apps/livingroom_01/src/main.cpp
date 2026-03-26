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

};
static const char *mqttSwtichStates[] = {
    g_espconfig.switch1_config.mqtt_status,
};
static const int mqttSwtichLen = sizeof(mqttSwtichSet) / sizeof(mqttSwtichSet[0]);

static int mqttSwitchCallbackCounter[] = {
    0,
};

static unsigned long now = 0;

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
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch1_config.mqtt_set, g_espconfig.switch1_config.default_value ? "OFF" : "ON");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch1_config.mqtt_set, g_espconfig.switch1_config.default_value ? "OFF" : "ON");
      break;
    }
  }
  else
  {
    switch (index)
    {
    case 0:
      digitalWrite(g_espconfig.switch1_config.pin, status ? LOW : HIGH );
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

  espApp.mqtt_.RegisterMqttSwitchSets(mqttSwtichSet, mqttSwtichStates, mqttSwitchHandler, mqttSwtichLen);
  // esp.RegisterMqttConnectedCallback(handleMqttConnected);
  // espApp.dht_.RegisterDhtCallback(handleDhtCallback);
  // espApp.mq135_.RegisterMq135Callback(handleMq135Callback);
  // espApp.hcsr501_.RegisterHchr501Callback(handleHchr501Callback);

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
}
