#ifndef PBR_STREAM_HPP
#define PBR_STREAM_HPP


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <span>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Stream final
{
    public:
        using Data = std::span<uint8_t>;

    public:
        Data        _data;
        size_t      _line;

    public:
        Stream () = default;

        Stream ( Stream const & ) = default;
        Stream &operator = ( Stream const & ) = default;

        Stream ( Stream && ) = default;
        Stream &operator = ( Stream && ) = default;

        explicit Stream ( Data data, size_t line ) noexcept;

        ~Stream () = default;

        void Init ( Data data, size_t line ) noexcept;
        [[nodiscard]] bool ExpectNotEmpty ( char const* streamSource, char const* where ) const noexcept;
};

} // namespace pbr


#endif // PBR_STREAM_HPP
