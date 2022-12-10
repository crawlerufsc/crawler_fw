#include "../include/pubsub_client.h"

#include <string.h>

PubSubClient::PubSubClient(const char *mqttHost, int mqttPort, const char *defaultTopic)
{
    this->mqttHost = mqttHost;
    this->mqttPort = mqttPort;
    this->messageId = -1;
    this->is_connected = false;
    this->defaultTopic = defaultTopic;
    loop_start();
    connect_async(this->mqttHost, this->mqttPort, 60);
}

PubSubClient::PubSubClient(const char *mqttHost, int mqttPort)
{
    this->mqttHost = mqttHost;
    this->mqttPort = mqttPort;
    this->messageId = -1;
    this->is_connected = false;
    this->defaultTopic = nullptr;
    loop_start();
    connect_async(this->mqttHost, this->mqttPort, 60);
}

void PubSubClient::on_connect(int rc)
{
    if (!rc)
    {
        is_connected = true;
#ifdef DEBUG
        std::cout << "Connected - code " << rc << std::endl;
#endif
    }

    if (this->defaultTopic != nullptr)
    {
        subscribeTo(this->defaultTopic);
    }
}

bool PubSubClient::blockUntilConnected(int timeout_ms)
{
    while (!is_connected || timeout_ms <= 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        timeout_ms--;
    }
    return timeout_ms > 0;
}

void PubSubClient::subscribeTo(const char *topic)
{
    printf("subscribing to topic %s\n", topic);
    int id = getNextMessageId();
    this->subscribe(&id, topic, 1);
}

int PubSubClient::getNextMessageId()
{
    if (messageId > 1000000)
        messageId = -1;
    return ++messageId;
}

void PubSubClient::on_disconnect(int rc)
{
    is_connected = false;
    // loop_start();
    // connect_async(this->mqttHost, this->mqttPort, 60);
}

void PubSubClient::on_message(const struct mosquitto_message *message)
{
    if (message->payloadlen < 1)
        return;

    char *payload_data = (char *)malloc(message->payloadlen);
    memcpy(payload_data, message->payload, message->payloadlen);

    std::string topic(message->topic);
    std::string payload(payload_data);

    onReceived(topic, payload);
}

void PubSubClient::publishTo(const char *topic, std::string payload)
{
    int messageId = getNextMessageId();
    publish(&messageId, topic, payload.size(), payload.c_str(), 1);
}

void PubSubClient::Stop()
{
    loop_stop();
}

// void PubSubClient::runMqttLoop()
// {
//     loop_forever();
// }