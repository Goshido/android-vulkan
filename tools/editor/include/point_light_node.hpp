#ifndef EDITOR_POINT_LIGHT_NODE_HPP
#define EDITOR_POINT_LIGHT_NODE_HPP


#include "point_light_info.hpp"
#include "workspace_node.hpp"


namespace editor {

class Workspace;

class PointLightNode final : public WorkspaceNode
{
    private:
        PointLightInfo*     _internal = nullptr;
        PointLightInfo      _info {};

    public:
        PointLightNode () = default;

        PointLightNode ( PointLightNode const & ) = delete;
        PointLightNode &operator = ( PointLightNode const & ) = delete;

        PointLightNode ( PointLightNode &&other ) noexcept;
        PointLightNode &operator = ( PointLightNode &&other ) noexcept;

        explicit PointLightNode ( Workspace &workspace, PointLightInfo &internal ) noexcept;

        ~PointLightNode () noexcept override;

        void Commit () noexcept;
        [[nodiscard]] PointLightInfo &GetInternalInfo () noexcept;

        void SetColor ( GXColorUNORM color ) noexcept;
        void SetIntensity ( float intensity ) noexcept;
        void SetLocation ( GXVec3 const &location ) noexcept;
        void SetRadius ( float radius ) noexcept;
};

} // namespace editor


#endif // EDITOR_POINT_LIGHT_NODE_HPP
