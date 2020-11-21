# About _C++ 20_, `-Wpedantic` support and _Android Studio_ integration

At the moment **[2020/11/21]**, there is no obvious way to connect `-Wpedantic` compiler flag with the project on _C++ 20_ while maintaining the integration with [_Android Studio_](https://developer.android.com/studio). The logic is this:

[_Android Studio_](https://developer.android.com/studio) is built on the [_CLion_](https://www.jetbrains.com/clion/) core. Now [_CLion_](https://www.jetbrains.com/clion/) parses a line from `CMakeLists.txt` like

`set (`[`CMAKE_CXX_STANDARD`](https://cmake.org/cmake/help/v3.10/variable/CMAKE_CXX_STANDARD.html) `xx)`

Depending on this the _IDE_ kernel calls [`clangd`](https://clangd.llvm.org/) (aka _IntelliSense_ server) and tells it which version of _C++_ standard to use.

It seems that the _Google_ remains on [_CMake_](https://cmake.org/) version `3.10.2` due to internal problems with the build system on [_Android NDK_](https://developer.android.com/ndk) + [Gradle](https://gradle.org/) + [_Android Studio_](https://developer.android.com/studio).

In this version of [_CMake_](https://cmake.org/), you cannot select a standard higher than 17 (via [`CMAKE_CXX_STANDARD`](https://cmake.org/cmake/help/v3.10/variable/CMAKE_CXX_STANDARD.html)).

On the other hand, if you do not specify the standard through [`CMAKE_CXX_STANDARD`](https://cmake.org/cmake/help/v3.10/variable/CMAKE_CXX_STANDARD.html), but specify it through the [`CMAKE_CXX_FLAGS`](https://cmake.org/cmake/help/v3.10/envvar/CXXFLAGS.html) flags, then the compilation will pass. However, this will completely disable [_clang-tidy_](https://clang.llvm.org/extra/clang-tidy/) integration in the [_Android Studio_](https://developer.android.com/studio) _IDE_.
Even `[[maybe_unused]]` will stop working because they appeared in _C++ 17_.

If you specify [`CMAKE_CXX_STANDARD`](https://cmake.org/cmake/help/v3.10/variable/CMAKE_CXX_STANDARD.html) 17, and throw the [`CMAKE_CXX_FLAGS`](https://cmake.org/cmake/help/v3.10/envvar/CXXFLAGS.html) flags for _C++ 20_ support then the build system will break because the resulting compilation command will contain both flags of the standards (20 and 17). At the same time standard 17 will win. As a consequence `-Wpedantic` will fall off on [`designated initializers`](https://en.cppreference.com/w/cpp/language/aggregate_initialization) extension, for example.

```bash
clang ... -std = c ++ 20 ... -std = gnu ++ 1z -o <*.o> <*.cpp>
```
