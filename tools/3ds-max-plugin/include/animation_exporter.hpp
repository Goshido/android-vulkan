#ifndef AVP_ANIMATION_EXPORTER_HPP
#define AVP_ANIMATION_EXPORTER_HPP


#include "bone_data_exporter.hpp"


namespace avp {

class AnimationExporter final : public BoneDataExporter
{
    public:
        AnimationExporter () = delete;

        AnimationExporter ( AnimationExporter const & ) = delete;
        AnimationExporter &operator = ( AnimationExporter const & ) = delete;

        AnimationExporter ( AnimationExporter && ) = delete;
        AnimationExporter &operator = ( AnimationExporter && ) = delete;

        ~AnimationExporter () = delete;

        static void Run ( HWND parent, MSTR const &path, int startFrame, int lastFrame ) noexcept;
};

} // namespace avp


#endif // AVP_ANIMATION_EXPORTER_HPP
