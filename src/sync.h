#ifndef MODELER_MODELER_H
#define MODELER_MODELER_H

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <concepts>

namespace sync
{

// forward decl
class Serializable;

using namespace nlohmann;

class Syncable
{
public:
    virtual ~Syncable()= default;
    virtual void save(json& j) = 0;
    virtual void load(const json& j) = 0;
};

template <typename T>
class Sync : public Syncable
{
public:
    template <bool>
    struct Saver
    {
        Sync* mSync;
        void save(json &j){};
    };

    template<> struct Saver<true>
    {
        Sync* mSync;
        void save(json &j){mSync->mPtr->save(j[mSync->mName]);}
    };

    template<> struct Saver<false>
    {
        Sync* mSync;
        void save(json &j){j[mSync->mName] = *(mSync->mPtr);}
    };

    template <bool>
    struct Loader
    {
        Sync* mSync;
        void load(const json &j){};
    };

    template<> struct Loader<true>
    {
        Sync* mSync;
        void load(const json &j){mSync->mPtr->load(j[mSync->mName]);}
    };

    template<> struct Loader<false>
    {
        Sync* mSync;
        void load(const json &j){*(mSync->mPtr) = j[mSync->mName];}
    };

    Sync(T* ptr, const std::string& name);
    void save(json &j) override;
    void load(const json &j) override;
private:
    T* mPtr;
    std::string mName;
    Saver<std::derived_from<T, Serializable>> mSaver;
    Loader<std::derived_from<T, Serializable>> mLoader;
};

template<typename T>
Sync<T>::Sync(T *ptr, const std::string &name)
{
    mPtr = ptr;
    mName = name;
    mSaver.mSync = this;
    mLoader.mSync = this;
}

template<typename T>
void Sync<T>::save(json &j)
{
    mSaver.save(j);
}

template<typename T>
void Sync<T>::load(const json &j)
{
    mLoader.load(j);
}

class Serializable
{
public:
    virtual void save(json& j);
    virtual void load(const json& j);
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
    void save(json &j) override;
    void load(const json &j) override;
    std::vector<T>& get();
private:
    std::vector<T> mVec;
};

template<typename T>
void SerializableVector<T>::save(json &j)
{
    for(auto& elem : mVec)
    {
        json jElem;
        elem.save(jElem);
        j.push_back(std::move(jElem));
    }
}

template<typename T>
void SerializableVector<T>::load(const json &j)
{
    mVec.clear();
    for(const auto& jElem : j)
    {
        T elem;
        elem.load(jElem);
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
    void save(json &j) override;
    void load(const json &j) override;
private:
    std::map<KeyType, ValueType> mMap;
};

template <typename KeyType, typename ValueType>
void SerializableMap<KeyType, ValueType>::save(json &j)
{
    for(const auto& elem : mMap)
    {
        json jElem;
        elem.first.save(jElem["key"]);
        elem.second.save(jElem["value"]);
        j.push_back(std::move(jElem));
    }
}

template <typename KeyType, typename ValueType>
void SerializableMap<KeyType, ValueType>::load(const json &j)
{
    mMap.clear();
    for(const auto& jElem : j)
    {
        KeyType key;
        ValueType value;
        key.load(jElem["key"]);
        value.load(jElem["value"]);
        mMap.emplace(std::move(key), std::move(value));
    }
}

}
#endif
