#ifndef PBR_SCENE_H
#define PBR_SCENE_H


#include "actor.h"
#include "render_session.h"

GX_DISABLE_COMMON_WARNINGS

#include <list>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Scene final
{
    public:
        using Actors = std::vector<ActorRef>;
        using FindResult = std::optional<std::reference_wrapper<Actors>>;

    private:
        std::unordered_map<std::string, Actors>     _actorStorage {};
        ComponentList                               _freeTransferResourceList {};
        ComponentList                               _renderableList {};

    public:
        Scene () = default;

        Scene ( Scene const & ) = delete;
        Scene& operator = ( Scene const & ) = delete;

        Scene ( Scene && ) = delete;
        Scene& operator = ( Scene && ) = delete;

        ~Scene () = default;

        void OnDestroyDevice () noexcept;

        void AppendActor ( ActorRef &actor ) noexcept;
        [[nodiscard, maybe_unused]] FindResult FindActors ( std::string const &actorName ) noexcept;
        void FreeTransferResources ( VkDevice device ) noexcept;
        void Submit ( RenderSession &renderSession ) noexcept;
};

} // namespace pbr


#endif // PBR_SCENE_H
