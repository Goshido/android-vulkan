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
        template<typename T>
        class Factory final
        {
            public:
                Factory () = delete;

                Factory ( Factory const & ) = delete;
                Factory &operator = ( Factory const & ) = delete;

                Factory ( Factory && ) = delete;
                Factory &operator = ( Factory && ) = delete;

                explicit Factory ( std::string_view type ) noexcept
                {
                    _spawners.insert (
                        std::pair (
                            type,

                            [] ( SaveState::Container const &info ) noexcept -> Ref {
                                return std::make_unique<T> ( info );
                            }
                        )
                    );
                }

                ~Factory () = default;
        };

    private:
        using Spawner = Ref ( * ) ( SaveState::Container const &info ) noexcept;
        using Spawners = std::unordered_map<std::string_view, Spawner>;

    private:
        GXMat4                      _local = GXMat4::IDENTITY;
        GXMat4                      _parent = GXMat4::IDENTITY;
        std::string                 _name {};

        static Spawners             _spawners;

    public:
        Component () = delete;

        Component ( Component const & ) = delete;
        Component &operator = ( Component const & ) = delete;

        Component ( Component && ) = delete;
        Component &operator = ( Component && ) = delete;

        explicit Component ( std::string &&name ) noexcept;
        explicit Component ( SaveState::Container const &info ) noexcept;

        virtual ~Component () = default;

        virtual void Save ( SaveState::Container &root ) const noexcept;

        [[nodiscard]] static std::optional<Ref> Spawn ( SaveState::Container const &info ) noexcept;
};

} // namespace editor


#endif // EDITOR_COMPONENT_HPP
