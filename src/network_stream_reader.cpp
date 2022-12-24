// Include atomic std library
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string>
#include "../include/network_stream_reader.h"
#include <iostream>
#include <nlohmann/json.hpp>

using nlohmann::json;

NetworkStreamReader::NetworkStreamReader(const char *serverIP, int serverPort, const char *localIP, int localPort) : PubSubClient(serverIP, serverPort, "/vision-module/status/stream")
{
    this->bufferSize = 1;
    this->server_ip = serverIP;
    this->server_port = serverPort;
    this->local_ip = localIP;
    this->local_port = localPort;
    this->isReceivingStream = false;
    this->frameMutex = new std::mutex();
    this->procFrame = nullptr;
    this->pipelineConfig = nullptr;

    this->asyncProcess = false;
    this->streamRequestUri = nullptr;
    this->onProcess = nullptr;
}

NetworkStreamReader ::~NetworkStreamReader()
{
    if (loop_run)
    {
        loop_run = false;
        if (processThread != nullptr)
            processThread->join();
    }

    if (procFrame != nullptr)
        delete procFrame;

    delete frameMutex;
}

NetworkStreamReader *NetworkStreamReader::withBufferSize(int bufferSize)
{
    this->bufferSize = bufferSize;
    return this;
}

NetworkStreamReader *NetworkStreamReader::withOnProcessCallback(std::function<void(Frame<u_char> *)> *onProcess)
{
    this->onProcess = onProcess;
    return this;
}
NetworkStreamReader *NetworkStreamReader::withStreamRequestUri(const char *streamRequestUri)
{
    this->streamRequestUri = streamRequestUri;
    return this;
}

NetworkStreamReader *NetworkStreamReader::async()
{
    this->asyncProcess = true;
    return this;
}

void NetworkStreamReader::connect()
{
    if (pipelineConfig == nullptr)
    {
        pipelineConfig = (char *)malloc(sizeof(char) * 512);

        sprintf(pipelineConfig, "udpsrc port=%d "
                                "! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 "
                                "! rtph264depay ! decodebin ! videoconvert "
                                "! appsink name=sink emit-signals=true sync=false max-buffers=%d drop=true",
                local_port, bufferSize);
    }

    requestStream();
}

void NetworkStreamReader::requestStream()
{
    json j{
        {"ip", local_ip},
        {"port", local_port},
        {"enable", true},
    };

    publishTo(streamRequestUri, j.dump());
}

void NetworkStreamReader::onFrameReceived(Frame<u_char> *frame)
{
    // printf("onFrameReceived @%p\n",frame);
    if (!this->loop_run || this->onProcess == nullptr)
        return;

    this->frameMutex->lock();

    if (asyncProcess)
    {
        if (procFrame == nullptr)
        {
            // printf("get frame to process: %p\n", frame);
            procFrame = frame;
            dropCurrentFrame();
        }
    }
    else
    {
        (*onProcess)(frame);
    }

    this->frameMutex->unlock();
}

void NetworkStreamReader::processThr()
{
    if (this->onProcess == nullptr)
    {
        // printf("onProcess is null\n");
        return;
    }

    while (this->loop_run)
    {
        if (procFrame != nullptr)
        {
            (*this->onProcess)(procFrame);
            delete procFrame;
            procFrame = nullptr;
        }
    }
}

void NetworkStreamReader::startRunProcessPipeline()
{
    loop_run = true;
    if (asyncProcess)
        processThread = new std::thread(&NetworkStreamReader::processThr, this);
}

void NetworkStreamReader::onReceived(std::string topic, std::string payload)
{
    if (isReceivingStream)
        return;

    printf("received answer for stream on topic %s. Payload: %s\n", topic.c_str(), payload.c_str());

    json p = json::parse(payload);

    if (p["targetIP"].get<std::string>() == std::string(local_ip) &&
        p["targetPort"].get<int>() == local_port)
    {
        isReceivingStream = true;

        initializeStream(pipelineConfig);
        startRunProcessPipeline();
        requestNextFrame();
    }
}

bool NetworkStreamReader::isConnected()
{
    return isReceivingStream;
}
