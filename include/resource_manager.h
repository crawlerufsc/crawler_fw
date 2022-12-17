#ifndef _RESOURCE_MANAGER_H
#define _RESOURCE_MANAGER_H

#include <functional>
#include <iostream>
#include <unordered_map>

using TypeInfoRef = std::reference_wrapper<const std::type_info>;

class ResourceManager
{
private:
    std::unordered_map<std::string, std::function<void *()>> factories;
    std::unordered_map<std::string, void *> singletons;

    ResourceManager()
    {
    }
    ~ResourceManager()
    {
    }
    ResourceManager(const ResourceManager &) = delete;
    ResourceManager &operator=(const ResourceManager &) = delete;

    template <typename T>
    std::function<void *()> convertToVoid(std::function<T *()> f)
    {
        return [=]()
        { return (void *)f(); };
    }

private:
    template <typename T>
    void addFactory(std::function<T *(void)> f)
    {
        addFactory<T>(typeid(T).name(), f);
    }

    template <typename T>
    void addFactory(std::string resourceName, std::function<T *(void)> f)
    {
        factories[resourceName] = convertToVoid(f);
    }

    template <typename T>
    T *getResourceD()
    {
        return getResource<T>(typeid(T).name());
    }

    template <typename T>
    T *getResourceD(std::string resourceName)
    {
        if (factories.find(resourceName) == factories.end())
            return nullptr;

        void *f = factories[resourceName]();
        return (T *)f;
    }

    template <typename T>
    T *getResourceSingleton()
    {
        return getResourceSingleton<T>(typeid(T).name());
    }

    template <typename T>
    T *getResourceSingleton(std::string resourceName)
    {
        if (singletons.find(resourceName) == singletons.end())
        {
            T *singleton = getResourceD<T>(resourceName);
            singletons[resourceName] = (void *)singleton;
            return singleton;
        }
        return (T *)singletons[resourceName];
    }

    template <typename T>
    void removeSingleton()
    {
        return removeSingleton<T>(typeid(T).name());
    }

    template <typename T>
    void removeSingleton(std::string resourceName)
    {
        if (singletons.find(resourceName) == singletons.end())
            return;

        T *resource = (T *)singletons[resourceName];
        delete resource;
        singletons.erase(resourceName);
    }

public:
    static ResourceManager &getInstance()
    {
        static ResourceManager instance;
        return instance;
    }

    template <typename T>
    static void addResourceFactory(std::function<T *()> f)
    {
        getInstance().addFactory<T>(f);
    }

    template <typename T>
    static void addResourceFactory(std::string resourceName, std::function<T *()> f)
    {
        getInstance().addFactory<T>(resourceName, f);
    }

    template <typename T>
    static T *getResource()
    {
        return getInstance().getResourceD<T>();
    }
    template <typename T>
    static T *getResource(std::string resourceName)
    {
        return getInstance().getResourceD<T>(resourceName);
    }

    template <typename T>
    static T *getSingletonResource()
    {
        return getInstance().getResourceSingleton<T>();
    }

    template <typename T>
    static T *getSingletonResource(std::string resourceName)
    {
        return getInstance().getResourceSingleton<T>(resourceName);
    }

    template <typename T>
    static void removeSingletonResource()
    {
        return getInstance().removeSingleton<T>();
    }

    template <typename T>
    static void removeSingletonResource(std::string resourceName)
    {
        return getInstance().removeSingleton<T>(resourceName);
    }

    template <typename T>
    static void usingResourceScope(std::function<void(T *)> scope)
    {
        T *res = getInstance().getResourceD<T>();
        scope(res);
        delete res;
    }

    template <typename T>
    static void usingResourceScope(std::string resourceName, std::function<void(T *)> scope)
    {
        T *res = getInstance().getResourceD<T>(resourceName);
        scope(res);
        delete res;
    }
};

// ResourceManager::ResourceManager() = default;
// ResourceManager::~ResourceManager() = default;

#endif