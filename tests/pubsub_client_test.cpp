#include <gtest/gtest.h>
#include <stdlib.h>
#include <set>

#include "../include/pubsub_client.h"

class Publisher : public PubSubClient
{
public:
    bool received;

    Publisher() : PubSubClient("127.0.0.1", 1883) {}

    void onReceived(std::string topic, std::string payload) override
    {
        FAIL();
    }
};

class Subscriber : public PubSubClient
{
public:
    bool received;
    Subscriber() : PubSubClient("127.0.0.1", 1883, "topic1")
    {
        received = false;
    }

    void onReceived(std::string topic, std::string payload) override
    {
        ASSERT_EQ("PUBLISHED_MSG_1", payload);
        received = true;
    }
};

TEST(PubSubClientTest, SimplePublishSubscribe)
{
    Publisher p;
    Subscriber s;

    ASSERT_TRUE(p.blockUntilConnected(1000));
    ASSERT_TRUE(s.blockUntilConnected(1000));

    s.received = false;
    p.received = false;

    p.publishTo("topic1", "PUBLISHED_MSG_1");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(s.received);
    ASSERT_FALSE(p.received);

    s.received = false;
    p.received = false;

    p.publishTo("topic11", "PUBLISHED_MSG_1");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(s.received);
    ASSERT_FALSE(p.received);
}