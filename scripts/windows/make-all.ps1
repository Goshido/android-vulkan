param
(
    [Switch]
    $shaderSource
)

Clear-Host
. scripts\windows\make-env.ps1 $shaderSource

# vertex shaders
scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$EDITOR_HLSL_DIRECTORY\hello_triangle.vs.hlsl"                                                                    `
    "$EDITOR_SHADER_DIRECTORY\hello_triangle.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\common_opaque.vs.hlsl"                                                                     `
    "$MOBILE_SHADER_DIRECTORY\common_opaque.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\dummy.vs.hlsl"                                                                             `
    "$MOBILE_SHADER_DIRECTORY\dummy.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\full_screen_triangle.vs.hlsl"                                                              `
    "$MOBILE_SHADER_DIRECTORY\full_screen_triangle.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\light_volume.vs.hlsl"                                                                      `
    "$MOBILE_SHADER_DIRECTORY\light_volume.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\mandelbrot.vs.hlsl"                                                                        `
    "$MOBILE_SHADER_DIRECTORY\mandelbrot.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\point_light_shadowmap_generator.vs.hlsl"                                                   `
    "$MOBILE_SHADER_DIRECTORY\point_light_shadowmap_generator.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\reflection_global.vs.hlsl"                                                                 `
    "$MOBILE_SHADER_DIRECTORY\reflection_global.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\static_mesh.vs.hlsl"                                                                       `
    "$MOBILE_SHADER_DIRECTORY\static_mesh.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\ui.vs.hlsl"                                                                                `
    "$MOBILE_SHADER_DIRECTORY\ui.vs.spv"

# pixel shaders
scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$EDITOR_HLSL_DIRECTORY\hello_triangle.ps.hlsl"                                                                    `
    "$EDITOR_SHADER_DIRECTORY\hello_triangle.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\blinn_phong_analytic.ps.hlsl"                                                              `
    "$MOBILE_SHADER_DIRECTORY\blinn_phong_analytic.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\blinn_phong_lut.ps.hlsl"                                                                   `
    "$MOBILE_SHADER_DIRECTORY\blinn_phong_lut.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\dummy_gbuffer.ps.hlsl"                                                                     `
    "$MOBILE_SHADER_DIRECTORY\dummy_gbuffer.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\dummy_light.ps.hlsl"                                                                       `
    "$MOBILE_SHADER_DIRECTORY\dummy_light.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\mandelbrot_analytic_color.ps.hlsl"                                                         `
    "$MOBILE_SHADER_DIRECTORY\mandelbrot_analytic_color.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\mandelbrot_lut_color.ps.hlsl"                                                              `
    "$MOBILE_SHADER_DIRECTORY\mandelbrot_lut_color.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\null.ps.hlsl"                                                                              `
    "$MOBILE_SHADER_DIRECTORY\null.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\opaque.ps.hlsl"                                                                            `
    "$MOBILE_SHADER_DIRECTORY\opaque.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\point_light.ps.hlsl"                                                                       `
    "$MOBILE_SHADER_DIRECTORY\point_light.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\reflection_global.ps.hlsl"                                                                 `
    "$MOBILE_SHADER_DIRECTORY\reflection_global.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\reflection_local.ps.hlsl"                                                                  `
    "$MOBILE_SHADER_DIRECTORY\reflection_local.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\stipple.ps.hlsl"                                                                           `
    "$MOBILE_SHADER_DIRECTORY\stipple.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\tone_mapper_custom_brightness.ps.hlsl"                                                     `
    "$MOBILE_SHADER_DIRECTORY\tone_mapper_custom_brightness.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\tone_mapper_default_brightness.ps.hlsl"                                                    `
    "$MOBILE_SHADER_DIRECTORY\tone_mapper_default_brightness.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\ui_custom_brightness.ps.hlsl"                                                              `
    "$MOBILE_SHADER_DIRECTORY\ui_custom_brightness.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\ui_default_brightness.ps.hlsl"                                                             `
    "$MOBILE_SHADER_DIRECTORY\ui_default_brightness.ps.spv"

# compute shaders
scripts\windows\make-spv.ps1                                                                                           `
    "CS"                                                                                                               `
    "cs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\exposure.cs.hlsl"                                                                          `
    "$MOBILE_SHADER_DIRECTORY\exposure.cs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "CS"                                                                                                               `
    "cs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\skin.cs.hlsl"                                                                              `
    "$MOBILE_SHADER_DIRECTORY\skin.cs.spv"
