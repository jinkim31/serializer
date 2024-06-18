#include "sync.h"

void jsync::Serializable::save(nlohmann::json &j)
{
    mSyncs.clear();
    listSync();
    for(const auto& sync : mSyncs)
    {
        sync->save(j[sync->name()]);
    }
    onSyncSave();
}

void jsync::Serializable::load(const nlohmann::json &j)
{
    mSyncs.clear();
    listSync();
    for(const auto& sync : mSyncs)
    {
        sync->load(j[sync->name()]);
    }
    onSyncLoad();
}
