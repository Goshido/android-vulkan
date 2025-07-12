#ifndef EDITOR_COMPONENT_HPP
#define EDITOR_COMPONENT_HPP


#include <GXCommon/GXMath.hpp>
#include "save_state.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <memory>
#include <optional>

GX_RESTORE_WARNING_STATE


namespace editor {

class Component
{
    public:
        using Ref = std::unique_ptr<Component>;

    protected:
        constexpr static std::string_view TYPE_KEY = "type";

    private:
        using Spawner = Ref ( * ) ( SaveState::Container const &info ) noexcept;
        using Spawners = std::unordered_map<std::string_view, Spawner>;

    protected:
        uint32_t            _version;
        GXMat4              _local = GXMat4::IDENTITY;
        GXMat4              _parent = GXMat4::IDENTITY;

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

        virtual void Save ( SaveState::Container &root ) const noexcept;

        static void InitSpawners () noexcept;
        [[nodiscard]] static std::optional<Ref> Spawn ( SaveState::Container const &info ) noexcept;

    private:
        template<typename T>
        static void InitSpawner () noexcept
        {
            _spawners.insert (
                std::pair (
                    T::TYPE,

                    [] ( SaveState::Container const &info ) noexcept -> Ref {
                        return std::make_unique<T> ( info );
                    }
                )
            );
        }
};

} // namespace editor


#endif // EDITOR_COMPONENT_HPP
