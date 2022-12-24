#include "../include/network_stream_logger.h"

#include <nlohmann/json.hpp>
#include <sstream>

#define PUBSUB_STREAM_RESPONSE_URI "/vision-module/status/stream"

using nlohmann::json;

NetworkStreamLogger::NetworkStreamLogger(const char *file, const char *pubSubServer, int pubSubPort, const char *localIP, int localPort) : PubSubClient(pubSubServer, pubSubPort)
{
    this->file = file;
    this->client = nullptr;
    this->local_ip = localIP;
    this->local_port = localPort;
    this->is_started = false;
}

NetworkStreamLogger::~NetworkStreamLogger()
{
    if (client != nullptr)
        delete client;
}

NetworkStreamLogger *NetworkStreamLogger::withStreamUri(const char *requestUri, const char *responseUri)
{
    this->streamRequestUri = requestUri;
    this->streamResponseUri = responseUri;
    return this;
}

bool NetworkStreamLogger::build()
{
    if (!blockUntilConnected(2000))
        return false;

    if (streamResponseUri != nullptr)
        subscribeTo(streamResponseUri);

    return true;
}

void NetworkStreamLogger::initializeFileOutputStream(const char *file, int localPort)
{
    std::stringstream ss;
    ss << "udpsrc port=" << localPort << " ! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 "
                                         "! rtph264depay ! h264parse ! decodebin ! videoconvert ! x264enc ! matroskamux ! filesink name=sink location="
       << file;

    // std::cout << "\ninitializing LOG stream with: " << ss.str() << "\n\n";

    client = new GstreamClient();
    client->initializeNonInterableStream(ss.str().c_str());
    this->is_started = true;
}

void NetworkStreamLogger::requestStream(bool enable)
{
    if (streamRequestUri == nullptr)
        return;

    json j{
        {"ip", local_ip},
        {"port", local_port},
        {"enable", enable},
    };

    publishTo(streamRequestUri, j.dump());
}

void NetworkStreamLogger::requestStreamStart()
{
    requestStream(true);
}

void NetworkStreamLogger::requestStreamStop()
{
    requestStream(false);
    this->is_started = false;

    if (client == nullptr)
        return;

    client->stop();
    delete client;
    client = nullptr;
}

void NetworkStreamLogger::onReceived(std::string topic, std::string payload)
{
    try
    {
        json p = json::parse(payload);

        if (p["targetIP"].get<std::string>() == std::string(local_ip) &&
            p["targetPort"].get<int>() == local_port)
        {
            initializeFileOutputStream(file, local_port);
        }
    } catch (...) {
        printf ("NetworkStreamLogger: onReceived(): error parsing payload: %s\n\n", payload.c_str());
    }
}