#ifndef PBR_EXPOSURE_SPECIALIZATION_HPP
#define PBR_EXPOSURE_SPECIALIZATION_HPP


namespace pbr {

class ExposureSpecialization final
{
    public:
        uint32_t        _workgroupCount = 0U;

        VkExtent2D      _mip5Resolution
        {
            .width = 0U,
            .height = 0U
        };

        float           _normalizeW = 1.0F;
        float           _normalizeH = 1.0F;

    public:
        ExposureSpecialization () = delete;

        ExposureSpecialization ( ExposureSpecialization const & ) = delete;
        ExposureSpecialization &operator = ( ExposureSpecialization const & ) = delete;

        ExposureSpecialization ( ExposureSpecialization && ) = delete;
        ExposureSpecialization &operator = ( ExposureSpecialization && ) = delete;

        explicit ExposureSpecialization ( VkExtent3D &dispatch,
            VkExtent2D &mipChainResolution,
            VkExtent2D const &imageResolution
        ) noexcept;

        ~ExposureSpecialization () = default;
};

} // namespace pbr


#endif // PBR_EXPOSURE_SPECIALIZATION_HPP
