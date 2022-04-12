#include <pbr/scene.h>
#include <pbr/renderable_component.h>


namespace pbr {

bool Scene::OnInitDevice () noexcept
{
    _scriptEngine = &ScriptEngine::GetInstance ();
    return _scriptEngine->Init ();
}

void Scene::OnDestroyDevice () noexcept
{
    _freeTransferResourceList.clear ();
    _renderableList.clear ();
    _actorStorage.clear ();

    ScriptEngine::Destroy ();
    _scriptEngine = nullptr;
}

void Scene::AppendActor ( ActorRef &actor ) noexcept
{
    Actors& actors = _actorStorage[ actor->GetName () ];
    actors.push_back ( actor );
    actors.back ()->RegisterComponents ( _freeTransferResourceList, _renderableList, *_scriptEngine );
}

[[maybe_unused]] Scene::FindResult Scene::FindActors ( std::string const &actorName ) noexcept
{
    auto findResult = _actorStorage.find ( actorName );

    return findResult == _actorStorage.end () ?
        std::nullopt :
        std::optional<std::reference_wrapper<Actors>> { findResult->second };
}

void Scene::FreeTransferResources ( VkDevice device ) noexcept
{
    for ( auto& component : _freeTransferResourceList )
    {
        // NOLINTNEXTLINE - downcast.
        auto& renderableComponent = static_cast<RenderableComponent&> ( *component.get () );
        renderableComponent.FreeTransferResources ( device );
    }

    _renderableList.splice ( _renderableList.cend (), _freeTransferResourceList );
}

void Scene::Submit ( RenderSession &renderSession ) noexcept
{
    for ( auto& component : _renderableList )
    {
        // NOLINTNEXTLINE - downcast.
        auto& renderableComponent = static_cast<RenderableComponent&> ( *component.get () );
        renderableComponent.Submit ( renderSession );
    }
}

} // namespace pbr
