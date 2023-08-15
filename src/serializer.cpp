#include "serializer.h"

void srz::Serializable::save(nlohmann::json &j)
{
    for(const auto& sync : mSyncs)
        sync->save(j);
}

void srz::Serializable::load(const nlohmann::json &j)
{
    for(const auto& sync : mSyncs)
        sync->load(j);
}
