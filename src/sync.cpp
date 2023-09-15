#include "sync.h"

void sync::Serializable::save(nlohmann::json &j)
{
    mSyncs.clear();
    listSync();
    for(const auto& sync : mSyncs)
    {
        sync->save(j[sync->name()]);
    }
    onSyncSave();
}

void sync::Serializable::load(const nlohmann::json &j)
{
    mSyncs.clear();
    listSync();
    for(const auto& sync : mSyncs)
    {
        sync->load(j[sync->name()]);
    }
    onSyncLoad();
    syncInit();
}
