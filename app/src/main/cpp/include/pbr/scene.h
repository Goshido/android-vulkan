#ifndef PBR_SCENE_H
#define PBR_SCENE_H


#include "actor.h"


namespace pbr {

class [[maybe_unused]] Scene final
{
    public:
        using Actors = std::vector<ActorRef>;
        using FindResult = std::optional<std::reference_wrapper<Actors>>;

    private:
        std::unordered_map<std::string, Actors>     _actorStorage {};

    public:
        Scene () = default;

        Scene ( Scene const & ) = delete;
        Scene& operator = ( Scene const & ) = delete;

        Scene ( Scene && ) = delete;
        Scene& operator = ( Scene && ) = delete;

        ~Scene () = default;

        [[maybe_unused]] void AppendActor ( ActorRef &actor ) noexcept;
        [[nodiscard, maybe_unused]] FindResult FindActors ( std::string const &actorName ) noexcept;
};

} // namespace pbr


#endif // PBR_SCENE_H
