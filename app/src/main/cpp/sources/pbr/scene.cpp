#include <pbr/scene.h>


namespace pbr {

[[maybe_unused]] void Scene::AppendActor ( ActorRef &actor ) noexcept
{
    _actorStorage[ actor->GetName () ].push_back ( actor );
}

[[maybe_unused]] Scene::FindResult Scene::FindActors ( std::string const &actorName ) noexcept
{
    auto findResult = _actorStorage.find ( actorName );

    return findResult == _actorStorage.end () ?
        std::nullopt :
        std::optional<std::reference_wrapper<Actors>> { findResult->second };
}

} // namespace pbr
