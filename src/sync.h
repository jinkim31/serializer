#ifndef SYNC_SYNC_H
#define SYNC_SYNC_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <concepts>
#include <functional>

namespace sync
{

// forward decl
class Serializable;

class Syncable
{
public:
    Syncable(const std::string& name){mName = name;}
    virtual ~Syncable()= default;
    virtual void save(nlohmann::json& j) = 0;
    virtual void load(const nlohmann::json& j) = 0;
    std::string& name(){return mName;}
private:
    std::string mName;
};


// function wrappers for save function partial specialization
template <typename T, bool>
struct Saver{};
template<typename T> struct Saver<T, true>
{
    T* mPtr;
    void save(nlohmann::json &j) {mPtr->save(j);}
};
template<typename T> struct Saver<T, false>
{
    T* mPtr;
    void save(nlohmann::json &j) {j = *(mPtr);}
};

// function wrappers for load function partial specialization
template <typename T, bool>
struct Loader{};
template<typename T> struct Loader<T, true>
{
    T* mPtr;
    void load(const nlohmann::json &j){mPtr->load(j);}
};
template<typename T> struct Loader<T, false>
{
    T* mPtr;
    void load(const nlohmann::json &j){*(mPtr) = j;}
};

template <typename T>
class Sync : public Syncable
{
public:
    Sync(T* ptr, const std::string& name);
    void save(nlohmann::json &j) override;
    void load(const nlohmann::json &j) override;
private:
    T* mPtr;
    Saver<T, std::derived_from<T, Serializable>> mSaver;
    Loader<T, std::derived_from<T, Serializable>> mLoader;
};

template<typename T>
Sync<T>::Sync(T *ptr, const std::string &name) : Syncable(name)
{
    mPtr = ptr;
    mSaver.mPtr = ptr;
    mLoader.mPtr = ptr;
}

template<typename T>
void Sync<T>::save(nlohmann::json &j)
{
    mSaver.save(j);
}

template<typename T>
void Sync<T>::load(const nlohmann::json &j)
{
    mLoader.load(j);
}

class Serializable
{
public:
    virtual void save(nlohmann::json& j);
    virtual void load(const nlohmann::json& j);
protected:
    virtual ~Serializable()= default;
    virtual void listSync(){};
    virtual void onSyncSave(){};
    virtual void onSyncLoad(){};
    template <typename T>
    void sync(T* ptr, const std::string& name)
    {
        auto sync = std::make_shared<Sync<T>>(ptr, name);
        mSyncs.emplace_back(sync);
    }
private:
    std::vector<std::shared_ptr<Syncable>> mSyncs;
};

template <typename T>
class SerializableVector : public Serializable
{
public:
    void save(nlohmann::json &j) override;
    void load(const nlohmann::json &j) override;
    std::vector<T>& get();
private:
    std::vector<T> mVec;
    Loader<T, std::derived_from<T, Serializable>> mLoader;
};

template<typename T>
void SerializableVector<T>::save(nlohmann::json &j)
{
    for(auto& elem : mVec)
    {
        nlohmann::json jElem;
        Saver<T, std::derived_from<T, Serializable>> mSaver{&elem};
        mSaver.save(jElem);
        j.push_back(std::move(jElem));
    }
}

template<typename T>
void SerializableVector<T>::load(const nlohmann::json &j)
{
    mVec.clear();
    for(const auto& jElem : j)
    {
        T elem;
        mLoader.mPtr = &elem;
        mLoader.load(jElem);
        mVec.push_back(std::move(elem));
    }
}

template<typename T>
std::vector<T> &SerializableVector<T>::get()
{
    return mVec;
}

template <typename KeyType, typename ValueType>
class SerializableMap : public Serializable
{
public:
    void save(nlohmann::json &j) override;
    void load(const nlohmann::json &j) override;
    std::map<KeyType, ValueType>& get();
private:
    std::map<KeyType, ValueType> mMap;
    Loader<KeyType, std::derived_from<KeyType, Serializable>> mKeyLoader;
    Loader<ValueType, std::derived_from<ValueType, Serializable>> mValueLoader;
};

template <typename KeyType, typename ValueType>
void SerializableMap<KeyType, ValueType>::save(nlohmann::json &j)
{
    for(auto& elem : mMap)
    {
        nlohmann::json jElem;
        Saver<KeyType, std::derived_from<KeyType, Serializable>> mKeySaver{const_cast<KeyType*>(&elem.first)};
        Saver<ValueType, std::derived_from<ValueType, Serializable>> mValueSaver{const_cast<ValueType*>(&elem.second)};
        mKeySaver.save(jElem["key"]);
        mValueSaver.save(jElem["value"]);
        j.push_back(std::move(jElem));
    }
}

template <typename KeyType, typename ValueType>
void SerializableMap<KeyType, ValueType>::load(const nlohmann::json &j)
{
    mMap.clear();
    for(const auto& jElem : j)
    {
        KeyType key;
        ValueType value;
        mKeyLoader.mPtr = &key;
        mValueLoader.mPtr = &value;
        mKeyLoader.load(jElem["key"]);
        mValueLoader.load(jElem["value"]);
        mMap.emplace(std::move(key), std::move(value));
    }
}

template <typename KeyType, typename ValueType>
std::map<KeyType, ValueType>& SerializableMap<KeyType, ValueType>::get()
{
    return mMap;
}

template <typename T>
class SerializablePolymorphicVector : Serializable
{
public:
    struct PolymorphicSharedPtr
    {
        std::shared_ptr<T> ptr;
        std::string typeName;
    };
    SerializablePolymorphicVector(const std::function<std::shared_ptr<T>(const std::string&)> &factory);
    std::vector<PolymorphicSharedPtr>& get();
    void save(nlohmann::json &j) override;
    void load(const nlohmann::json &j) override;
private:
    std::vector<PolymorphicSharedPtr> mVec;
    std::function<std::shared_ptr<T>(const std::string&)> mFactory;
    Loader<T, std::derived_from<T, Serializable>> mLoader;
};

template<typename T>
SerializablePolymorphicVector<T>::SerializablePolymorphicVector(const std::function<std::shared_ptr<T>(const std::string&)> &factory)
{
    mFactory = factory;
}

template<typename T>
std::vector<typename SerializablePolymorphicVector<T>::PolymorphicSharedPtr> &SerializablePolymorphicVector<T>::get()
{
    return mVec;
}

template<typename T>
void SerializablePolymorphicVector<T>::save(nlohmann::json &j)
{
    for(auto& elem : mVec)
    {
        nlohmann::json jElem;
        Saver<T, std::derived_from<T, Serializable>> mSaver{elem.ptr.get()};
        jElem["type"] = elem.typeName;
        mSaver.save(jElem["data"]);
        j.push_back(std::move(jElem));
    }
}

template<typename T>
void SerializablePolymorphicVector<T>::load(const nlohmann::json &j)
{
    mVec.clear();
    for(const auto& jElem : j)
    {
        PolymorphicSharedPtr elem;
        std::shared_ptr<T> ptr = mFactory(jElem["type"]);
        if(!ptr)
            throw std::runtime_error("[Sync] Factory returned nullptr while SerializablePolymorphicVector load().");
        mLoader.mPtr = ptr.get();
        mLoader.load(jElem["data"]);
        mVec.push_back({ptr, jElem["type"]});
    }
}

}
#endif
