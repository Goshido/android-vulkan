#ifndef PBR_MARIO_CAMERA_H
#define PBR_MARIO_CAMERA_H


#include "target.h"

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan.h>

GX_RESTORE_WARNING_STATE


namespace pbr::mario {

class Camera final
{
    private:
        GXMat4              _local;
        GXMat4              _projection;
        ITarget const*      _target;

    public:
        Camera () noexcept;

        Camera ( Camera const & ) = delete;
        Camera& operator = ( Camera const & ) = delete;

        Camera ( Camera && ) = delete;
        Camera& operator = ( Camera && ) = delete;

        ~Camera () = default;

        void Focus () noexcept;

        [[nodiscard]] GXMat4 const& GetLocalMatrix () const noexcept;

        [[nodiscard]] GXMat4 const& GetProjectionMatrix () const noexcept;
        void OnUpdate ( float deltaTime ) noexcept;
        void OnResolutionChanged ( VkExtent2D const &resolution ) noexcept;
        void SetTarget ( ITarget const &target ) noexcept;
};

} // namespace pbr::mario


#endif // PBR_MARIO_CAMERA_H
