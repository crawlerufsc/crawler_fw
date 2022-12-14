#include "test_video_source.h"

#include <gtest/gtest.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "../include/network_stream_reader.h"
#include "../include/frame.h"
#include <nlohmann/json.hpp>
#include <sstream>

using nlohmann::json;

class DummyServerStreamReader : public PubSubClient
{
public:
    DummyServerStreamReader() : PubSubClient("127.0.0.1", 1883, "/tests/request")
    {
        source = nullptr;
    }

private:
    TestVideoSource *source;

    void onReceived(std::string topic, std::string payload) override
    {
        std::cout << "received connection request on topic " << topic << " payload: '" << payload << "'\n";
        json s = json::parse(payload);

        std::string ip = s["ip"].get<std::string>();
        int port = s["port"].get<int>();
        bool add = s["enable"].get<bool>();

        ASSERT_EQ(ip, "127.0.0.1");
        ASSERT_EQ(port, 40002);
        ASSERT_TRUE(add);

        if (source != nullptr)
        {
            std::cerr << "already serving on " << ip << ":" << port << "\n";
            FAIL();
        }

        std::cout << "opening source stream to " << ip << ":" << port << "\n";

        source = new TestVideoSource("videotestsrc pattern=snow ! videoscale ! video/x-raw, width=640, height=480 ! "
                                     "x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink host=127.0.0.1 port=40002");

        json j{
            {"targetIP", ip},
            {"targetPort", port},
            {"enable", true},
        };

        publishTo("/vision-module/status/stream", j.dump());
    }
    void onStop()
    {
        delete source;
        source = nullptr;
    }
};

int frameCount = 0;

void onProcess(StreamData *frame)
{
    //std::cout << "received frame\n";
    if (frame == nullptr)
    {
        std::cout << "frame shouldn't be nullptr\n";
        FAIL();
    }
    frameCount++;
}

TEST(NetworkStreamReaderTst, SimpleReceiveSync)
{
    frameCount = 0;
    DummyServerStreamReader *server = new DummyServerStreamReader();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    NetworkStreamReader *reader = (new NetworkStreamReader("127.0.0.1", 1883, "127.0.0.1", 40002))
                                      ->withBufferSize(1)
                                      ->withOnProcessCallback(&onProcess)
                                      ->withStreamRequestUri("/tests/request");
    reader->connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    ASSERT_TRUE(frameCount > 0);
    delete reader;
    delete server;
}

TEST(NetworkStreamReaderTst, SimpleReceiveSyncAsync)
{
    frameCount = 0;
    DummyServerStreamReader *server = new DummyServerStreamReader();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    NetworkStreamReader *reader = (new NetworkStreamReader("127.0.0.1", 1883, "127.0.0.1", 40002))
                                      ->withBufferSize(1)
                                      ->withOnProcessCallback(&onProcess)
                                      ->withStreamRequestUri("/tests/request")
                                      ->async();
    reader->connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    ASSERT_TRUE(frameCount > 0);
    delete reader;
    delete server;
}
