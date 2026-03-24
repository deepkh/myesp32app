
#include "EspApp.h"
#include "EspConfig.h"

const char *mqttSwtichSet[] = {
    g_espconfig.switch1_config.mqtt_set,
};
const char *mqttSwtichStates[] = {
    g_espconfig.switch1_config.mqtt_status,
};
const int mqttSwtichLen = sizeof(mqttSwtichSet) / sizeof(mqttSwtichSet[0]);

static int mqttSwitchCallbackCounter[] = {
    0};

static void mqttSwitchHandler(unsigned int index, const char *topic, const char *msg)
{
  Serial.printf("mqttSwitchHandler %d '%s' -> '%s'\n", index, topic, msg);

  // Sync up in the initial stage
  if (mqttSwitchCallbackCounter[index] == 0)
  {
    switch (index)
    {
    case 0:
      Serial.printf("mqttSwitchHandler %d '%s' sync up -> '%s'\n", index, g_espconfig.switch1_config.mqtt_set, g_espconfig.switch1_config.default_value ? "ON" : "OFF");
      espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch1_config.mqtt_set, g_espconfig.switch1_config.default_value ? "ON" : "OFF");
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
    }
  }

  mqttSwitchCallbackCounter[index]++;
}

/**
 *    Temp & Humidity
 */
static void HandleDhtCallback(float temperature, float humidity)
{
}

/**
 *    Air Quality
 */
static void HandleMq135Callback(float value)
{
}

/**
 *    PR Motion Sensor
 */
static void HandleHchr501Callback(int status)
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
  // esp.RegisterMqttConnectedCallback(HandleMqttConnected);
  // espApp.dht_.RegisterDhtCallback(HandleDhtCallback);
  // espApp.mq135_.RegisterMq135Callback(HandleMq135Callback);
  // espApp.hcsr501_.RegisterHchr501Callback(HandleHchr501Callback);

  if (!espApp.begin())
  {
    Serial.printf("failed to esp.Begin()\n");
    return;
  }

}

void loop()
{
  unsigned long now = millis();
  espApp.loop(now);
}
