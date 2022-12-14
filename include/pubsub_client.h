#ifndef _PUBSUB_CLIENT_H
#define _PUBSUB_CLIENT_H

#include <mosquittopp.h>
#include <string>
#include <thread>


class PubSubClient : public mosqpp::mosquittopp
{
private:
    const char *mqttHost;
    int mqttPort;
    bool is_connected;
    int messageId;

    void on_connect(int rc);
    void on_disconnect(int rc);
    void on_message(const struct mosquitto_message *message);
    int getNextMessageId();

protected:
    virtual void onReceived(std::string topic, std::string payload) {};
    virtual void onStop() {};
    const  char *defaultTopic;
    

    
public:
    PubSubClient(const char *mqttHost, int mqttPort, const char *topic);
    PubSubClient(const char *mqttHost, int mqttPort);

    void publishTo(const char *topic, std::string payload);
    
    void subscribeTo(const char *topic);

    bool isConnected() {
        return is_connected;
        
    }

    bool blockUntilConnected(int timeout);

    void Stop();
};

#endif