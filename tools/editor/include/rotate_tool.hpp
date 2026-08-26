#ifndef EDITOR_ROTATE_TOOL_HPP
#define EDITOR_ROTATE_TOOL_HPP


#include "gizmo_sphere_collider.hpp"
#include "gizmo_ring_collider.hpp"
#include "sdf_cone.hpp"
#include "sdf_line_segment.hpp"
#include "sdf_ring.hpp"
#include "sdf_ring_billboard.hpp"
#include "sdf_sphere.hpp"
#include "tool.hpp"


namespace editor {

class RotateTool final : public Tool
{
    private:
        enum class eAxis : uint8_t
        {
            X,
            Y,
            Z,
            ToCamera,
            None
        };

        struct TangentLine final
        {
            GXVec3                                  _tangentLocation {};
            float                                   _distance = 0.0F;
            GXVec3                                  _tangentDirection {};
        };

        struct Closest final
        {
            SDF*                                    _control = nullptr;
            float                                   _distance = std::numeric_limits<float>::max ();
        };

    private:
        constexpr static float TORUS_THICKNESS = 1.6F;
        constexpr static float RING_SIZE = 9.5F;
        constexpr static float RING_THICKNESS = 0.78F;
        constexpr static float SPHERE_SIZE = 6.6F;

    private:
        SDFRing                                     _x
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            GXQuat ( 7.071068e-1F, 0.0F, 7.071068e-1F, 0.0F ),
            GXVec3 ( 7.45F, 7.45F, 1.5e-2F ),
            eSDFPalette::Red
        };

        SDFRing                                     _y
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            GXQuat ( 7.071068e-1F, 7.071068e-1F, 0.0F, 0.0F ),
            GXVec3 ( 7.5F, 7.5F, 1.5e-2F ),
            eSDFPalette::Green
        };

        SDFRing                                     _z
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 7.55F, 7.55F, 1.5e-2F ),
            eSDFPalette::Blue
        };

        SDFRingBillboard                            _ring
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 8.3F, 8.3F, 2.0e-2F ),
            eSDFPalette::Grey
        };

        SDFSphere                                   _body
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            7.54F,
            eSDFPalette::BlackGlass
        };

        SDFLineSegment                              _tangentLine
        {
            GXVec3 ( -3.5F, 0.0F, 7.8F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 7.0F, 5.0e-2F, 5.0e-2F ),
            eSDFPalette::Yellow
        };

        SDFCone                                     _tangentDirectionA
        {
            GXVec3 ( 3.5F, 0.0F, 7.8F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 1.0F, 2.7e-1F, 2.7e-1F ),
            eSDFPalette::Yellow,
            0.0F
        };

        SDFCone                                     _tangentDirectionB
        {
            GXVec3 ( -3.5F, 0.0F, 7.8F ),
            GXQuat ( 0.0F, 0.0F, 1.0F, 0.0F ),
            GXVec3 ( 1.0F, 2.7e-1F, 2.7e-1F ),
            eSDFPalette::Yellow,
            0.0F
        };

        GizmoRingCollider                           _xCollider { _x.GetScale ()._data[ 0U ], TORUS_THICKNESS };
        GizmoRingCollider                           _yCollider { _y.GetScale ()._data[ 0U ], TORUS_THICKNESS };
        GizmoRingCollider                           _zCollider { _z.GetScale ()._data[ 0U ], TORUS_THICKNESS };
        GizmoRingCollider                           _ringCollider { RING_SIZE, RING_THICKNESS };
        GizmoSphereCollider                         _bodyCollider { SPHERE_SIZE };

        SDF*                                        _controlSDF = nullptr;

        std::unordered_map<SDF*, GXVec3>            _inactiveSize
        {
            { &_x, _x.GetScale () },
            { &_y, _y.GetScale () },
            { &_z, _z.GetScale () },
            { &_ring, _ring.GetScale () }
        };

        GXVec3                                      _rotateAxisVector {};
        float                                       _initialScalarDistance;

        GXVec3                                      _tangentLocation {};
        float                                       _rotateSpeed;

        GXQuat                                      _tangentRenderRotation {};
        GXQuat                                      _tangentDirectionBRenderRotation {};
        GXQuat                                      _initialRotation {};

        GXQuat                                      _rotation = GXQuat::IDENTITY;

        // FUCK
        GXVec3                                      _location { 0.0F, 0.0F, 12.0F };

        GXVec3                                      _tangentDirection {};
        GXVec3                                      _tangentRenderPosition {};
        GXVec3                                      _tangentDirectionARenderPosition {};

        bool                                        _rotateBall = false;
        eAxis                                       _rotateAxis = eAxis::None;

        VkOffset2D                                  _lastMouse {};
        bool                                        _lastLMBPressed = false;

    public:
        RotateTool () = default;

        RotateTool ( RotateTool const & ) = delete;
        RotateTool &operator = ( RotateTool const & ) = delete;

        RotateTool ( RotateTool && ) = delete;
        RotateTool &operator = ( RotateTool && ) = delete;

        ~RotateTool () = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Hover () noexcept override;
        void Click () noexcept override;
        void Begin () noexcept override;
        void Move () noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;

        void Update ( GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            GXVec3 const &cameraLocation,
            GXMat3 const &cameraBasis,
            GXVec3 const &vi,
            VkOffset2D const &mouse,
            bool leftMouseButtonPressed
        ) noexcept;

    private:
        void ActivateSDF ( SDF &sdf ) noexcept;
        void DeactivateSDF () noexcept;

        void HandleRingRotate ( GXVec3 const &rayOrigin, GXVec3 const &rayDirection ) noexcept;
        void HandleBallRotate ( VkOffset2D const &mouse, GXMat3 const &cameraBasis ) noexcept;

        void CheckBody ( Closest &closest,
            GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            GXVec3 const &cameraLocation,
            GXVec3 const &vi,
            VkOffset2D const &mouse,
            bool lmbPressed
        ) noexcept;

        void CheckRing ( Closest &closest,
            SDF &sdf,
            GizmoRingCollider const &collider,
            GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            GXVec3 const &cameraLocation,
            GXMat3 const &cameraBasis,
            GXVec3 const &k,
            GXVec3 const &vi,
            float s,
            bool billboard,
            bool lmbPressed,
            eAxis axis
        ) noexcept;

        // Method returns ring radius.
        [[nodiscard]] float SetupRing ( SDFRingBase &ring, eSDFPalette color ) const noexcept;

        void ResetVisuals () noexcept;

        [[nodiscard]] bool LockAxis () noexcept;
        [[nodiscard]] bool LockBall () noexcept;

        [[nodiscard]] static TangentLine ResolveTangentLine ( GXVec3 const &ringPosition,
            GXVec3 const &ringDirection,
            GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            GXVec3 const &oppositeCameraDirection,
            float radius
        ) noexcept;

        [[nodiscard]] static float ResolveSkewLines ( GXVec3 const &aPosition,
            GXVec3 const &aDirection,
            GXVec3 const &bPosition,
            GXVec3 const &bDirection
        ) noexcept;
};

} // namespace editor


#endif // EDITOR_ROTATE_TOOL_HPP
