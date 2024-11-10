cmake_minimum_required ( VERSION 3.22.1 )

project ( html-validator LANGUAGES CXX )

# Using C++ 20 standard.
set ( CMAKE_CXX_STANDARD 20 )
set ( CMAKE_CXX_STANDARD_REQUIRED ON )

add_executable ( html-validator
    sources/main.cpp
    sources/windows/main.rc
    ../../app/src/main/cpp/sources/GXCommon/GXMath.cpp
    ../../app/src/main/cpp/sources/GXCommon/GXMath.cpp
    ../../app/src/main/cpp/sources/GXCommon/Intrinsics/GXMathCPU.cpp
    ../../app/src/main/cpp/sources/pbr/ascii_string.cpp
    ../../app/src/main/cpp/sources/pbr/attribute_checker.cpp
    ../../app/src/main/cpp/sources/pbr/attribute_parser.cpp
    ../../app/src/main/cpp/sources/pbr/color_property.cpp
    ../../app/src/main/cpp/sources/pbr/color_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/color_value.cpp
    ../../app/src/main/cpp/sources/pbr/common_css_rule.cpp
    ../../app/src/main/cpp/sources/pbr/css_computed_values.cpp
    ../../app/src/main/cpp/sources/pbr/css_parser.cpp
    ../../app/src/main/cpp/sources/pbr/display_property.cpp
    ../../app/src/main/cpp/sources/pbr/display_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/div_html5_element.cpp
    ../../app/src/main/cpp/sources/pbr/doctype_parser.cpp
    ../../app/src/main/cpp/sources/pbr/fail_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/font_face_css_rule.cpp
    ../../app/src/main/cpp/sources/pbr/font_family_property.cpp
    ../../app/src/main/cpp/sources/pbr/font_family_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/href_attribute_checker.cpp
    ../../app/src/main/cpp/sources/pbr/html5_element.cpp
    ../../app/src/main/cpp/sources/pbr/html5_parser.cpp
    ../../app/src/main/cpp/sources/pbr/html5_tag.cpp
    ../../app/src/main/cpp/sources/pbr/image_attribute_checker.cpp
    ../../app/src/main/cpp/sources/pbr/img_html5_element.cpp
    ../../app/src/main/cpp/sources/pbr/length_property.cpp
    ../../app/src/main/cpp/sources/pbr/length_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/length_shorthand_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/length_value.cpp
    ../../app/src/main/cpp/sources/pbr/length_value_parser.cpp
    ../../app/src/main/cpp/sources/pbr/link_html5_element.cpp
    ../../app/src/main/cpp/sources/pbr/number_parser.cpp
    ../../app/src/main/cpp/sources/pbr/position_property.cpp
    ../../app/src/main/cpp/sources/pbr/position_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/property.cpp
    ../../app/src/main/cpp/sources/pbr/property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/property_parser.cpp
    ../../app/src/main/cpp/sources/pbr/set_attribute_checker.cpp
    ../../app/src/main/cpp/sources/pbr/src_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/stream.cpp
    ../../app/src/main/cpp/sources/pbr/stylesheet_rel_attribute_checker.cpp
    ../../app/src/main/cpp/sources/pbr/tag_parser.cpp
    ../../app/src/main/cpp/sources/pbr/text_align_property.cpp
    ../../app/src/main/cpp/sources/pbr/text_align_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/text_html5_element.cpp
    ../../app/src/main/cpp/sources/pbr/unique_attribute_checker.cpp
    ../../app/src/main/cpp/sources/pbr/utf8_parser.cpp
    ../../app/src/main/cpp/sources/pbr/vertical_align_property.cpp
    ../../app/src/main/cpp/sources/pbr/vertical_align_property_checker.cpp
    ../../app/src/main/cpp/sources/pbr/whitespace.cpp
    ../../app/src/main/cpp/sources/platform/windows/file.cpp
    ../../app/src/main/cpp/sources/platform/windows/logger.cpp
)

target_include_directories ( html-validator
    PRIVATE
    ../../app/src/main/cpp/include
)

target_compile_options ( html-validator PRIVATE
    /Wall
    /WX
    /wd4820
    /wd4514
    /wd4577
    /wd4710
    /wd4711
    /wd5045
)

target_compile_definitions ( html-validator PRIVATE
    NOMINMAX
)

# Precompiled headers
target_precompile_headers ( html-validator PRIVATE
    ../../app/src/main/cpp/include/precompiled_headers.hpp
)

# See https://cmake.org/cmake/help/v3.22/prop_tgt/MSVC_RUNTIME_LIBRARY.html
set_property ( TARGET html-validator PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
