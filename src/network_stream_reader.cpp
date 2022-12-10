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
}

NetworkStreamReader ::~NetworkStreamReader()
{
    if (loop_run)
    {
        loop_run = false;
        requestFrameThread->join();
        if (processThread != nullptr)
            processThread->join();
    }

    if (procFrame != nullptr)
        delete procFrame;

    if (requestFrameThread != nullptr)
        delete requestFrameThread;

    if (processThread != nullptr)
        delete processThread;

    delete frameMutex;
}

NetworkStreamReader *NetworkStreamReader::withBufferSize(int bufferSize)
{
    this->bufferSize = bufferSize;
    return this;
}

NetworkStreamReader *NetworkStreamReader::withOnProcessCallback(void (*onProcess)(StreamData *))
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
    //printf("connect()\n");
    if (pipelineConfig == nullptr)
    {
        char confmsg[512];
        sprintf(confmsg, "udpsrc port=%d "
                         "! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 "
                         "! rtph264depay ! decodebin ! videoconvert "
                         "! appsink name=sink emit-signals=true sync=false max-buffers=%d drop=true",
                local_port, bufferSize);

        pipelineConfig = g_strdup(confmsg);

        initializeStream(pipelineConfig);
       // printf("set pipelineConfig\n");
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
   // printf("requestStream() to %s, data:%s\n", streamRequestUri, j.dump());
}

void NetworkStreamReader::onFrameReceived(Frame<u_char> *frame)
{
    if (!this->loop_run || this->onProcess == nullptr)
        return;

    this->frameMutex->lock();
    frameRequestWaitForAnswer = false;

    if (asyncProcess && procFrame == nullptr)
    {
        procFrame = frame;
        dropCurrentFrame();
    }
    else
    {
        onProcess(frame);
    }

    this->frameMutex->unlock();
}

void NetworkStreamReader::requestFrameThr()
{
    while (this->loop_run)
    {
        if (frameRequestWaitForAnswer)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        this->frameMutex->lock();
        frameRequestWaitForAnswer = true;
        this->onRequestNextFrame();
        this->frameMutex->unlock();
    }
}

void NetworkStreamReader::processThr()
{
    if (this->onProcess == nullptr)
        return;

    while (this->loop_run)
    {
        if (procFrame != nullptr)
        {
            this->onProcess(procFrame);
            delete procFrame;
            procFrame = nullptr;
        }
    }
}

void NetworkStreamReader::startRunProcessPipeline()
{
    loop_run = true;
    requestFrameThread = new std::thread(&NetworkStreamReader::requestFrameThr, this);
    if (asyncProcess)
        processThread = new std::thread(&NetworkStreamReader::processThr, this);
}

void NetworkStreamReader::onReceived(std::string topic, std::string payload)
{
    json p = json::parse(payload);

    if (p["targetIP"].get<std::string>() == std::string(local_ip) &&
        p["targetPort"].get<int>() == local_port)
    {
        isReceivingStream = true;
        startRunProcessPipeline();
    }
}

bool NetworkStreamReader::isConnected()
{
    return isReceivingStream;
}

void NetworkStreamReader::onRequestNextFrame()
{
    if (!loop_run || !isReceivingStream)
        return;

    requestNextFrame();
}