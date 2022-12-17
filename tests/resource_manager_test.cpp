#include <gtest/gtest.h>
#include <stdlib.h>

#include "../include/resource_manager.h"

static bool onDeleteWasCalled = false;

class TestClassA
{
public:
    TestClassA()
    {
        i = 0;
    }
    int i;

    ~TestClassA()
    {
        onDeleteWasCalled = true;
    }
};

TEST(ResourceManagerTest, NonSingletonResourceTest)
{
    TestClassA *a1 = new TestClassA();
    ASSERT_EQ(0, a1->i);

    ResourceManager::addResourceFactory<TestClassA>([]
                                                    {
        TestClassA *p = new TestClassA();
        p->i = 3;
        return p; });

    TestClassA *a2 = ResourceManager::getResource<TestClassA>();
    ASSERT_EQ(3, a2->i);
}

TEST(ResourceManagerTest, SingletonResourceTest)
{
    TestClassA *a1 = new TestClassA();
    ASSERT_EQ(0, a1->i);

    ResourceManager::addResourceFactory<TestClassA>([]
                                                    {
        TestClassA *p = new TestClassA();
        p->i = 3;
        return p; });

    TestClassA *a2 = ResourceManager::getSingletonResource<TestClassA>();
    ASSERT_EQ(3, a2->i);

    a2->i = 5;
    a2 = nullptr;

    a2 = ResourceManager::getSingletonResource<TestClassA>();
    ASSERT_EQ(5, a2->i);
}

TEST(ResourceManagerTest, UsingResourceScope)
{
    ResourceManager::addResourceFactory<TestClassA>([=]()
                                                    {
        TestClassA *p = new TestClassA();
        p->i = 3;
        return p; });

    ResourceManager::usingResourceScope<TestClassA>([=](TestClassA *a)
                                                    { ASSERT_EQ(a->i, 3); });

    ASSERT_TRUE(onDeleteWasCalled);
}