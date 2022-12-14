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

    void (*onProcess)(Frame<u_char> *frame);

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
    NetworkStreamReader *withOnProcessCallback(void (*onProcess)(Frame<u_char> *));
    NetworkStreamReader *async();

    void connect();
    bool isConnected();
};

#endif