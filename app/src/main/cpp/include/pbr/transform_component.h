#ifndef PBR_TRANSFORM_COMPONENT_H
#define PBR_TRANSFORM_COMPONENT_H


#include "component.h"
#include "transform_component_desc.h"


namespace pbr {

class TransformComponent final : public Component
{
    private:
        GXMat4          _local {};

        static int      _registerTransformComponentIndex;

    public:
        TransformComponent () noexcept;

        TransformComponent ( TransformComponent const & ) = delete;
        TransformComponent& operator = ( TransformComponent const & ) = delete;

        TransformComponent ( TransformComponent && ) = delete;
        TransformComponent& operator = ( TransformComponent && ) = delete;

        explicit TransformComponent ( TransformComponentDesc const &desc, uint8_t const* data ) noexcept;

        ~TransformComponent () override = default;

        [[nodiscard]] bool Register ( lua_State &vm ) noexcept;

        [[nodiscard]] static bool Init ( lua_State &vm ) noexcept;

    private:
        [[nodiscard]] ComponentRef& GetReference () noexcept override;

        [[nodiscard]] static int OnCreate ( lua_State* state );
        [[nodiscard]] static int OnGetTransform ( lua_State* state );
};

} // namespace pbr


#endif // PBR_TRANSFORM_COMPONENT_H
