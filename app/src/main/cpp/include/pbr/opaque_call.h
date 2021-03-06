#ifndef OPAQUE_CALL_H
#define OPAQUE_CALL_H


#include "mesh_group.h"
#include <half_types.h>


namespace pbr {

class OpaqueCall final
{
    public:
        using UniqueList = std::vector<std::pair<MeshRef, OpaqueData>>;
        using BatchList = std::map<std::string_view, MeshGroup>;

    private:
        BatchList       _batch;
        UniqueList      _unique;

    public:
        OpaqueCall () = delete;

        OpaqueCall ( OpaqueCall const & ) = delete;
        OpaqueCall& operator = ( OpaqueCall const & ) = delete;

        OpaqueCall ( OpaqueCall && ) = default;
        OpaqueCall& operator = ( OpaqueCall && ) = default;

        // Note maxBatch will be updated only if it's less than current max unique elements of this drawcall.
        // Note maxUnique will be updated only if it's less than current max unique elements of this drawcall.
        explicit OpaqueCall ( size_t &maxBatch,
            size_t &maxUnique,
            MeshRef &mesh,
            const GXMat4 &local,
            GXAABB const &worldBounds,
            android_vulkan::Half4 const &color0,
            android_vulkan::Half4 const &color1,
            android_vulkan::Half4 const &color2,
            android_vulkan::Half4 const &color3
        );

        ~OpaqueCall () = default;

        // The method returns the maximum batch item count.
        // Note maxBatch will be updated only if it's less than current max unique elements of this drawcall.
        // Note maxUnique will be updated only if it's less than current max unique elements of this drawcall.
        void Append ( size_t &maxBatch,
            size_t &maxUnique,
            MeshRef &mesh,
            const GXMat4 &local,
            GXAABB const &worldBounds,
            android_vulkan::Half4 const &color0,
            android_vulkan::Half4 const &color1,
            android_vulkan::Half4 const &color2,
            android_vulkan::Half4 const &color3
        );

        [[nodiscard]] BatchList const& GetBatchList () const;
        [[nodiscard]] UniqueList const& GetUniqueList () const;

    private:
        void AddBatch ( size_t &maxBatch,
            MeshRef &mesh,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            android_vulkan::Half4 const &color0,
            android_vulkan::Half4 const &color1,
            android_vulkan::Half4 const &color2,
            android_vulkan::Half4 const &color3
        );

        void AddUnique ( size_t &maxUnique,
            MeshRef &mesh,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            android_vulkan::Half4 const &color0,
            android_vulkan::Half4 const &color1,
            android_vulkan::Half4 const &color2,
            android_vulkan::Half4 const &color3
        );
};

} // namespace pbr


#endif // OPAQUE_CALL_H
