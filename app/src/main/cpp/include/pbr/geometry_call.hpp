#ifndef PBR_GEOMETRY_CALL_HPP
#define PBR_GEOMETRY_CALL_HPP


#include "mesh_group.hpp"


namespace pbr {

class GeometryCall final
{
    public:
        using UniqueList = std::vector<std::pair<MeshRef, GeometryData>>;
        using BatchList = std::map<std::string_view, MeshGroup>;

    private:
        BatchList       _batch;
        UniqueList      _unique;

    public:
        GeometryCall () = delete;

        GeometryCall ( GeometryCall const & ) = delete;
        GeometryCall &operator = ( GeometryCall const & ) = delete;

        GeometryCall ( GeometryCall && ) = default;
        GeometryCall &operator = ( GeometryCall && ) = default;

        // Note maxBatch will be updated only if it's less than current max unique elements of this drawcall.
        // Note maxUnique will be updated only if it's less than current max unique elements of this drawcall.
        explicit GeometryCall ( MeshRef &mesh,
            const GXMat4 &local,
            GXAABB const &worldBounds,
            GeometryPassProgram::ColorData const &colorData
        ) noexcept;

        ~GeometryCall () = default;

        // The method returns the maximum batch item count.
        // Note maxBatch will be updated only if it's less than current max unique elements of this drawcall.
        // Note maxUnique will be updated only if it's less than current max unique elements of this drawcall.
        void Append ( MeshRef &mesh,
            const GXMat4 &local,
            GXAABB const &worldBounds,
            GeometryPassProgram::ColorData const &colorData
        ) noexcept;

        [[nodiscard]] BatchList const &GetBatchList () const noexcept;
        [[nodiscard]] UniqueList const &GetUniqueList () const noexcept;

    private:
        void AddBatch ( MeshRef &mesh,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            GeometryPassProgram::ColorData const &colorData
        ) noexcept;

        void AddUnique ( MeshRef &mesh,
            GXMat4 const &local,
            GXAABB const &worldBounds,
            GeometryPassProgram::ColorData const &colorData
        ) noexcept;
};

} // namespace pbr


#endif // PBR_GEOMETRY_CALL_HPP
