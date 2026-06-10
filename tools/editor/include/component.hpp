#ifndef EDITOR_COMPONENT_HPP
#define EDITOR_COMPONENT_HPP


#include "save_state.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <memory>
#include <optional>
#include <string>
#include <string_view>

GX_RESTORE_WARNING_STATE


namespace editor {

class Actor;

class Component;
using ComponentRef = std::unique_ptr<Component>;

class Component
{
    protected:
        constexpr static std::string_view TYPE_KEY = "type";

    private:
        using Spawner = ComponentRef ( * ) ( SaveState::Container const &info ) noexcept;
        using Spawners = std::unordered_map<std::string_view, Spawner>;

    protected:
        Actor*              _actor = nullptr;

        GXQuat              _rotation = GXQuat::IDENTITY;
        GXVec3              _scale = GXVec3::ONE;
        GXVec3              _location = GXVec3::ZERO;

        uint32_t            _version;

    private:
        std::string         _name {};
        static Spawners     _spawners;

    public:
        Component () = delete;

        Component ( Component const & ) = delete;
        Component &operator = ( Component const & ) = delete;

        Component ( Component && ) = delete;
        Component &operator = ( Component && ) = delete;

        explicit Component ( uint32_t version, std::string &&name ) noexcept;
        explicit Component ( SaveState::Container const &info ) noexcept;

        virtual ~Component () = default;

        // Method must be called in derived class
        virtual void Register ( Actor &actor ) noexcept;

        // Method must be called in derived class
        virtual void Unregister () noexcept;

        virtual void ActorTransformChanged () noexcept;
        virtual void Save ( SaveState::Container &root ) const noexcept;

        void SetName ( std::string_view name ) noexcept;

        static void InitSpawners () noexcept;

        [[nodiscard]] static std::optional<ComponentRef> Spawn ( SaveState::Container const &info ) noexcept;

    private:
        template<typename T>
        static void InitSpawner () noexcept
        {
            _spawners.insert (
                std::pair (
                    T::TYPE,

                    [] ( SaveState::Container const &info ) noexcept -> ComponentRef {
                        return std::make_unique<T> ( info );
                    }
                )
            );
        }
};

} // namespace editor


#endif // EDITOR_COMPONENT_HPP
