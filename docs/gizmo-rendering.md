# Gizmo rendering

## <a id="table-of-content">Table of content</a>

- [_Analytical Anti-Aliasing for SDF_](#aaa)
  - [_Anti-Aliasing zone_](#aa-zone)
  - [_Pixel coverage_](#pixel-coverage)
  - [_Efficient ray-marching threshold_](#threshold)
  - [_Final result_](#result)
- [_Shell extend_](#shell-extend)
- [_SDF simplifications_](#simplification)
  - [_Line segment_](#line-segment)
- [_Known limitations_](#limitations)

## <a id="aaa">Analytical Anti-Aliasing for _SDF_</a>

The core principle of anti-aliasing Signed Distance Fields (_SDF_) is the analytical estimation of how close a given pixel's center is to the shape's boundary. The objective is to determine an accurate sub-pixel coverage value for pixels that straddle the edge of the shape.
The "loop of pixels" that forms the boundary in the final image is the critical area of focus.

[↬ table of content ⇧](#table-of-content)

## <a id="aa-zone">Anti-Aliasing zone</a>

The anti-aliasing algorithm assigns an alpha value to each pixel within this transition zone:

- Alpha approaches 1: when the pixel's center is extremely close to, or just inside, the shape's boundary.
- Alpha approaches 0: when the pixel's center is far from the boundary.

This analytical method uses the _SDF_ value itself, scaled by the estimated pixel size, to smoothly interpolate between full transparency and full opacity, effectively simulating a precise sub-pixel fill without resorting to costly supersampling.

<img src="./images/sdf-aaa-loop.svg"/>

[↬ table of content ⇧](#table-of-content)

### <a id="pixel-coverage">Pixel coverage</a>

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
    &\eta=\left|CD\right|\div\dfrac{h}{2}                                                                             \\
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
    &\overrightarrow{OE} - \textit{SDF}\text{ ray vector}                                                             \\
\end{aligned}
$$

<img src="./images/pixel-size.svg"/>

At this point, a rule can be established to map _SDF_ proximity to the pixel center and transparency:

- Fully Transparent: If the _SDF_ shape proximity is greater than the pixel size relative to the pixel center.
- Fully Opaque: If the _SDF_ shape touches the pixel center.
- In all other cases, apply linear interpolation clamped between 0 and 1 to determine the final alpha.

⚠️ Anti-aliasing breaks when world to _SDF_ coordinate transformations involve scaling. Pay extra attention to $\widehat{OC}$ normalization when computing $\overrightarrow{v}$.

[↬ table of content ⇧](#table-of-content)

### <a id="threshold">Efficient ray-marching threshold</a>

When ray-marching _SDF_, many tutorials rely on a "magic constant" — typically `0.0005` — as a proximity threshold for hit detection. However, since distant objects occupy fewer pixels, a fixed threshold is often inefficient or imprecise. To maintain visual consistency, the threshold should be dynamically calculated based on the camera's field of view and the distance the ray has traveled.

The following picture shows pixel coverage for 3 zones during ray-marching:

<img src="./images/sdf-threshold.svg"/>

Distant _SDF_ regions are less sensitive to threshold precision, whereas areas near the camera require much higher accuracy. The image below highlights the artifacts caused by using a naive fixed threshold:

<img src="./images/fixed-threshold-issue.svg"/>

To solve this, we should use a threshold that adjusts based on pixel coverage at every step. This threshold is naturally tied to the desired alpha quantization. Using 8-bit alpha as a baseline, the algorithm defines a 'hit' when the distance to the _SDF_ surface is smaller than the ratio of pixel coverage to the available discrete alpha values.

Let’s examine the iteration distribution. It is important to note that pixels missing the _SDF_ shape are not included in this data. Note, those pixels generally reach an iteration count of `30`.

<img src="./images/fixed-vs-dynamic-threshold.svg"/>

The number of alpha quantization levels clearly affects performance. Since this is highly dependent on the specific profiling environment, it is up to the individual developer to choose the configuration that best fits their needs.

[↬ table of content ⇧](#table-of-content)

## <a id="result">Final result</a>

<img src="./images/aaa-sdf.svg"/>

The reference shader implementation:

```cpp
// 1 / 255
#define ALPHA_8_BIT     3.92156e-3F

// From https://iquilezles.org/articles/distfunctions/
float32_t SDFLineSegment ( in float32_t3 p, in float32_t3 a, in float32_t3 b, in float32_t r )
{
    float32_t3 const pa = p - a;
    float32_t3 const ba = b - a;
    float32_t const h = saturate ( dot ( pa, ba ) / dot ( ba, ba ) );
    return length ( mad ( ba, -h, pa ) ) - r;
}

float32_t LinearStep ( in float32_t step, in float32_t x )
{
    const float32_t s = -step;
    return saturate((x + s) / s);
}

//----------------------------------------------------------------------------------------------------------------------

float32_t4 PS(in VertexToPixel inputData,
    in float32_t4 color,
    in float32_t3 segmentA,
    in float32_t3 segmentB,
    in float32_t segmentRadius,
    in float32_t maxDistance,
    in uint32_t maxSteps
)
{
    float32_t3 const ray = normalize ( inputData._canvas - inputData._camera );

    // precomputing part of dot product due to dot product property: dot(S * a, b) = S * dot(a, b)
    float32_t const pixelScale = dot ( ray, inputData._vi );

    // x - current distance from SDF
    // y - maximum allowed distance (camera far plane)
    float32_t2 alpha = float32_t2 ( 0.0F, maxDistance );

    // x - adjustable minimal distance to consider ray vs SDF hit
    // y - ray distance has traveled
    float32_t2 beta = (float32_t2)0.0F;

    // x - closest distance detected
    // y - closest ray length corresponding closest distance detected
    float32_t2 closest = (float32_t2)maxDistance;

    float32_t const dynamicThresholdFactor = pixelScale * ALPHA_8_BIT;

    for ( uint32_t steps = 0U; steps < maxSteps; ++steps )
    {
        alpha.x = SDFLineSegment ( mad ( ray, beta.y, inputData._camera ), segmentA, segmentB, segmentRadius );
        closest = lerp ( closest, float32_t2 ( alpha.x, beta.y ), closest.x > alpha.x );
        beta.y += alpha.x;
        beta.x = beta.y * dynamicThresholdFactor;

        if ( any ( alpha < beta ) )
        {
            break;
        }
    }

    const float32_t insideProbe = SDFLineSegment (
        mad ( ray, mad ( pixelScale, beta.y, closest.y ), inputData._camera ),
        segmentA,
        segmentB,
        segmentRadius
    );

    const float32_t2 cases = float32_t2 (
        // inside SDF shape
        color.w,

        // AA loop or outside SDF shape
        color.w * LinearStep ( closest.y * pixelScale, closest.x )
    );

    return float32_t4 ( color.xyz, cases[ (uint32_t)( insideProbe >= 0.0F ) ] );
}
```

[↬ table of content ⇧](#table-of-content)

## <a id="shell-extend">Shell extend</a>

By default there is some camera angles where anti-aliasing is missing:

<img src="./images/too-small-shell.svg">

This issue occurs because the _SDF_ shell geometry lacks sufficient space to accommodate the pixel loop’s anti-aliasing transition zone. To resolve this, the shell geometry must be inflated using the following process:

- Calculate Pixel Size $\eta$: Determine the specific pixel size at each corner of the _SDF_ shell geometry using [_formula above_](#pixel-coverage).
- Apply Inflation Factor $\omega$: Use these calculated values as the inflation factor for each corresponding corner.
- Define Directionality: Ensure the corner data explicitly defines the direction of inflation.

The following image illustrates this concept applied to a box-shaped shell:

<img src="./images/shell-extend.svg">

The value $\omega$ represents the length of the longest diagonal (the space diagonal) of a cube with a side length equal to the pixel size $\eta$:

$$
\begin{aligned}
    &\omega=\eta\sqrt{3}
\end{aligned}
$$

<img src="./images/shell-extend-result.svg">

⚠️ **Crucial Note**: Ensure that the _SDF_ shell inflation direction remains independent of any scaling transformations. The inflation must ignore the object's scale to work properly.

[↬ table of content ⇧](#table-of-content)

## <a id="simplification">_SDF_ simplifications</a>

[↬ table of content ⇧](#table-of-content)

### <a id="line-segment">Line segment</a>

The original formula was taken from [_iquilezles.org_](https://iquilezles.org/articles/distfunctions/)

```cpp
float32_t sdCapsule ( in float32_t3 p, in float32_t3 a, in float32_t3 b, float32_t r )
{
    float32_t3 const pa = p - a;
    float32_t3 const ba = b - a;
    float32_t const h = clamp ( dot ( pa, ba ) / dot ( ba, ba ), 0.0F, 1.0F );
    return length ( pa - ba * h ) - r;
}
```

Let's optimize it for X-axis line segment with half length $e$ and center at the origin.

$$
\begin{aligned}
    &a=\left(-e,0,0\right)                                                                                            \\
    &b=\left(e,0,0\right)                                                                                             \\
    &p=\left(p_x, p_y, p_z\right)                                                                                     \\
    &pa=\left(p_x-\left(-e\right),p_y,p_z\right)=\left(p_x+e,p_y,p_z\right)                                           \\
    &ba=\left(e-\left(-e\right),0,0\right)=\left(2e, 0, 0\right)                                                      \\
    &pa\cdot{ba}=2e\left(p_x+e\right)                                                                                 \\
    &ba\cdot{ba}=4e^2                                                                                                 \\
    &\text{------------}                                                                                              \\
    &h=saturate\left(\frac{2e\left(p_x+e\right)}{4e^2}\right)=saturate\left(\frac{p_x+e}{2e}\right)                   \\
    &h=saturate\left(\frac{p_x}{2e}+\frac{1}{2}\right)                                                                \\
    &\phi=\frac{1}{2e}                                                                                                \\
    &h=saturate\left({\phi}p_x+\frac{1}{2}\right)                                                                     \\
    &\alpha=mad\left(ba,-h,pa\right)=\left(-2he+p_x+e,p_y,p_z\right)                                                  \\
    &\alpha=\left(p_x+e\right(1-2h\left),p_y,p_z\right)                                                               \\
    &\beta=p_x+e\left(1-2h\right)                                                                                     \\
    &\left|\alpha\right|=\sqrt{{\beta}^2+{p_y}^2+{p_z}^2}                                                             \\
    &SDF=\sqrt{{\beta}^2+{p_y}^2+{p_z}^2}-r                                                                           \\
    &\text{------------}                                                                                              \\
    &\phi=\frac{1}{2e}                                                                                                \\
    &h=saturate\left({\phi}p_x+\frac{1}{2}\right)=saturate\left(mad\left(\phi, p_x, \frac{1}{2}\right)\right)         \\
    &\beta=p_x+e-2he=p_x+mad\left(h,-2e,e\right)                                                                      \\
    &\gamma=-2e                                                                                                       \\
    &\beta=p_x+mad\left(h,\gamma,e\right)                                                                             \\
    &SDF=\left|\left(\beta,p_y,p_z\right)\right|-r                                                                    \\
    &\text{------------}                                                                                              \\
    &\phi=\frac{1}{2e}                                                                                                \\
    &\gamma=-2e                                                                                                       \\
    &\lambda=-r                                                                                                       \\
    &h=saturate\left(mad\left(p_x,\phi,\frac{1}{2}\right)\right)                                                      \\
    &SDF=\lambda+\left|\left(p_x+mad\left(h,\gamma,e\right),p_y,p_z\right)\right|                                     \\
\end{aligned}
$$

[↬ table of content ⇧](#table-of-content)

## <a id="limitations">Known limitations</a>

Avoid self-intersecting _SDF_ shapes with differing colors. These areas lose analytical anti-aliasing, leading to jagged edges.

<img src="./images/sdf-limitation-001.svg">

ℹ️ Suggestion: For the best visual quality, consider reassembling your geometry to eliminate overlaps.

[↬ table of content ⇧](#table-of-content)
