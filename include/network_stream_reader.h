#ifndef _NETWORK_STREAM_READER_H
#define _NETWORK_STREAM_READER_H

// Include gstreamer library
#include <gst/gst.h>
#include <gst/app/app.h>
#include <stdio.h>
#include <string>
#include "frame.h"
#include <mutex>
#include <thread>
#include "pubsub_client.h"
#include "gstream_client.h"
#include <functional>

class NetworkStreamReader : private PubSubClient, private GstreamClient
{
private:
    const char *server_ip;
    const char *local_ip;
    char *pipelineConfig;
    const char *streamRequestUri;
    int server_port;
    int local_port;
    int bufferSize;
    bool isReceivingStream;
    Frame<u_char> *procFrame;

    std::function<void(Frame<u_char> *)> *onProcess;

    void requestStream();

    void onStop() override {}
    void onReceived(std::string topic, std::string payload) override;
    void onFrameReceived(Frame<u_char> *frame) override;
    void startRunProcessPipeline();
    void processThr();
    void requestFrameThr();

    std::mutex *frameMutex;
    bool loop_run;
    bool asyncProcess;

    std::thread *requestFrameThread;
    std::thread *processThread;

protected:
public:
    NetworkStreamReader(const char *pubSubServer, int pubSubPort, const char *localIP, int localPort);
    ~NetworkStreamReader();

    NetworkStreamReader *withStreamRequestUri(const char *streamRequestUri);
    NetworkStreamReader *withBufferSize(int bufferSize);
    NetworkStreamReader *withOnProcessCallback(std::function<void(Frame<u_char> *)> *onProcess);
    NetworkStreamReader *async();

    void connect();
    bool isConnected();
};

#endif