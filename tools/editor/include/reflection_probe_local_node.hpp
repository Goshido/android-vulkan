#ifndef EDITOR_REFLECTION_PROBE_LOCAL_NODE_HPP
#define EDITOR_REFLECTION_PROBE_LOCAL_NODE_HPP


#include "reflection_probe_local_info.hpp"
#include "workspace_node.hpp"


namespace editor {

class Workspace;

class ReflectionProbeLocalNode final : public WorkspaceNode
{
    private:
        ReflectionProbeLocalInfo*       _internal = nullptr;
        ReflectionProbeLocalInfo        _info {};

    public:
        ReflectionProbeLocalNode () = default;

        ReflectionProbeLocalNode ( ReflectionProbeLocalNode const & ) = delete;
        ReflectionProbeLocalNode &operator = ( ReflectionProbeLocalNode const & ) = delete;

        ReflectionProbeLocalNode ( ReflectionProbeLocalNode &&other ) noexcept;
        ReflectionProbeLocalNode &operator = ( ReflectionProbeLocalNode &&other ) noexcept;

        explicit ReflectionProbeLocalNode ( Workspace &workspace, ReflectionProbeLocalInfo &internal ) noexcept;

        ~ReflectionProbeLocalNode () noexcept override;

        void Commit () noexcept;
        [[nodiscard]] ReflectionProbeLocalInfo &GetInternalInfo () noexcept;

        void SetIntensity ( float intensity ) noexcept;
        void SetRadius ( float radius ) noexcept;
};

} // namespace editor


#endif // EDITOR_REFLECTION_PROBE_LOCAL_NODE_HPP
