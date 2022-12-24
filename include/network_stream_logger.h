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
    const char *streamResponseUri;
    GstreamClient *client;
    bool is_started;

private:
    void initializeFileOutputStream(const char *file, int localPort);
    void onReceived(std::string topic, std::string payload) override;
    void requestStream(bool enable);

public:
    NetworkStreamLogger(const char *file, const char *pubSubServer, int pubSubPort, const char *localIP, int localPort);
    ~NetworkStreamLogger();

    NetworkStreamLogger *withStreamUri(const char *requestUri, const char *responseUri);
    void requestStreamStart();
    void requestStreamStop();
    bool build();

    bool isLogging() {
        return is_started;
    }
};

#endif