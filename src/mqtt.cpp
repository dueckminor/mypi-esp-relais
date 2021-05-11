#include <Arduino.h>
#include <PubSubClient.h>
#include "mqtt.h"
#include "wifi.h"
#include "settings.h"

const char* mqtt_server = MYPI_HOST;

static String topicPrefix;
static String clientID;

static PubSubClient mqttClient(WifiGetClient());

static void (*mqttCallbackFn)(const char* topic, const byte* payload, unsigned int length) = NULL;

static void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  if (strlen(topic) <= topicPrefix.length())
  {
    return;
  }
  topic+=topicPrefix.length();
  if (mqttCallbackFn != NULL) {
    mqttCallbackFn(topic,payload,length);
  }
}

void MqttSetCallback(void fn(const char* topic, const byte* payload, unsigned int length))
{
  mqttCallbackFn = fn;
}

void MqttInitialize()
{
    mqttClient.setServer(mqtt_server, 8883);
    mqttClient.setCallback(mqttCallback);
}

bool mqttLoopConnected = false;

void MqttEnsureConnected();
bool MqttPublish(const char* topic, const char* payload);

void MqttBeginLoop() {
  mqttLoopConnected = false;
}

void mqttReconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    // Create a random client ID
    clientID = "esp/";
    clientID += WiFi.macAddress();
    clientID.replace(":","");

    topicPrefix = clientID + "/";

    String statusTopic = topicPrefix + "status";
    // Attempt to connect
    if (mqttClient.connect(clientID.c_str(),statusTopic.c_str(),2,true,"down")) {
      // Once connected, publish an announcement...
      MqttEnsureConnected();
      mqttClient.publish(statusTopic.c_str(), "up", true);
      // ... and resubscribe
      mqttClient.subscribe((topicPrefix+"#").c_str());
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void MqttEnsureConnected() {
  if (!mqttLoopConnected) {
    if (!mqttClient.loop()) {
      mqttReconnect();
    }
    mqttLoopConnected = true;
  }
}

bool MqttPublish(const char* topic, const char* payload) {
  MqttEnsureConnected();
  return mqttClient.publish(topic, payload);
}

void mqttKeepAlive() {
  static unsigned long lastTime = 0;
  unsigned long now = millis();
  if (!mqttLoopConnected) {
    if ((now-lastTime) < 1000) {
      return;
    }
    MqttEnsureConnected();
  }

  lastTime = millis();
}

MqttTopic::MqttTopic(const char * topic) : m_topic(topicPrefix+topic)
{

}

MqttTopic::MqttTopic(const char * group, const char * topic) : m_topic(topicPrefix+group+"/"+topic)
{

}

bool MqttTopic::Publish(const char * payload)
{
  return MqttPublish(m_topic.c_str(),payload);
}

