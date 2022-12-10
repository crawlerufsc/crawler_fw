#include <gtest/gtest.h>
#include <stdlib.h>
#include <set>

#include "../include/pubsub_client.h"

class Publisher : public PubSubClient
{
public:
    bool received;

    Publisher() : PubSubClient("127.0.0.1", 1883, nullptr) {}

    void onReceived(std::string topic, std::string payload) override
    {
        FAIL();
    }
    void onStop() override
    {
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
    void onStop() override
    {
    }
};

TEST(PubSubClientTest, SimplePublishSubscribe)
{
    Publisher p;
    Subscriber s;

    p.Publish("topic1", "PUBLISHED_MSG_1");
    ASSERT_TRUE(s.received);
    ASSERT_FALSE(p.received);

    p.Publish("topic11", "PUBLISHED_MSG_1");
    ASSERT_FALSE(s.received);
    ASSERT_FALSE(p.received);
}