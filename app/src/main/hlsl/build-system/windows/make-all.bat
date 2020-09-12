@echo off

:: vertex shaders
call make-vs.bat common-opaque
call make-vs.bat common-opaque-batch
call make-vs.bat mandelbrot
call make-vs.bat screen-quad
call make-vs.bat static-mesh

:: pixel shaders
call make-ps.bat blinn-phong-analytic
call make-ps.bat blinn-phong-lut
call make-ps.bat mandelbrot-analytic-color
call make-ps.bat mandelbrot-lut-color
call make-ps.bat opaque
call make-ps.bat texture-present
