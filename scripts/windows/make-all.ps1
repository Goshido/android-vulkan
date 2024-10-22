param
(
    [Switch]
    $shaderSource
)

Clear-Host
scripts\windows\make-env.ps1 $shaderSource

# vertex shaders
scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$EDITOR_HLSL_DIRECTORY\hello_triangle.vs"                                                                         `
    "$EDITOR_SHADER_DIRECTORY\hello_triangle.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\common_opaque.vs"                                                                          `
    "$MOBILE_SHADER_DIRECTORY\common_opaque.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\full_screen_triangle.vs"                                                                   `
    "$MOBILE_SHADER_DIRECTORY\full_screen_triangle.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\light_volume.vs"                                                                           `
    "$MOBILE_SHADER_DIRECTORY\light_volume.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\mandelbrot.vs"                                                                             `
    "$MOBILE_SHADER_DIRECTORY\mandelbrot.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\point_light_shadowmap_generator.vs"                                                        `
    "$MOBILE_SHADER_DIRECTORY\point_light_shadowmap_generator.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\reflection_global.vs"                                                                      `
    "$MOBILE_SHADER_DIRECTORY\reflection_global.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\static_mesh.vs"                                                                            `
    "$MOBILE_SHADER_DIRECTORY\static_mesh.vs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "VS"                                                                                                               `
    "vs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\ui.vs"                                                                                     `
    "$MOBILE_SHADER_DIRECTORY\ui.vs.spv"

# pixel shaders
scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$EDITOR_HLSL_DIRECTORY\hello_triangle.ps"                                                                         `
    "$EDITOR_SHADER_DIRECTORY\hello_triangle.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\blinn_phong_analytic.ps"                                                                   `
    "$MOBILE_SHADER_DIRECTORY\blinn_phong_analytic.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\blinn_phong_lut.ps"                                                                        `
    "$MOBILE_SHADER_DIRECTORY\blinn_phong_lut.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\mandelbrot_analytic_color.ps"                                                              `
    "$MOBILE_SHADER_DIRECTORY\mandelbrot_analytic_color.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\mandelbrot_lut_color.ps"                                                                   `
    "$MOBILE_SHADER_DIRECTORY\mandelbrot_lut_color.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\null.ps"                                                                                   `
    "$MOBILE_SHADER_DIRECTORY\null.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\opaque.ps"                                                                                 `
    "$MOBILE_SHADER_DIRECTORY\opaque.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\point_light.ps"                                                                            `
    "$MOBILE_SHADER_DIRECTORY\point_light.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\reflection_global.ps"                                                                      `
    "$MOBILE_SHADER_DIRECTORY\reflection_global.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\reflection_local.ps"                                                                       `
    "$MOBILE_SHADER_DIRECTORY\reflection_local.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\stipple.ps"                                                                                `
    "$MOBILE_SHADER_DIRECTORY\stipple.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\tone_mapper_custom_brightness.ps"                                                          `
    "$MOBILE_SHADER_DIRECTORY\tone_mapper_custom_brightness.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\tone_mapper_default_brightness.ps"                                                         `
    "$MOBILE_SHADER_DIRECTORY\tone_mapper_default_brightness.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\ui_custom_brightness.ps"                                                                   `
    "$MOBILE_SHADER_DIRECTORY\ui_custom_brightness.ps.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "PS"                                                                                                               `
    "ps"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\ui_default_brightness.ps"                                                                  `
    "$MOBILE_SHADER_DIRECTORY\ui_default_brightness.ps.spv"

# compute shaders
scripts\windows\make-spv.ps1                                                                                           `
    "CS"                                                                                                               `
    "cs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\exposure.cs"                                                                               `
    "$MOBILE_SHADER_DIRECTORY\exposure.cs.spv"

scripts\windows\make-spv.ps1                                                                                           `
    "CS"                                                                                                               `
    "cs"                                                                                                               `
    "$MOBILE_HLSL_DIRECTORY\skin.cs"                                                                                   `
    "$MOBILE_SHADER_DIRECTORY\skin.cs.spv"
