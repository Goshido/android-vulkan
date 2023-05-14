Clear-Host

.\make-env.ps1

# vertex shaders
.\make-vs.ps1 common-opaque
.\make-vs.ps1 light-volume
.\make-vs.ps1 mandelbrot
.\make-vs.ps1 point-light-shadowmap-generator
.\make-vs.ps1 reflection-global
.\make-vs.ps1 screen-quad
.\make-vs.ps1 static-mesh

# pixel shaders
.\make-ps.ps1 blinn-phong-analytic
.\make-ps.ps1 blinn-phong-lut
.\make-ps.ps1 mandelbrot-analytic-color
.\make-ps.ps1 mandelbrot-lut-color
.\make-ps.ps1 null
.\make-ps.ps1 opaque
.\make-ps.ps1 point-light
.\make-ps.ps1 reflection-global
.\make-ps.ps1 reflection-local
.\make-ps.ps1 stipple
.\make-ps.ps1 texture-present
