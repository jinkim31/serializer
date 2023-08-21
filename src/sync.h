#ifndef SYNC_SYNC_H
#define SYNC_SYNC_H

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
    Syncable(const std::string& name){mName = name;}
    virtual ~Syncable()= default;
    virtual void save(json& j) = 0;
    virtual void load(const json& j) = 0;
    std::string& name(){return mName;}
private:
    std::string mName;
};

template <typename T, bool>
struct Saver{};

template<typename T> struct Saver<T, true>
{
    T* mPtr;
    void save(json &j) {mPtr->save(j);}
};

template<typename T> struct Saver<T, false>
{
    T* mPtr;
    void save(json &j) {j = *(mPtr);}
};

template <typename T, bool>
struct Loader{};

template<typename T> struct Loader<T, true>
{
    T* mPtr;
    void load(const json &j){mPtr->load(j);}
};

template<typename T> struct Loader<T, false>
{
    T* mPtr;
    void load(const json &j){*(mPtr) = j;}
};

template <typename T>
class Sync : public Syncable
{
public:
    Sync(T* ptr, const std::string& name);
    void save(json &j) override;
    void load(const json &j) override;
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
    Loader<T, std::derived_from<T, Serializable>> mLoader;
};

template<typename T>
void SerializableVector<T>::save(json &j)
{
    for(auto& elem : mVec)
    {
        json jElem;
        Saver<T, std::derived_from<T, Serializable>> mSaver{&elem};
        mSaver.save(jElem);
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
    void save(json &j) override;
    void load(const json &j) override;
    std::map<KeyType, ValueType>& get();
private:
    std::map<KeyType, ValueType> mMap;
    Loader<KeyType, std::derived_from<KeyType, Serializable>> mKeyLoader;
    Loader<ValueType, std::derived_from<ValueType, Serializable>> mValueLoader;
};

template <typename KeyType, typename ValueType>
void SerializableMap<KeyType, ValueType>::save(json &j)
{
    for(auto& elem : mMap)
    {
        json jElem;
        Saver<KeyType, std::derived_from<KeyType, Serializable>> mKeySaver{&elem.first};
        Saver<ValueType, std::derived_from<ValueType, Serializable>> mValueSaver{&elem.second};
        mKeySaver.save(jElem["key"]);
        mValueSaver.save(jElem["Value"]);
        j.push_back(std::move(jElem));
    }
}

template <typename KeyType, typename ValueType>
void SerializableMap<KeyType, ValueType>::load(const json &j)
{
    mMap.clear();
    for(const auto& jElem : j)
    {
        std::cout<<"loading"<<j<<std::endl;
        KeyType key;
        ValueType value;
        mKeyLoader.mPtr = &key;
        mValueLoader.mPtr = &value;
        mKeyLoader.load(jElem["key"]);
        mValueLoader.load(jElem["Value"]);
        mMap.emplace(std::move(key), std::move(value));
    }
}

template <typename KeyType, typename ValueType>
std::map<KeyType, ValueType>& SerializableMap<KeyType, ValueType>::get()
{
    return mMap;
}

}
#endif
