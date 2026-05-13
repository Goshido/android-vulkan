#ifndef EDITOR_COMPONENT_HPP
#define EDITOR_COMPONENT_HPP


#include "message_queue.hpp"
#include "save_state.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <memory>
#include <optional>
#include <string>
#include <string_view>

GX_RESTORE_WARNING_STATE


namespace editor {

class Component;
using ComponentRef = std::unique_ptr<Component>;

class Component
{
    protected:
        constexpr static std::string_view TYPE_KEY = "type";

    private:
        using Spawner = ComponentRef ( * ) ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept;
        using Spawners = std::unordered_map<std::string_view, Spawner>;

    protected:
        MessageQueue        &_messageQueue;

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

        explicit Component ( MessageQueue &messageQueue, uint32_t version, std::string &&name ) noexcept;
        explicit Component ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept;

        virtual ~Component () = default;

        virtual void Register () noexcept = 0;
        virtual void Unregister () noexcept = 0;

        virtual void Save ( SaveState::Container &root ) const noexcept;

        void SetName ( std::string_view name ) noexcept;

        static void InitSpawners () noexcept;

        [[nodiscard]] static std::optional<ComponentRef> Spawn ( MessageQueue &messageQueue,
            SaveState::Container const &info
        ) noexcept;

    private:
        template<typename T>
        static void InitSpawner () noexcept
        {
            _spawners.insert (
                std::pair (
                    T::TYPE,

                    [] ( MessageQueue &messageQueue, SaveState::Container const &info ) noexcept -> ComponentRef {
                        return std::make_unique<T> ( messageQueue, info );
                    }
                )
            );
        }
};

} // namespace editor


#endif // EDITOR_COMPONENT_HPP
