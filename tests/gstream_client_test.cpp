#include "test_video_source.h"

#include <gtest/gtest.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "../include/gstream_client.h"
#include "../include/frame.h"

class GstreamClientTest : public GstreamClient
{
public:
    GstreamClientTest() : GstreamClient() {
        frameCount = 0;
    }

protected:
    void onFrameReceived(Frame<u_char> *frame) override
    {
        //std::cout << "received a new frame\n";
        ASSERT_EQ(frame->width, 640);
        ASSERT_EQ(frame->height, 480);
        frameCount++;
        ASSERT_TRUE(frame->data != nullptr);
        ASSERT_EQ(frame->len, 6*640*480);
    }

public:
    int frameCount;
};

TEST(GStreamClient, SimpleReceive)
{
    const char *sourcePipeline = "videotestsrc pattern=snow ! videoscale ! video/x-raw, width=640, height=480 ! "
                                 "x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink host=127.0.0.1 port=30001";

    const char *sinkPipeline = "udpsrc port=30001 "
                     "! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 "
                     "! rtph264depay ! decodebin ! videoconvert "
                     "! appsink name=sink emit-signals=true sync=false max-buffers=1 drop=true";

    TestVideoSource source(sourcePipeline);

    GstreamClientTest *test = new GstreamClientTest();
    test->initializeStream(sinkPipeline);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_TRUE(test->frameCount > 0);
    delete test;
}

TEST(GStreamClient, StopReceiving)
{
    const char *sourcePipeline = "videotestsrc pattern=snow ! videoscale ! video/x-raw, width=640, height=480 ! "
                                 "x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink host=127.0.0.1 port=30001";

    const char *sinkPipeline = "udpsrc port=30001 "
                     "! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96 "
                     "! rtph264depay ! decodebin ! videoconvert "
                     "! appsink name=sink emit-signals=true sync=false max-buffers=1 drop=true";

    TestVideoSource source(sourcePipeline);

    GstreamClientTest test;
    test.initializeStream(sinkPipeline);
    test.stop();
    ASSERT_TRUE(test.frameCount == 0);

    test.restart();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(test.frameCount > 0);
}