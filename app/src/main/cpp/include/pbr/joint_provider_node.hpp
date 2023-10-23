#ifndef PBR_JOINT_PROVIDER_NODE_HPP
#define PBR_JOINT_PROVIDER_NODE_HPP


#include "node_link.hpp"
#include <joint.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <list>
#include <optional>
#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class JointProviderNode : public NodeLink
{
    public:
        using Result = std::optional<android_vulkan::Joint>;

    public:
        JointProviderNode ( JointProviderNode const& ) = default;
        JointProviderNode& operator= ( JointProviderNode const& ) = default;

        JointProviderNode ( JointProviderNode&& ) = default;
        JointProviderNode& operator= ( JointProviderNode&& ) = default;

        virtual ~JointProviderNode () = default;

        [[nodiscard]] virtual Result GetJoint ( std::string const &name ) noexcept = 0;
        virtual void Update ( float deltaTime ) noexcept = 0;

    protected:
        JointProviderNode () = default;
};

} // namespace pbr


#endif //  PBR_JOINT_PROVIDER_NODE_HPP
