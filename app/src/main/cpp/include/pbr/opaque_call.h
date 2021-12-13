#ifndef PBR_OPAQUE_CALL_H
#define PBR_OPAQUE_CALL_H


#include "mesh_group.h"


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
        explicit OpaqueCall ( MeshRef &mesh,
            const GXMat4 &local,
            GXAABB const &worldBounds,
            android_vulkan::ColorUnorm const &color0,
            android_vulkan::ColorUnorm const &color1,
            android_vulkan::ColorUnorm const &color2,
            android_vulkan::ColorUnorm const &color3
        ) noexcept;

        ~OpaqueCall () = default;

        // The method returns the maximum batch item count.
        // Note maxBatch will be updated only if it's less than current max unique elements of this drawcall.
        // Note maxUnique will be updated only if it's less than current max unique elements of this drawcall.
        void Append ( MeshRef &mesh,
            const GXMat4 &local,
            GXAABB const &worldBounds,
            android_vulkan::ColorUnorm const &color0,
            android_vulkan::ColorUnorm const &color1,
            android_vulkan::ColorUnorm const &color2,
            android_vulkan::ColorUnorm const &color3
        ) noexcept;

        [[nodiscard]] BatchList const& GetBatchList () const noexcept;
        [[nodiscard]] UniqueList const& GetUniqueList () const noexcept;

    private:
        void AddBatch ( MeshRef &mesh,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            android_vulkan::ColorUnorm const &color0,
            android_vulkan::ColorUnorm const &color1,
            android_vulkan::ColorUnorm const &color2,
            android_vulkan::ColorUnorm const &color3
        ) noexcept;

        void AddUnique ( MeshRef &mesh,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            android_vulkan::ColorUnorm const &color0,
            android_vulkan::ColorUnorm const &color1,
            android_vulkan::ColorUnorm const &color2,
            android_vulkan::ColorUnorm const &color3
        ) noexcept;
};

} // namespace pbr


#endif // PBR_OPAQUE_CALL_H
