cmake_minimum_required ( VERSION 3.22.1 )
set ( CMAKE_CXX_STANDARD 20 )
project ( Vulkan-validation-layers )

# Enable link time optimization for release build via native flag.
# Note there is CMAKE_INTERPROCEDURAL_OPTIMZATION_RELEASE built-in variable but it works not reliably with
# NDK 22.0.7026061 at link time (linking errors on Linux).
string ( APPEND CMAKE_CXX_FLAGS_RELEASE " -flto=thin" )

# layer_utils static library

add_library ( layer_utils
    STATIC
    ../layers/vk_layer_config.cpp
    ../layers/utils/vk_layer_extension_utils.cpp
    ../layers/error_message/logging.cpp
    ../layers/utils/vk_layer_utils.cpp
    ../layers/generated/vk_format_utils.cpp
    ../layers/external/xxhash.cpp
)

target_include_directories ( layer_utils
    PRIVATE
    ../layers/generated
    ../layers
    Vulkan-Headers/include
    ../layers/external
    robin-hood-hashing/src/include
)

target_compile_definitions ( layer_utils
    PRIVATE
    -DVK_ENABLE_BETA_EXTENSIONS
    -DVK_USE_PLATFORM_ANDROID_KHR
    -DUSE_ROBIN_HOOD_HASHING
    -DXXH_NO_LONG_LONG
)

target_compile_options ( layer_utils PRIVATE
    -Wall
    -Werror
    -Wno-unused-const-variable
    -Wno-unused-function
    -fvisibility=hidden
)

set_target_properties ( layer_utils PROPERTIES
    CXX_VISIBILITY_PRESET hidden
)

# Finding Android log library

find_library ( log-lib
    log
)

# VkLayer_khronos_validation library

