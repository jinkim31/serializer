#ifndef MODELER_MODELER_H
#define MODELER_MODELER_H

#include <nlohmann/json.hpp>
#include <fstream>

namespace srz
{

using namespace nlohmann;

class Syncable
{
public:
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

    template<> struct Saver<false>
    {
        Sync* mSync;
        void save(json &j){mSync->mPtr->save(j[mSync->mName]);}
    };

    template<> struct Saver<true>
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

    template<> struct Loader<false>
    {
        Sync* mSync;
        void load(const json &j){mSync->mPtr->load(j[mSync->mName]);}
    };

    template<> struct Loader<true>
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
    Saver<std::is_fundamental<T>::value> mSaver;
    Loader<std::is_fundamental<T>::value> mLoader;
template<bool> friend struct Saver;
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
    void save(json& j);
    void load(const json& j);
protected:
    template <typename T>
    void sync(T* ptr, const std::string& name)
    {
        auto sync = std::make_shared<Sync<T>>(ptr, name);
        mSyncs.emplace_back(sync);
    }
private:
    std::vector<std::shared_ptr<Syncable>> mSyncs;
};

class SerializableArray : public Serializable
{
protected:

};

}
#endif
