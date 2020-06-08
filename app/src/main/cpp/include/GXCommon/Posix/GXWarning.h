// version 1.0
// No include guard allowed
// Not include this explicitly! Use GXCommon/GXWarnings.h instead.


#define GX_SAVE_WARNING_STATE _Pragma ( "clang diagnostic push" )
#define GX_RESTORE_WARNING_STATE _Pragma ( "clang diagnostic pop" )

#define GX_WARNING_HELPER0(x) #x
#define GX_WARNING_HELPER1(x) GX_WARNING_HELPER0 ( clang diagnostic ignored x )
#define GX_WARNING_HELPER2(x) GX_WARNING_HELPER1 ( #x )
#define GX_DISABLE_WARNING(x) _Pragma ( GX_WARNING_HELPER2 ( x ) )

#define GX_DISABLE_COMMON_WARNINGS \
    GX_SAVE_WARNING_STATE \
    GX_DISABLE_WARNING ( -Wshadow )
