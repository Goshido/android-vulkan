#ifndef AVP_CLASS_DESC_HPP
#define AVP_CLASS_DESC_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <3ds_max_sdk/iparamb2.h>

GX_RESTORE_WARNING_STATE


namespace avp {

class ClassDesc : public ClassDesc2
{
    private:
        [[maybe_unused]] HINSTANCE      _instance = nullptr;

    public:
        ClassDesc () = default;

        ClassDesc ( ClassDesc const & ) = delete;
        ClassDesc &operator = ( ClassDesc const & ) = delete;

        ClassDesc ( ClassDesc && ) = delete;
        ClassDesc &operator = ( ClassDesc && ) = delete;

        ~ClassDesc () = default;

        void Init ( HINSTANCE instance ) noexcept;

    private:
        [[nodiscard]] int IsPublic () override;
        [[nodiscard]] void* Create ( BOOL loading ) override;
        [[nodiscard]] MCHAR const* ClassName () override;
        [[nodiscard]] MCHAR const* NonLocalizedClassName () override;
        [[nodiscard]] MCHAR const* InternalName () override;
        [[nodiscard]] SClass_ID SuperClassID () override;
        [[nodiscard]] Class_ID ClassID () override;
        [[nodiscard]] MCHAR const* Category () override;
        [[nodiscard]] HINSTANCE HInstance () override;
};

} // namespace avp


#endif // AVP_CLASS_DESC_HPP
