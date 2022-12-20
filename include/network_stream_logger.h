#ifndef _NETWORK_STREAM_LOGGER
#define _NETWORK_STREAM_LOGGER

#include "gstream_client.h"
#include "pubsub_client.h"

class NetworkStreamLogger : private PubSubClient
{
private:
    const char *file;
    const char *local_ip;
    int local_port;
    const char *streamRequestUri;
    GstreamClient *client;
    bool is_started;

private:
    void initializeFileOutputStream(const char *file, int localPort);
    void onReceived(std::string topic, std::string payload) override;

public:
    NetworkStreamLogger(const char *file, const char *pubSubServer, int pubSubPort, const char *localIP, int localPort);
    ~NetworkStreamLogger();

    NetworkStreamLogger *withStreamRequestUri(const char *streamRequestUri);
    void requestStreamStart();
    void requestStreamStop();

    bool isLogging() {
        return is_started;
    }
};

#endif