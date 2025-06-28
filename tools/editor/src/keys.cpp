#include <precompiled_headers.hpp>
#include <keys.hpp>


namespace editor {

bool KeyModifier::AnyAltPressed () const noexcept
{
    return _leftAlt | _rightAlt;
}

bool KeyModifier::AnyCtrlPressed () const noexcept
{
    return _leftCtrl | _rightCtrl;
}

bool KeyModifier::AnyShiftPressed () const noexcept
{
    return _leftShift | _rightShift;
}

} // namespace editor
