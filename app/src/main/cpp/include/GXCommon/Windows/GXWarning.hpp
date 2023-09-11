// version 1.4
// No include guard allowed
// Not include this explicitly! Use GXCommon/GXWarning.h instead.


#define GX_SAVE_WARNING_STATE __pragma ( warning ( push ) )
#define GX_RESTORE_WARNING_STATE __pragma ( warning ( pop ) )

#define GX_DISABLE_WARNING(w) __pragma ( warning ( disable : w ) )

#define GX_DISABLE_COMMON_WARNINGS \
    GX_SAVE_WARNING_STATE \
    GX_DISABLE_WARNING ( 4091 )     /* 'keyword' : ignored on left of 'type' when no variable is declared */ \
    GX_DISABLE_WARNING ( 4100 )     /* 'identifier' : unreferenced formal parameter */ \
    GX_DISABLE_WARNING ( 4263 )     /* 'function' : member function does not override any base class virtual member function */ \
    GX_DISABLE_WARNING ( 4264 )     /* 'virtual_function' : no override available for virtual member function from base 'class'; function is hidden */ \
    GX_DISABLE_WARNING ( 4266 )     /* no override available for virtual member function from base 'BaseObject'; function is hidden */ \
    GX_DISABLE_WARNING ( 4365 )     /* 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch */ \
    GX_DISABLE_WARNING ( 4464 )     /* relative include path contains '..' */ \
    GX_DISABLE_WARNING ( 4471 )     /* 'enumeration': a forward declaration of an unscoped enumeration must have an underlying type (int assumed) */ \
    GX_DISABLE_WARNING ( 4505 )     /* unreferenced local function has been removed */ \
    GX_DISABLE_WARNING ( 4530 )     /* C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc */ \
    GX_DISABLE_WARNING ( 4555 )     /* expression has no effect; expected expression with side-effect */ \
    GX_DISABLE_WARNING ( 4619 )     /* #pragma warning : there is no warning number 'number' */ \
    GX_DISABLE_WARNING ( 4625 )     /* copy constructor was implicitly defined as deleted */ \
    GX_DISABLE_WARNING ( 4626 )     /* assignment operator was implicitly defined as deleted */ \
    GX_DISABLE_WARNING ( 4668 )     /* 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives' */ \
    GX_DISABLE_WARNING ( 4710 )     /* 'function' : function not inlined */ \
    GX_DISABLE_WARNING ( 4774 )     /* 'string' : format string expected in argument number is not a string literal */ \
    GX_DISABLE_WARNING ( 4917 )     /* 'declarator' : a GUID can only be associated with a class, interface or namespace */ \
    GX_DISABLE_WARNING ( 5026 )     /* move constructor was implicitly defined as deleted */ \
    GX_DISABLE_WARNING ( 5027 )     /* move assignment operator was implicitly defined as deleted */ \
    GX_DISABLE_WARNING ( 5038 )     /* data member 'member1' will be initialized after data member 'member2' */ \
    GX_DISABLE_WARNING ( 5039 )     /* 'function': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception */ \
    GX_DISABLE_WARNING ( 5204 )     /* class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly */ \
    GX_DISABLE_WARNING ( 5219 )     /* implicit conversion from 'type' to 'float', possible loss of data */ \
    GX_DISABLE_WARNING ( 5262 )     /* implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases */ \
    GX_DISABLE_WARNING ( 5264 )     /* 'variable-name': 'const' variable is not used */ \
    GX_DISABLE_WARNING ( 5266 )     /* 'const' qualifier on return type has no effect */
