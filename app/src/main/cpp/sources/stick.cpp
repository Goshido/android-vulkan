#include <stick.h>

GX_DISABLE_COMMON_WARNINGS

#include <cstring>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

Stick::Stick ():
    _context ( nullptr ),
    _handler ( nullptr ),
    _stateCurrent ( 0.0F, 0.0F ),
    _stateOld ( 0.0F, 0.0F )
{
    // NOTHING
}

void Stick::Bind ( void* context, StickHandler handler )
{
    _stateCurrent = GXVec2 ( 0.0F, 0.0F );
    _stateOld = GXVec2 ( 0.0F, 0.0F );
    _context = context;
    _handler = handler;
}

void Stick::Unbind ()
{
    _stateOld = _stateCurrent;
    _context = nullptr;
    _handler = nullptr;
}

void Stick::Execute ()
{
    if ( !_handler || memcmp ( &_stateCurrent, &_stateOld, sizeof ( _stateCurrent ) ) == 0 )
        return;

    _handler ( _context, _stateCurrent._data[ 0U ], _stateCurrent._data[ 1U ] );
    _stateOld = _stateCurrent;
}

void Stick::Update ( AInputEvent* event, int32_t horizontalAxis, int32_t verticalAxis )
{
    _stateCurrent._data[ 0U ] = AMotionEvent_getAxisValue ( event, horizontalAxis, 0U );
    _stateCurrent._data[ 1U ] = -AMotionEvent_getAxisValue ( event, verticalAxis, 0U );
}

} // namespace android_vulkan
