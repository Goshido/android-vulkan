#ifndef EDITOR_REFLECTION_PROBE_GLOBAL_NODE_HPP
#define EDITOR_REFLECTION_PROBE_GLOBAL_NODE_HPP


#include "reflection_probe_global_info.hpp"
#include "workspace_node.hpp"


namespace editor {

class Workspace;

class ReflectionProbeGlobalNode final : public WorkspaceNode
{
    private:
        ReflectionProbeGlobalInfo*      _internal = nullptr;
        ReflectionProbeGlobalInfo       _info {};

    public:
        ReflectionProbeGlobalNode () = default;

        ReflectionProbeGlobalNode ( ReflectionProbeGlobalNode const & ) = delete;
        ReflectionProbeGlobalNode &operator = ( ReflectionProbeGlobalNode const & ) = delete;

        ReflectionProbeGlobalNode ( ReflectionProbeGlobalNode &&other ) noexcept;
        ReflectionProbeGlobalNode &operator = ( ReflectionProbeGlobalNode &&other ) noexcept;

        explicit ReflectionProbeGlobalNode ( Workspace &workspace, ReflectionProbeGlobalInfo &internal ) noexcept;

        ~ReflectionProbeGlobalNode () noexcept override;

        void Commit () noexcept;
        [[nodiscard]] ReflectionProbeGlobalInfo &GetInternalInfo () noexcept;

        void SetIntensity ( float intensity ) noexcept;
};

} // namespace editor


#endif // EDITOR_REFLECTION_PROBE_GLOBAL_NODE_HPP
