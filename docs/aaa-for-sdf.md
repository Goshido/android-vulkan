# Analytical Anti-Aliasing for _SDF_

## <a id="table-of-content">Table of content</a>

- [_3D case_](#3d-case)
- [_SDF decal in 3D space case_](#decal-case)

## <a id="3d-case">_3D_ case</a>

The core principle of anti-aliasing Signed Distance Fields (_SDF_) is the analytical estimation of how close a given pixel's center is to the shape's boundary. The objective is to determine an accurate sub-pixel coverage value for pixels that straddle the edge of the shape.
The "loop of pixels" that forms the boundary in the final image is the critical area of focus.

The anti-aliasing algorithm assigns an alpha value to each pixel within this transition zone:

- Alpha approaches 1: when the pixel's center is extremely close to, or just inside, the shape's boundary.
- Alpha approaches 0: when the pixel's center is far from the boundary.

This analytical method uses the _SDF_ value itself, scaled by the estimated pixel size, to smoothly interpolate between full transparency and full opacity, effectively simulating a precise sub-pixel fill without resorting to costly supersampling.

<img src="./images/sdf-aaa-loop.svg"/>

The most tricky task is to estimate pixel coverage. The _SDF_ shape is defined in _3D_ coordinates and volumes, while the screen is defined by _2D_ pixel grids and areas. The relationship between camera settings and the pixel coverage is determined by the following rules:

<img src="./images/pixel-size.svg"/>

$$
\begin{aligned}
    &\tan{\frac{\varphi}{2}}=\frac{\left|CD\right|}{\left|OC\right|}                                                  \\
    &\left|OC\right|=\widehat{OC}\cdot\overrightarrow{OE}                                                             \\
    &\left|CD\right|=\left(\widehat{OC}\cdot\overrightarrow{OE}\right)\tan{\frac{\varphi}{2}}
\end{aligned}
$$

Note that we are using a half of field of view angle. No magic here. It's literally on the picture above.

$\widehat{OC}$ is the *"forward"* vector of the camera, [unit vector](https://en.wikipedia.org/wiki/Unit_vector).

Please note that the $\left|CD\right|$ is equivalent to half the vertical pixel count $\left(h\right)$. So each pixel represents a physical length of $\eta$ units:

$$
\begin{aligned}
    &\eta=\dfrac{h\cdot\left|CD\right|}{2}                                                                            \\
    &\eta=\dfrac{h\left(\widehat{OC}\cdot\overrightarrow{OE}\right)\tan{\frac{\varphi}{2}}}{2}
\end{aligned}
$$

The formula above could be simplified. $\tan{\frac{\varphi}{2}}$ and $\dfrac{h}{2}$ could be precomputed and combined:

$$
\begin{aligned}
    &\alpha=\dfrac{h\tan{\frac{\varphi}{2}}}{2}                                                                       \\
    &\eta=\alpha\left(\widehat{OC}\cdot\overrightarrow{OE}\right)
\end{aligned}
$$

[↬ table of content ⇧](#table-of-content)

## <a id="decal-case">_SDF_ decal in _3D_ space case</a>

[↬ table of content ⇧](#table-of-content)
