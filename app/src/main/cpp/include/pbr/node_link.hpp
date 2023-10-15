#ifndef PBR_NODE_LINK_HPP
#define PBR_NODE_LINK_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <deque>

GX_RESTORE_WARNING_STATE


namespace pbr {

struct NodeLink
{
    private:
        std::deque<NodeLink*>       _nodes {};

    public:
        NodeLink () = default;

        NodeLink ( NodeLink const& ) = default;
        NodeLink& operator= ( NodeLink const& ) = default;

        NodeLink ( NodeLink&& ) = default;
        NodeLink& operator= ( NodeLink&& ) = default;

        virtual ~NodeLink () = default;

    public:
        void RegisterNode ( NodeLink* node ) noexcept;
        void UnregisterNode ( NodeLink* node ) noexcept;

    protected:
        void UnregisterSelf () noexcept;
};

} // namespace pbr


#endif // PBR_NODE_LINK_HPP
