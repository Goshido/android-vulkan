#ifndef AVP_SKIN_EXPORTER_HPP
#define AVP_SKIN_EXPORTER_HPP


#include "exporter.hpp"


namespace avp {

class SkinExporter final : public Exporter
{
    public:
        SkinExporter () = delete;

        SkinExporter ( SkinExporter const & ) = delete;
        SkinExporter &operator = ( SkinExporter const & ) = delete;

        SkinExporter ( SkinExporter && ) = delete;
        SkinExporter &operator = ( SkinExporter && ) = delete;

        ~SkinExporter () = delete;

        static void Run ( HWND parent, MSTR const &path ) noexcept;
};

} // namespace avp


#endif // AVP_SKIN_EXPORTER_HPP
