cmake_minimum_required ( VERSION 3.22.1 )
set ( CMAKE_CXX_STANDARD 20 )
project ( android-vulkan )

# Enable link time optimization for release build via native flag.
# Note there is CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE built-in variable but it works not reliably with
# NDK 22.0.7026061 at link time (linking errors on Linux).
string ( APPEND CMAKE_CXX_FLAGS_RELEASE " -flto=thin" )

# log support
find_library ( log-lib log REQUIRED )

# Main app
add_library ( android-vulkan SHARED
    app/src/main/cpp/sources/animation_track.cpp
    app/src/main/cpp/sources/contact_detector.cpp
    app/src/main/cpp/sources/contact_manager.cpp
    app/src/main/cpp/sources/core.cpp
    app/src/main/cpp/sources/cyrus_beck.cpp
    app/src/main/cpp/sources/epa.cpp
    app/src/main/cpp/sources/game.cpp
    app/src/main/cpp/sources/gamepad.cpp
    app/src/main/cpp/sources/gjk.cpp
    app/src/main/cpp/sources/gjk_base.cpp
    app/src/main/cpp/sources/global_force_gravity.cpp
    app/src/main/cpp/sources/guid_generator.cpp
    app/src/main/cpp/sources/half_types.cpp
    app/src/main/cpp/sources/ktx_media_container.cpp
    app/src/main/cpp/sources/memory_allocator.cpp
    app/src/main/cpp/sources/mesh_geometry.cpp
    app/src/main/cpp/sources/pcm_streamer.cpp
    app/src/main/cpp/sources/pcm_streamer_ogg.cpp
    app/src/main/cpp/sources/pcm_streamer_wav.cpp
    app/src/main/cpp/sources/physics.cpp
    app/src/main/cpp/sources/ray_caster.cpp
    app/src/main/cpp/sources/renderer.cpp
    app/src/main/cpp/sources/rigid_body.cpp
    app/src/main/cpp/sources/shape.cpp
    app/src/main/cpp/sources/shape_box.cpp
    app/src/main/cpp/sources/shape_sphere.cpp
    app/src/main/cpp/sources/simplex.cpp
    app/src/main/cpp/sources/skin_data.cpp
    app/src/main/cpp/sources/sound_emitter.cpp
    app/src/main/cpp/sources/sound_emitter_global.cpp
    app/src/main/cpp/sources/sound_emitter_spatial.cpp
    app/src/main/cpp/sources/sound_mixer.cpp
    app/src/main/cpp/sources/sound_storage.cpp
    app/src/main/cpp/sources/sutherland_hodgman.cpp
    app/src/main/cpp/sources/texture_cube.cpp
    app/src/main/cpp/sources/texture2D.cpp
    app/src/main/cpp/sources/trace.cpp
    app/src/main/cpp/sources/uniform_buffer.cpp
    app/src/main/cpp/sources/velocity_solver.cpp
    app/src/main/cpp/sources/vulkan_loader.cpp
    app/src/main/cpp/sources/vulkan_utils.cpp
    app/src/main/cpp/sources/GXCommon/GXMath.cpp
    app/src/main/cpp/sources/GXCommon/Intrinsics/GXMathNeon.cpp
    app/src/main/cpp/sources/GXCommon/Vulkan/GXMathBackend.cpp
    app/src/main/cpp/sources/intrinsics/pcm_streamer_neon.cpp
    app/src/main/cpp/sources/mandelbrot/mandelbrot_analytic_color.cpp
    app/src/main/cpp/sources/mandelbrot/mandelbrot_base.cpp
    app/src/main/cpp/sources/mandelbrot/mandelbrot_lut_color.cpp
    app/src/main/cpp/sources/pbr/actor.cpp
    app/src/main/cpp/sources/pbr/animation_graph.cpp
    app/src/main/cpp/sources/pbr/animation_player_node.cpp
    app/src/main/cpp/sources/pbr/ascii_string.cpp
    app/src/main/cpp/sources/pbr/attribute_checker.cpp
    app/src/main/cpp/sources/pbr/attribute_parser.cpp
    app/src/main/cpp/sources/pbr/camera.cpp
    app/src/main/cpp/sources/pbr/camera_component.cpp
    app/src/main/cpp/sources/pbr/color_property.cpp
    app/src/main/cpp/sources/pbr/color_property_checker.cpp
    app/src/main/cpp/sources/pbr/color_value.cpp
    app/src/main/cpp/sources/pbr/common_css_rule.cpp
    app/src/main/cpp/sources/pbr/component.cpp
    app/src/main/cpp/sources/pbr/compute_program.cpp
    app/src/main/cpp/sources/pbr/css_computed_values.cpp
    app/src/main/cpp/sources/pbr/css_parser.cpp
    app/src/main/cpp/sources/pbr/css_ui_element.cpp
    app/src/main/cpp/sources/pbr/cube_map_manager.cpp
    app/src/main/cpp/sources/pbr/default_texture_manager.cpp
    app/src/main/cpp/sources/pbr/display_property.cpp
    app/src/main/cpp/sources/pbr/display_property_checker.cpp
    app/src/main/cpp/sources/pbr/div_html5_element.cpp
    app/src/main/cpp/sources/pbr/div_ui_element.cpp
    app/src/main/cpp/sources/pbr/doctype_parser.cpp
    app/src/main/cpp/sources/pbr/exposure_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/exposure_pass.cpp
    app/src/main/cpp/sources/pbr/exposure_program.cpp
    app/src/main/cpp/sources/pbr/fail_property_checker.cpp
    app/src/main/cpp/sources/pbr/font_face_css_rule.cpp
    app/src/main/cpp/sources/pbr/font_family_property.cpp
    app/src/main/cpp/sources/pbr/font_family_property_checker.cpp
    app/src/main/cpp/sources/pbr/font_storage.cpp
    app/src/main/cpp/sources/pbr/full_screen_triangle_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/gbuffer.cpp
    app/src/main/cpp/sources/pbr/geometry_call.cpp
    app/src/main/cpp/sources/pbr/geometry_pass.cpp
    app/src/main/cpp/sources/pbr/geometry_pass_instance_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/geometry_pass_material.cpp
    app/src/main/cpp/sources/pbr/geometry_pass_program.cpp
    app/src/main/cpp/sources/pbr/geometry_pass_sampler_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/geometry_pass_texture_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/geometry_subpass_base.cpp
    app/src/main/cpp/sources/pbr/graphics_program.cpp
    app/src/main/cpp/sources/pbr/href_attribute_checker.cpp
    app/src/main/cpp/sources/pbr/html5_element.cpp
    app/src/main/cpp/sources/pbr/html5_parser.cpp
    app/src/main/cpp/sources/pbr/html5_tag.cpp
    app/src/main/cpp/sources/pbr/image_attribute_checker.cpp
    app/src/main/cpp/sources/pbr/image_ui_element.cpp
    app/src/main/cpp/sources/pbr/img_html5_element.cpp
    app/src/main/cpp/sources/pbr/length_property.cpp
    app/src/main/cpp/sources/pbr/length_property_checker.cpp
    app/src/main/cpp/sources/pbr/length_shorthand_property_checker.cpp
    app/src/main/cpp/sources/pbr/length_value.cpp
    app/src/main/cpp/sources/pbr/length_value_parser.cpp
    app/src/main/cpp/sources/pbr/light.cpp
    app/src/main/cpp/sources/pbr/light_lightup_base_program.cpp
    app/src/main/cpp/sources/pbr/light_pass.cpp
    app/src/main/cpp/sources/pbr/light_volume_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/lightup_common_descriptor_set.cpp
    app/src/main/cpp/sources/pbr/lightup_common_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/link_html5_element.cpp
    app/src/main/cpp/sources/pbr/material.cpp
    app/src/main/cpp/sources/pbr/material_manager.cpp
    app/src/main/cpp/sources/pbr/material_pool.cpp
    app/src/main/cpp/sources/pbr/mesh_group.cpp
    app/src/main/cpp/sources/pbr/mesh_manager.cpp
    app/src/main/cpp/sources/pbr/node_link.cpp
    app/src/main/cpp/sources/pbr/number_parser.cpp
    app/src/main/cpp/sources/pbr/opaque_material.cpp
    app/src/main/cpp/sources/pbr/opaque_program.cpp
    app/src/main/cpp/sources/pbr/opaque_subpass.cpp
    app/src/main/cpp/sources/pbr/pbr_game.cpp
    app/src/main/cpp/sources/pbr/point_light.cpp
    app/src/main/cpp/sources/pbr/point_light_component.cpp
    app/src/main/cpp/sources/pbr/point_light_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/point_light_lightup.cpp
    app/src/main/cpp/sources/pbr/point_light_lightup_program.cpp
    app/src/main/cpp/sources/pbr/point_light_pass.cpp
    app/src/main/cpp/sources/pbr/point_light_shadowmap_generator_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/point_light_shadowmap_generator_program.cpp
    app/src/main/cpp/sources/pbr/position_property.cpp
    app/src/main/cpp/sources/pbr/position_property_checker.cpp
    app/src/main/cpp/sources/pbr/property.cpp
    app/src/main/cpp/sources/pbr/property_checker.cpp
    app/src/main/cpp/sources/pbr/property_parser.cpp
    app/src/main/cpp/sources/pbr/present_render_pass.cpp
    app/src/main/cpp/sources/pbr/reflection_component.cpp
    app/src/main/cpp/sources/pbr/reflection_global_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/reflection_global_pass.cpp
    app/src/main/cpp/sources/pbr/reflection_global_program.cpp
    app/src/main/cpp/sources/pbr/reflection_local_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/reflection_local_pass.cpp
    app/src/main/cpp/sources/pbr/reflection_local_program.cpp
    app/src/main/cpp/sources/pbr/reflection_probe.cpp
    app/src/main/cpp/sources/pbr/reflection_probe_global.cpp
    app/src/main/cpp/sources/pbr/reflection_probe_local.cpp
    app/src/main/cpp/sources/pbr/render_session.cpp
    app/src/main/cpp/sources/pbr/render_session_stats.cpp
    app/src/main/cpp/sources/pbr/renderable_component.cpp
    app/src/main/cpp/sources/pbr/rigid_body_component.cpp
    app/src/main/cpp/sources/pbr/sampler.cpp
    app/src/main/cpp/sources/pbr/sampler_manager.cpp
    app/src/main/cpp/sources/pbr/scene.cpp
    app/src/main/cpp/sources/pbr/script_component.cpp
    app/src/main/cpp/sources/pbr/script_engine.cpp
    app/src/main/cpp/sources/pbr/scriptable_gamepad.cpp
    app/src/main/cpp/sources/pbr/scriptable_gxmat3.cpp
    app/src/main/cpp/sources/pbr/scriptable_gxmat4.cpp
    app/src/main/cpp/sources/pbr/scriptable_gxquat.cpp
    app/src/main/cpp/sources/pbr/scriptable_gxvec3.cpp
    app/src/main/cpp/sources/pbr/scriptable_gxvec4.cpp
    app/src/main/cpp/sources/pbr/scriptable_logger.cpp
    app/src/main/cpp/sources/pbr/scriptable_material.cpp
    app/src/main/cpp/sources/pbr/scriptable_penetration.cpp
    app/src/main/cpp/sources/pbr/scriptable_sweep_test_result.cpp
    app/src/main/cpp/sources/pbr/set_attribute_checker.cpp
    app/src/main/cpp/sources/pbr/skeletal_mesh_component.cpp
    app/src/main/cpp/sources/pbr/skin_pool.cpp
    app/src/main/cpp/sources/pbr/sound_emitter_global_component.cpp
    app/src/main/cpp/sources/pbr/sound_emitter_spatial_component.cpp
    app/src/main/cpp/sources/pbr/src_property_checker.cpp
    app/src/main/cpp/sources/pbr/srgb_program.cpp
    app/src/main/cpp/sources/pbr/static_mesh_component.cpp
    app/src/main/cpp/sources/pbr/stipple_material.cpp
    app/src/main/cpp/sources/pbr/stipple_program.cpp
    app/src/main/cpp/sources/pbr/stipple_subpass.cpp
    app/src/main/cpp/sources/pbr/stream.cpp
    app/src/main/cpp/sources/pbr/stub_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/stylesheet_rel_attribute_checker.cpp
    app/src/main/cpp/sources/pbr/tag_parser.cpp
    app/src/main/cpp/sources/pbr/text_align_property.cpp
    app/src/main/cpp/sources/pbr/text_align_property_checker.cpp
    app/src/main/cpp/sources/pbr/text_html5_element.cpp
    app/src/main/cpp/sources/pbr/text_ui_element.cpp
    app/src/main/cpp/sources/pbr/tone_mapper_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/tone_mapper_pass.cpp
    app/src/main/cpp/sources/pbr/tone_mapper_program.cpp
    app/src/main/cpp/sources/pbr/transform_component.cpp
    app/src/main/cpp/sources/pbr/ui_element.cpp
    app/src/main/cpp/sources/pbr/ui_layer.cpp
    app/src/main/cpp/sources/pbr/ui_pass.cpp
    app/src/main/cpp/sources/pbr/ui_pass_common_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/ui_pass_image_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/ui_pass_transform_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/ui_program.cpp
    app/src/main/cpp/sources/pbr/uniform_buffer_pool.cpp
    app/src/main/cpp/sources/pbr/uniform_buffer_pool_manager.cpp
    app/src/main/cpp/sources/pbr/unique_attribute_checker.cpp
    app/src/main/cpp/sources/pbr/universal_game.cpp
    app/src/main/cpp/sources/pbr/utf8_parser.cpp
    app/src/main/cpp/sources/pbr/vertical_align_property.cpp
    app/src/main/cpp/sources/pbr/vertical_align_property_checker.cpp
    app/src/main/cpp/sources/pbr/whitespace.cpp
    app/src/main/cpp/sources/pbr/box_stack/box_stack.cpp
    app/src/main/cpp/sources/pbr/collision/collision.cpp
    app/src/main/cpp/sources/pbr/collision/manipulator.cpp
    app/src/main/cpp/sources/pbr/ray_casting/ray_casting.cpp
    app/src/main/cpp/sources/pbr/stipple_test/stipple_test.cpp
    app/src/main/cpp/sources/pbr/sweep_testing/actor_body.cpp
    app/src/main/cpp/sources/pbr/sweep_testing/actor_sweep.cpp
    app/src/main/cpp/sources/pbr/sweep_testing/sweep_testing.cpp
    app/src/main/cpp/sources/platform/android/file.cpp
    app/src/main/cpp/sources/platform/android/logger.cpp
    app/src/main/cpp/sources/rainbow/rainbow.cpp
    app/src/main/cpp/sources/rotating_mesh/game.cpp
    app/src/main/cpp/sources/rotating_mesh/game_analytic.cpp
    app/src/main/cpp/sources/rotating_mesh/game_lut.cpp
)

