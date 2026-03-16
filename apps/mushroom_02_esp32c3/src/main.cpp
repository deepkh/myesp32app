
#include "EspApp.h"
#include "EspConfig.h"

bool initalSucceeded = false;

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
  espApp.mqtt_.GetMqttClient().publish(g_espconfig.dht_config.mqtt_temperature_set, String(temperature).c_str(), true);
  espApp.mqtt_.GetMqttClient().publish(g_espconfig.dht_config.mqtt_humidity_set, String(humidity).c_str(), true);
  // Serial.printf("HI %f %f\n", temperature, humidity);
}

/**
 *    Air Quality
 */
static void HandleMq135Callback(float value)
{
  espApp.mqtt_.GetMqttClient().publish(g_espconfig.mq135_config.mqtt_set, String(value).c_str(), true);
  // Serial.printf("mq135 %f \n", value);
}

/**
 *    PR Motion Sensor
 */
static int pirPrevStatus = -1;
static int pirBufferCounter = 0;
static void HandleHchr501Callback(int status)
{
  // 0 -> 0
  // 1 -> 1
  if (pirPrevStatus == status)
  {
    pirBufferCounter = 0;
    return;
  }
  else
  {

    // 1 -> 0 && pirBufferCounter < 600  = 300 secs ?
    // Turn off Buffer
    if (pirPrevStatus == 1 && status == 0 && pirBufferCounter < 600)
    {
      pirBufferCounter++;
      return;
    }

    // 0 -> 1
    // 1 -> 0 && pirBufferCounter >= 600
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.hcsr501_config.mqtt_set, status ? "ON" : "OFF");
    espApp.mqtt_.GetMqttClient().publish(g_espconfig.switch2_config.mqtt_set, status ? "ON" : "OFF");

    pirBufferCounter = 0;
    pirPrevStatus = status;
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

  // set default switchs value
  delay(10);
  digitalWrite(g_espconfig.switch1_config.pin, g_espconfig.switch1_config.default_value);


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

  if (!espApp.begin()) {
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
}
