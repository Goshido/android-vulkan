#ifndef PBR_DEFAULT_TEXTURE_MANAGER_H
#define PBR_DEFAULT_TEXTURE_MANAGER_H


#include "types.h"


namespace pbr {

class DefaultTextureManager final
{
    private:
        constexpr static size_t BUFFER_COUNT = 5U;

    private:
        Texture2DRef        _albedo {};
        Texture2DRef        _emission {};
        Texture2DRef        _mask {};
        Texture2DRef        _normal {};
        Texture2DRef        _params {};

        VkCommandBuffer     _commandBuffers[ BUFFER_COUNT ];

    public:
        DefaultTextureManager () = default;

        DefaultTextureManager ( DefaultTextureManager const & ) = delete;
        DefaultTextureManager& operator = ( DefaultTextureManager const & ) = delete;

        DefaultTextureManager ( DefaultTextureManager && ) = delete;
        DefaultTextureManager& operator = ( DefaultTextureManager && ) = delete;

        ~DefaultTextureManager () = default;

        void FreeTransferResources ( VkDevice device, VkCommandPool commandPool ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        [[nodiscard]] Texture2DRef const& GetAlbedo () const noexcept;
        [[nodiscard]] Texture2DRef const& GetEmission () const noexcept;
        [[nodiscard]] Texture2DRef const& GetMask () const noexcept;
        [[nodiscard]] Texture2DRef const& GetNormal () const noexcept;
        [[nodiscard]] Texture2DRef const& GetParams () const noexcept;
};

} // namespace pbr


#endif // PBR_DEFAULT_TEXTURE_MANAGER_H