# ARM Neon support: All ARMv8-based devices support Neon. So no any additional compile flags need to specify.
# See https://developer.android.com/ndk/guides/cpu-arm-neon#cmake_2
target_include_directories ( android-vulkan PRIVATE
    app/src/main/cpp/include
    app/src/main/cpp/include/freetype2
)

# Disable libandroid-vulkan.so symbol publishing
set_target_properties ( android-vulkan PROPERTIES
    CXX_VISIBILITY_PRESET hidden
)

# Treat compile warnings as errors
target_compile_options ( android-vulkan PRIVATE
    -fno-exceptions
    -fno-rtti
    -Wall
    -Werror
    -Wextra
    -Wpedantic
    -Wshadow
)

# See docs/preprocessor-macros.md
target_compile_definitions ( android-vulkan PRIVATE
#    -DANDROID_ENABLE_TRACE
    -DANDROID_NATIVE_MODE_PORTRAIT
    -DANDROID_VULKAN_DEBUG
#    -DANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION
    -DANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS
    -DANDROID_VULKAN_STRICT_MODE
    -DVK_NO_PROTOTYPES
    -DVK_USE_PLATFORM_ANDROID_KHR
)

target_link_libraries ( android-vulkan
    aaudio
    android
    ${log-lib}
    ${CMAKE_SOURCE_DIR}/third-party/jniLibs/arm64-v8a/libfreetype.so
    ${CMAKE_SOURCE_DIR}/third-party/jniLibs/arm64-v8a/libLua.so
    ${CMAKE_SOURCE_DIR}/third-party/jniLibs/arm64-v8a/libvorbisfile.so
)
