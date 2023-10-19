param
(
    [Switch]
    $shaderSource
)

Clear-Host
.\make-env.ps1 $shaderSource

# vertex shaders
.\make-vs.ps1 common_opaque
.\make-vs.ps1 full_screen_triangle
.\make-vs.ps1 light_volume
.\make-vs.ps1 mandelbrot
.\make-vs.ps1 point_light_shadowmap_generator
.\make-vs.ps1 reflection_global
.\make-vs.ps1 static_mesh
.\make-vs.ps1 ui

# pixel shaders
.\make-ps.ps1 blinn_phong_analytic
.\make-ps.ps1 blinn_phong_lut
.\make-ps.ps1 mandelbrot_analytic_color
.\make-ps.ps1 mandelbrot_lut_color
.\make-ps.ps1 null
.\make-ps.ps1 opaque
.\make-ps.ps1 point_light
.\make-ps.ps1 reflection_global
.\make-ps.ps1 reflection_local
.\make-ps.ps1 stipple
.\make-ps.ps1 tone_mapper
.\make-ps.ps1 ui

# compute shaders
.\make-cs.ps1 exposure
.\make-cs.ps1 skin
