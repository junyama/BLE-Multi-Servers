#include <PubSubClient.h>
#include <StreamUtils.h>

#include "MyScanCallbacks.cpp"
#include "VoltMater.hpp"
#include "LipoMater.hpp"
#include "MyLog.cpp"

void mqttServerSetup();
void mqttDeviceSetup();
void reConnectMqttServer();
void publish(String topic, String message);
void publishJson(String topic, JsonDocument doc, bool retained);
void publishHaDiscovery();
void mqttCallback(char *topic_, byte *payload, unsigned int length);