add_library ( VkLayer_khronos_validation
    SHARED
    ../layers/state_tracker/state_tracker.cpp
    ../layers/core_checks/cc_android.cpp
    ../layers/core_checks/cc_device.cpp
    ../layers/state_tracker/device_memory_state.cpp
    ../layers/core_checks/cc_device_memory.cpp
    ../layers/core_checks/cc_external_object.cpp
    ../layers/state_tracker/base_node.cpp
    ../layers/state_tracker/buffer_state.cpp
    ../layers/state_tracker/cmd_buffer_state.cpp
    ../layers/core_checks/cc_cmd_buffer_dynamic.cpp
    ../layers/core_checks/cc_cmd_buffer.cpp
    ../layers/core_checks/cc_copy_blit_resolve.cpp
    ../layers/state_tracker/image_state.cpp
    ../layers/state_tracker/pipeline_state.cpp
    ../layers/state_tracker/pipeline_layout_state.cpp
    ../layers/state_tracker/pipeline_sub_state.cpp
    ../layers/core_checks/cc_image.cpp
    ../layers/core_checks/cc_image_layout.cpp
    ../layers/core_checks/cc_pipeline_compute.cpp
    ../layers/core_checks/cc_pipeline_graphics.cpp
    ../layers/core_checks/cc_pipeline_ray_tracing.cpp
    ../layers/core_checks/cc_pipeline.cpp
    ../layers/state_tracker/queue_state.cpp
    ../layers/state_tracker/render_pass_state.cpp
    ../layers/core_checks/cc_render_pass.cpp
    ../layers/state_tracker/video_session_state.cpp
    ../layers/core_checks/cc_video.cpp
    ../layers/core_checks/cc_drawdispatch.cpp
    ../layers/state_tracker/descriptor_sets.cpp
    ../layers/core_checks/cc_descriptor.cpp
    ../layers/core_checks/cc_buffer.cpp
    ../layers/state_tracker/shader_module.cpp
    ../layers/state_tracker/shader_instruction.cpp
    ../layers/core_checks/cc_shader.cpp
    ../layers/core_checks/cc_synchronization.cpp
    ../layers/generated/spirv_validation_helper.cpp
    ../layers/generated/spirv_grammar_helper.cpp
    ../layers/generated/command_validation.cpp
    ../layers/generated/dynamic_state_helper.cpp
    ../layers/gpu_validation/gpu_validation.cpp
    ../layers/gpu_validation/gpu_utils.cpp
    ../layers/gpu_validation/debug_printf.cpp
    ../layers/best_practices/best_practices_utils.cpp
    ../layers/best_practices/bp_buffer.cpp
    ../layers/best_practices/bp_cmd_buffer.cpp
    ../layers/best_practices/bp_copy_blit_resolve.cpp
    ../layers/best_practices/bp_descriptor.cpp
    ../layers/best_practices/bp_device_memory.cpp
    ../layers/best_practices/bp_drawdispatch.cpp
    ../layers/best_practices/bp_framebuffer.cpp
    ../layers/best_practices/bp_image.cpp
    ../layers/best_practices/bp_instance_device.cpp
    ../layers/best_practices/bp_pipeline.cpp
    ../layers/best_practices/bp_ray_tracing.cpp
    ../layers/best_practices/bp_render_pass.cpp
    ../layers/best_practices/bp_synchronization.cpp
    ../layers/best_practices/bp_video.cpp
    ../layers/best_practices/bp_wsi.cpp
    ../layers/generated/best_practices.cpp
    ../layers/sync/sync_utils.cpp
    ../layers/sync/sync_vuid_maps.cpp
    ../layers/error_message/core_error_location.cpp
    ../layers/generated/sync_validation_types.cpp
    ../layers/sync/sync_validation.cpp
    ../layers/utils/convert_to_renderpass2.cpp
    ../layers/generated/layer_chassis_dispatch.cpp
    ../layers/generated/chassis.cpp
    ../layers/generated/valid_param_values.cpp
    ../layers/layer_options.cpp
    ../layers/core_checks/cc_query.cpp
    ../layers/core_checks/cc_queue.cpp
    ../layers/core_checks/cc_ray_tracing.cpp
    ../layers/core_checks/cc_wsi.cpp
    ../layers/core_checks/cc_ycbcr.cpp
    ../layers/generated/parameter_validation.cpp
    ../layers/stateless/sl_buffer.cpp
    ../layers/stateless/sl_cmd_buffer_dynamic.cpp
    ../layers/stateless/sl_cmd_buffer.cpp
    ../layers/stateless/sl_descriptor.cpp
    ../layers/stateless/sl_device_memory.cpp
    ../layers/stateless/sl_external_object.cpp
    ../layers/stateless/sl_framebuffer.cpp
    ../layers/stateless/sl_image.cpp
    ../layers/stateless/sl_instance_device.cpp
    ../layers/stateless/sl_pipeline.cpp
    ../layers/stateless/sl_ray_tracing.cpp
    ../layers/stateless/sl_render_pass.cpp
    ../layers/stateless/sl_synchronization.cpp
    ../layers/stateless/sl_wsi.cpp
    ../layers/generated/object_tracker.cpp
    ../layers/object_tracker/object_tracker_utils.cpp
    ../layers/generated/thread_safety.cpp
    ../layers/generated/vk_safe_struct_utils.cpp
    ../layers/generated/vk_safe_struct_core.cpp
    ../layers/generated/vk_safe_struct_khr.cpp
    ../layers/generated/vk_safe_struct_ext.cpp
    ../layers/generated/vk_safe_struct_vendor.cpp
    ../layers/state_tracker/image_layout_map.cpp
    ../layers/containers/subresource_adapter.cpp
    ../layers/external/vma/vma.cpp
)

set_target_properties ( VkLayer_khronos_validation PROPERTIES
    CXX_VISIBILITY_PRESET hidden
)

target_include_directories ( VkLayer_khronos_validation
    PRIVATE
    ../layers
    ../layers/generated
    Vulkan-Headers/include
    ../layers/external
    robin-hood-hashing/src/include
    SPIRV-Headers/include
    SPIRV-Tools/include
)

target_compile_definitions ( VkLayer_khronos_validation
    PRIVATE
    -DVK_ENABLE_BETA_EXTENSIONS
    -DVK_USE_PLATFORM_ANDROID_KHR
    -DUSE_ROBIN_HOOD_HASHING
    -DXXH_NO_LONG_LONG
)

target_compile_options ( VkLayer_khronos_validation PRIVATE
    -Wall
    -Werror
    -Wno-nullability-completeness
    -Wno-unused-const-variable
    -Wno-unused-function
    -Wno-unused-variable
    -frtti
    -fvisibility=hidden
)

target_link_libraries ( VkLayer_khronos_validation
    ${log-lib}
    android
    $ENV{VK_LAYER_DIR}/glslang/glslang/libglslang.a
    $ENV{VK_LAYER_DIR}/SPIRV-Tools/build/app/local/arm64-v8a/libSPIRV-Tools.a
    $ENV{VK_LAYER_DIR}/SPIRV-Tools/build/app/local/arm64-v8a/libSPIRV-Tools-opt.a
    layer_utils
)
