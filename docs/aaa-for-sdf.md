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
    &\eta=\left|CD\right|\div\dfrac{h}{2}                                                                            \\
    &\eta=\dfrac{2\cdot\left|CD\right|}{h}                                                                            \\
    &\eta=\dfrac{2\left(\widehat{OC}\cdot\overrightarrow{OE}\right)\tan{\frac{\varphi}{2}}}{h}
\end{aligned}
$$

The equation above could be simplified. $\tan{\frac{\varphi}{2}}$ and $\dfrac{2}{h}$ could be precomputed and combined:

$$
\begin{aligned}
    &\alpha=\dfrac{2\tan{\frac{\varphi}{2}}}{h}                                                                       \\
    &\eta=\alpha\left(\widehat{OC}\cdot\overrightarrow{OE}\right)
\end{aligned}
$$

Another runtime optimization happens around dot product:

$$
\overrightarrow{a}\cdot\overrightarrow{b}=a_xb_x+a_yb_y+a_zb_z
$$

Imagine that $\overrightarrow{a}$ is scaled by $\lambda$:

$$
\begin{aligned}
    &\lambda\overrightarrow{a}\cdot\overrightarrow{b}=\lambda{a_x}b_x+\lambda{a_y}b_y+\lambda{a_z}b_z                 \\
    &\lambda\overrightarrow{a}\cdot\overrightarrow{b}=\lambda\left(a_xb_x+a_yb_y+a_zb_z\right)                        \\
    &\lambda\overrightarrow{a}\cdot\overrightarrow{b}=\lambda\left(\overrightarrow{a}\cdot\overrightarrow{b}\right)
\end{aligned}
$$

It's exactly the same scenario which happens with $\eta$. So it's possible to precompute $\alpha$ and $\widehat{OC}$ as $\overrightarrow{v}$. Pay attention that $\overrightarrow{v}$ is a **uniform constant** in fact. This will simplify whole equation to single dot product:

$$
\begin{aligned}
    &\overrightarrow{v}=\dfrac{2\tan{\frac{\varphi}{2}}}{h}\widehat{OC}                                               \\
    &\eta=\overrightarrow{v}\cdot\overrightarrow{OE}                                                                  \\
    &                                                                                                                 \\
    &\text{where:}                                                                                                    \\
    &\eta - \text{pixel size in scene units}                                                                          \\
    &h - \text{vertical pixel count}                                                                                  \\
    &\varphi - \text{camera field of view angle}                                                                      \\
    &\widehat{OC} - \text{camera forward direction, unit vector}                                                      \\
    &\overrightarrow{OE} - \text{SDF ray vector}                                                                      \\

\end{aligned}
$$

<img src="./images/pixel-size.svg"/>

[↬ table of content ⇧](#table-of-content)

## <a id="decal-case">_SDF_ decal in _3D_ space case</a>

[↬ table of content ⇧](#table-of-content)
