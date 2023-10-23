#ifndef PBR_COMPONENT_HPP
#define PBR_COMPONENT_HPP


#include "types.hpp"
#include <android_vulkan_sdk/pbr/component_desc.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lstate.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

class Component
{
    public:
        // Forward declaration. This will init '_handlers' static field.
        class StaticInitializer;

    private:
        using Handler = ComponentRef ( * ) ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

    protected:
        std::string         _name;

    private:
        ClassID             _classID;
        static Handler      _handlers[ static_cast<size_t> ( ClassID::COUNT ) ];

    public:
        Component () = delete;

        Component ( Component const & ) = delete;
        Component &operator = ( Component const & ) = delete;

        Component ( Component && ) = delete;
        Component &operator = ( Component && ) = delete;

        virtual ~Component () = default;

        [[nodiscard]] virtual ComponentRef &GetReference () noexcept = 0;

        [[nodiscard]] ClassID GetClassID () const noexcept;
        [[nodiscard]] std::string const &GetName () const noexcept;

        [[nodiscard]] static ComponentRef Create ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        static void Register ( lua_State &vm ) noexcept;

    protected:
        explicit Component ( ClassID classID ) noexcept;
        explicit Component ( ClassID classID, std::string &&name ) noexcept;

    private:
        [[nodiscard]] static ComponentRef CreateCamera ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static ComponentRef CreateStaticMesh ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static ComponentRef CreatePointLight ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static ComponentRef CreateReflection ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static ComponentRef CreateRigidBody ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static ComponentRef CreateScript ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static ComponentRef CreateSoundEmitterGlobal ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static ComponentRef CreateSoundEmitterSpatial ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static ComponentRef CreateTransform ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static ComponentRef CreateUnknown ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            size_t &dataRead,
            ComponentDesc const &desc,
            uint8_t const* data,
            VkCommandBuffer const* commandBuffers
        ) noexcept;

        [[nodiscard]] static int OnGetName ( lua_State* state );
};

} // namespace pbr


#endif // PBR_COMPONENT_HPP
