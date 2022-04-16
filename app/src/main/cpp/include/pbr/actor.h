#ifndef PBR_ACTOR_H
#define PBR_ACTOR_H


#include "script_engine.h"
#include "types.h"
#include <physics.h>

GX_DISABLE_COMMON_WARNINGS

#include <functional>
#include <unordered_map>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Actor final
{
    public:
        using Components = std::vector<ComponentRef>;
        using FindResult = std::optional<std::reference_wrapper<Components>>;

    private:
        std::unordered_map<std::string, Components>     _componentStorage {};
        std::string const                               _name;

    public:
        Actor () noexcept;

        Actor ( Actor const & ) = delete;
        Actor& operator = ( Actor const & ) = delete;

        Actor ( Actor && ) = delete;
        Actor& operator = ( Actor && ) = delete;

        explicit Actor ( std::string &&name ) noexcept;

        ~Actor () = default;

        void AppendComponent ( ComponentRef &component ) noexcept;
        [[nodiscard, maybe_unused]] FindResult FindComponents ( std::string const &componentName ) noexcept;
        [[nodiscard]] std::string const& GetName () const noexcept;

        void RegisterComponents ( ComponentList &freeTransferResource,
            ComponentList &renderable,
            android_vulkan::Physics &physics,
            ScriptEngine &scriptEngine
        ) noexcept;
};

} // namespace pbr


#endif // PBR_ACTOR_H
