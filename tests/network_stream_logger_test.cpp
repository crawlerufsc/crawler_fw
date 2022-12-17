#include "test_video_source.h"

#include <gtest/gtest.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "../include/network_stream_logger.h"
#include "../include/frame.h"
#include <nlohmann/json.hpp>
#include <sstream>

using nlohmann::json;

class DummyServerStreamLogger : public PubSubClient
{
public:
    DummyServerStreamLogger() : PubSubClient("127.0.0.1", 1883, "/tests/request")
    {
        source = nullptr;
    }

private:
    TestVideoSource *source;
    std::string local_ip;
    long local_port;

    void startStream(std::string ip, long port)
    {
        if (source != nullptr)
        {
            std::cerr << "already serving on " << ip << ":" << port << "\n";
            return;
        }

        std::cout << "opening source stream to " << ip << ":" << port << "\n";

        source = new TestVideoSource("videotestsrc ! videoscale ! video/x-raw, width=640, height=480 ! "
                                     "x264enc ! rtph264pay ! udpsink host=127.0.0.1 port=40002");

        json j{
            {"targetIP", ip},
            {"targetPort", port},
            {"enable", true},
        };

        publishTo("/vision-module/status/stream", j.dump());

        local_ip = ip;
        local_port = port;
    }

    void stopStream()
    {
        delete source;
        source = nullptr;

        json j{
            {"targetIP", local_ip},
            {"targetPort", local_port},
            {"enable", false},
        };

        publishTo("/vision-module/status/stream", j.dump());
    }

    void onReceived(std::string topic, std::string payload) override
    {
        std::cout << "received connection request on topic " << topic << " payload: '" << payload << "'\n";
        json s = json::parse(payload);

        std::string ip = s["ip"].get<std::string>();
        int port = s["port"].get<int>();
        bool enable = s["enable"].get<bool>();

        ASSERT_EQ(ip, "127.0.0.1");
        ASSERT_EQ(port, 40002);

        if (enable)
            startStream(ip, port);
        else
            stopStream();
    }
    void onStop()
    {
        delete source;
        source = nullptr;
    }
};

TEST(NetworkStreamLoggerTst, SimpleFileLog)
{
    DummyServerStreamLogger *server = new DummyServerStreamLogger();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    NetworkStreamLogger *logger = new NetworkStreamLogger(
        "test_file.mkv",
        "127.0.0.1",
        1883,
        "127.0.0.1",
        40002);

    logger->withStreamRequestUri("/tests/request");
    logger->requestStreamStart();

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    logger->requestStreamStop();

    delete logger;
    delete server;
}

TEST(NetworkStreamLoggerTst, CreateAndDestroyMultipleTimes)
{
    DummyServerStreamLogger *server = new DummyServerStreamLogger();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (int i = 0; i < 10; i++)
    {
        NetworkStreamLogger *logger = new NetworkStreamLogger(
            "test_file.mkv",
            "127.0.0.1",
            1883,
            "127.0.0.1",
            40002);

        logger->withStreamRequestUri("/tests/request");
        logger->requestStreamStart();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        logger->requestStreamStop();
        delete logger;
    }
}