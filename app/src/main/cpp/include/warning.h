#ifndef ANDROID_VULKAN_WARNING_H
#define ANDROID_VULKAN_WARNING_H


#define AV_SAVE_WARNING_STATE _Pragma ( "clang diagnostic push" )
#define AV_RESTORE_WARNING_STATE _Pragma ( "clang diagnostic pop" )

#define AV_WARNING_HELPER0(x) #x
#define AV_WARNING_HELPER1(x) AV_WARNING_HELPER0 ( clang diagnostic ignored x )
#define AV_WARNING_HELPER2(x) AV_WARNING_HELPER1 ( #x )
#define AV_DISABLE_WARNING(x) _Pragma ( AV_WARNING_HELPER2 ( x ) )

#define AV_DISABLE_COMMON_WARNINGS \
    AV_SAVE_WARNING_STATE \
    AV_DISABLE_WARNING ( -Wshadow )


#endif // ANDROID_VULKAN_WARNING_H
