#ifndef EDITOR_CROSS_LIKE_TOOL_HPP
#define EDITOR_CROSS_LIKE_TOOL_HPP


#include "sdf_box_with_flip.hpp"
#include "sdf_line_segment_with_flip.hpp"
#include "tool.hpp"


namespace editor {

class CrossLikeTool : public Tool
{
    protected:
        enum class eAxis : uint8_t
        {
            X,
            Y,
            Z,
            None
        };

        struct ColorSet final
        {
            eSDFPalette                 _active = eSDFPalette::White;
            eSDFPalette                 _standby = eSDFPalette::White;
        };

        struct Closest final
        {
            SDF*                        _control = nullptr;
            SDF*                        _cap = nullptr;
            float                       _distance = std::numeric_limits<float>::max ();
        };

    protected:
        constexpr static GXVec3         PLANE_SCALE { 0.2F, 2.4F, 2.4F };
        constexpr static GXVec2         PLANE_OFFSET { -0.2F, 1.1F };
        constexpr static float          PLANE_FLIP_OFFSET = 2.15F;

        constexpr static eSDFPalette    X_COLOR = eSDFPalette::Red;
        constexpr static eSDFPalette    X_PLANE_ACTIVE_COLOR = eSDFPalette::RedGlass;
        constexpr static eSDFPalette    X_PLANE_STANDBY_COLOR = eSDFPalette::RedGhost;

        constexpr static eSDFPalette    Y_COLOR = eSDFPalette::Green;
        constexpr static eSDFPalette    Y_PLANE_ACTIVE_COLOR = eSDFPalette::GreenGlass;
        constexpr static eSDFPalette    Y_PLANE_STANDBY_COLOR = eSDFPalette::GreenGhost;

        constexpr static eSDFPalette    Z_COLOR = eSDFPalette::Blue;
        constexpr static eSDFPalette    Z_PLANE_ACTIVE_COLOR = eSDFPalette::BlueGlass;
        constexpr static eSDFPalette    Z_PLANE_STANDBY_COLOR = eSDFPalette::BlueGhost;

        constexpr static eSDFPalette    WORK_OPAQUE_COLOR = eSDFPalette::Yellow;
        constexpr static eSDFPalette    WORK_TRANSPARENT_COLOR = eSDFPalette::YellowGlass;
        constexpr static eSDFPalette    WORK_INACTIVE_COLOR = eSDFPalette::Grey;


        SDF*                            _control = nullptr;
        SDF*                            _cap = nullptr;

        GXQuat                          _rotation = GXQuat::IDENTITY;
        float                           _initialNegativeScalarDistance = 0.0F;

        // FUCK
        GXVec3                          _location { -1.2F, -1.0F, 3.0F };
        eAxis                           _workAxis = eAxis::None;

        GXVec3                          _initialState {};
        eAxis                           _workPlane = eAxis::None;

        GXVec3                          _workDirection {};
        bool                            _lastLMBPressed = false;

    public:
        CrossLikeTool ( CrossLikeTool const & ) = delete;
        CrossLikeTool &operator = ( CrossLikeTool const & ) = delete;

        CrossLikeTool ( CrossLikeTool && ) = delete;
        CrossLikeTool &operator = ( CrossLikeTool && ) = delete;

    protected:
        explicit CrossLikeTool () = default;
        ~CrossLikeTool () = default;

        [[nodiscard]] static bool FlipTest ( SDFBoxWithFlip const &plane, GXVec3 const &cameraLocation ) noexcept;

        static void HidePlane ( SDFBoxWithFlip &plane,
            SDFLineSegmentWithFlip &lineA,
            SDFLineSegmentWithFlip &lineB
        ) noexcept;

        [[nodiscard]] static std::optional<float> ResolveAxisScalarDistance ( GXVec3 const &axisOrigin,
            GXVec3 const &axisDirection,
            GXVec3 const &cameraLocation,
            GXVec3 const &cameraForward
        ) noexcept;

        [[nodiscard]] static std::optional<GXVec3> ResolvePlaneIntersection ( GXVec3 const &planeOrigin,
            GXVec3 const &planeNormal,
            GXVec3 const &cameraLocation,
            GXVec3 const &cameraForward
        ) noexcept;
};

} // namespace editor


#endif // EDITOR_CROSS_LIKE_TOOL_HPP
