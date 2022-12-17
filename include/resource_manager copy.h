#ifndef _RESOURCE_MANAGER_H
#define _RESOURCE_MANAGER_H

#include <functional>
#include <iostream>
#include <unordered_map>

using TypeInfoRef = std::reference_wrapper<const std::type_info>;

struct ResourceManagerHasher
{
    std::size_t operator()(TypeInfoRef code) const
    {
        return code.get().hash_code();
    }
};

struct ResourceManagerEqualTo
{
    bool operator()(TypeInfoRef lhs, TypeInfoRef rhs) const
    {
        return lhs.get() == rhs.get();
    }
};

class ResourceManager
{
private:
    std::unordered_map<TypeInfoRef, std::function<void *()>, ResourceManagerHasher, ResourceManagerEqualTo> factories;
    std::unordered_map<TypeInfoRef, void *, ResourceManagerHasher, ResourceManagerEqualTo> singletons;

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
    void addFactoryF(std::function<T *(void)> f)
    {
        // if (factories.find(typeid(T)) != factories.end()) return;
        factories[typeid(T)] = convertToVoid(f);
    }

    template <typename T>
    T *getResourceF(std::string resourceName)
    {
        if (factories.find(typeid(T)) == factories.end())
            return nullptr;

        void *f = factories[typeid(T)]();
        return (T *)f;
    }

    template <typename T>
    T *getResourceS()
    {
        if (singletons.find(typeid(T)) == singletons.end())
        {
            T *singleton = getResourceF<T>();
            singletons[typeid(T)] = (void *)singleton;
            return singleton;
        }
        return (T *)singletons[typeid(T)];
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
        getInstance().addFactoryF<T>(f);
    }

    template <typename T>
    static T *getResource()
    {
        return getInstance().getResourceF<T>();
    }
    
    template <typename T>
    static T *getResource(std::string resourceName)
    {
        return getInstance().getResourceF<T>(resourceName);
    }

    template <typename T>
    static T *getSingletonResource()
    {
        return getInstance().getResourceS<T>();
    }

    template <typename T>
    static T *getSingletonResource(std::string resourceName)
    {
        return getInstance().getResourceS<T>(resourceName);
    }

    template <typename T>
    static void usingResourceScope(std::function<void (T *)> scope)
    {
        T * res = getInstance().getResourceF<T>();
        scope(res);
        delete res;
    }


};

// ResourceManager::ResourceManager() = default;
// ResourceManager::~ResourceManager() = default;

#endif