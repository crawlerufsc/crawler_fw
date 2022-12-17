#include "../include/network_stream_logger.h"

#include <nlohmann/json.hpp>
#include <sstream>

using nlohmann::json;

NetworkStreamLogger::NetworkStreamLogger(const char *file, const char *pubSubServer, int pubSubPort, const char *localIP, int localPort) : PubSubClient(pubSubServer, pubSubPort, "/vision-module/status/stream")
{
    this->file = file;
    this->client = nullptr;
    this->local_ip = localIP;
    this->local_port = localPort;
}

NetworkStreamLogger::~NetworkStreamLogger()
{
    if (client != nullptr)
        delete client;
}

NetworkStreamLogger *NetworkStreamLogger::withStreamRequestUri(const char *streamRequestUri)
{
    this->streamRequestUri = streamRequestUri;
    return this;
}

void NetworkStreamLogger::initializeFileOutputStream(const char *file, int localPort)
{
    std::stringstream ss;
    ss << "udpsrc port=" << localPort << " ! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 "
    "! rtph264depay ! h264parse ! decodebin ! videoconvert ! x264enc ! matroskamux ! filesink name=sink location=" << file;

    //std::cout << "initializing stream with: " << ss.str() << "\n\n";
    client = new GstreamClient();
    client->initializeNonInterableStream(ss.str().c_str());
}

void NetworkStreamLogger::requestStreamStart()
{
    blockUntilConnected(2000);

    json j{
        {"ip", local_ip},
        {"port", local_port},
        {"enable", true},
    };

    publishTo(streamRequestUri, j.dump());
}

void NetworkStreamLogger::requestStreamStop()
{
    json j{
        {"ip", local_ip},
        {"port", local_port},
        {"enable", false},
    };

    publishTo(streamRequestUri, j.dump());

    //printf("client->stop()\n");

    client->stop();
    //printf("delete client\n");
    delete client;
    client = nullptr;
}

void NetworkStreamLogger::onReceived(std::string topic, std::string payload)
{
    json p = json::parse(payload);

    if (p["targetIP"].get<std::string>() == std::string(local_ip) &&
        p["targetPort"].get<int>() == local_port)
    {
        initializeFileOutputStream(file, local_port);
    }
}