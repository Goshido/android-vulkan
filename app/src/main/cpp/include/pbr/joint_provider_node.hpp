#ifndef PBR_JOINT_PROVIDER_NODE_HPP
#define PBR_JOINT_PROVIDER_NODE_HPP


#include <joint.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class JointProviderNode
{
    public:
        JointProviderNode ( JointProviderNode const& ) = default;
        JointProviderNode& operator= ( JointProviderNode const& ) = default;

        JointProviderNode ( JointProviderNode&& ) = default;
        JointProviderNode& operator= ( JointProviderNode&& ) = default;

        virtual ~JointProviderNode () = default;

        [[nodiscard]] virtual android_vulkan::Joint GetJoint ( std::string const &name, uint32_t frame ) noexcept = 0;
        virtual void Update ( float deltaTime ) noexcept = 0;

    protected:
        JointProviderNode () = default;
};

} // namespace pbr


#endif //  PBR_JOINT_PROVIDER_NODE_HPP
