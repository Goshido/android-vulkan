# Proper _UI_

## <a id="table-of-content">Table of content</a>

- [_Proper blending_](#blending)
- [_Proper text rendering_](#text)
- [_Benchmarking_](#benchmark)
- [_Conclusion_](#conclusion)

## <a id="blending">Proper blending</a>

Let's start from the issue:

<img src="./images/ui-wrong-blending.png"/>

You could notice than _Google Chrome_ much darker that _Android-Vulkan_. Caption area is a good example that blending looks different. Same time any opaque area has exactly the same color in both impementations. For example close button color is 1 to 1 match. Accoriding to [_CSS_](https://www.w3.org/TR/SVG11/masking.html#SimpleAlphaBlending) it should be used _premultiplied alpha_ or classic alpha blending. One blending could be transformed into another. _premultiplied alpha_ has advantages in image filtering and it allows to separate blending areas for later compositing.

It turns out that the answer is `UNORM` format for render target. Hardware produces slightly different results for `UNORM` and `sRGB` during blending. In the case above the render target format is `sRGB`. After changing it to `UNORM` the blending itself starts to match:

<img src="./images/ui-correct-blending.png"/>

The bending equation is classical:

```cpp
...

.blendEnable = VK_TRUE,
.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
.colorBlendOp = VK_BLEND_OP_ADD,
.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
.alphaBlendOp = VK_BLEND_OP_ADD,

.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT |
    VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT |
    VK_COLOR_COMPONENT_A_BIT

...
```

[↬ table of content ⇧](#table-of-content)

## <a id="text">Proper text rendering</a>

Let's start from the issue:

<img src="./images/ui-incorrect-text-blending.png"/>

The text looks thinner in _Android-Vulkan_ implementation.

On the first though it could be subpixel rendering on the _Window_. It has impact on appearance of course. But I assure you that this is not the main contribution to the visuals.

It should be some mapping from:

- Glyph luma
- Color alpha

<img src="./images/ui-color-alpha-luma-idea.png"/>

After some amount of experiments such mapping has been found. Here is final result:

<img src="./images/ui-correct-text-blending.png"/>

Let's look how this mapping is looks like first:

<img src="./images/ui-text-alpha-mapping-001.png"/>

<img src="./images/ui-text-alpha-mapping-002.png"/>

<img src="./images/ui-text-alpha-mapping-003.png"/>

<img src="./images/ui-text-alpha-mapping-004.png"/>

<img src="./images/ui-text-alpha-mapping-005.png"/>

Full video link is [here](./videos/ui-text-alpha-mapping.mp4).

This fancy mapping is constructed by three cubic [Bezier curves](https://en.wikipedia.org/wiki/B%C3%A9zier_curve).

[↬ table of content ⇧](#table-of-content)

## <a id="benchmark">Benchmarking</a>

<img src="./images/text-benchmark.png" width="800"/>

⁘ Scene complexity

**Metric** | **Submitted** | **Rendered** | **Culled**
--- | --- | --- | ---
Vertices | 10182 | 1392 | 87%
Opaque meshes | 70 | 13 | 82%
Stipple meshes | 4 | 0 | 100%
Point lights | 0 | 0 | N/A
Local reflections | 0 | 0 | N/A
Global reflections | 0 | 0 | N/A
_UI_ vertices | 35520 | 35520 | 0%

⁘ Performance

**Method** | **Frame time**
--- | ---
Lossless _LUT_ | 14.651 ms
Compressed _ASTC6x6 LUT_ | 14.671 ms
Newton approximation | 17.967 ms

[↬ table of content ⇧](#table-of-content)

## <a id="conclusion">Conclusion</a>

_LUT_ approach shows better performance compare to iterative _Newton_ approximation of two Bezier curves. Lossless and compressed _LUT_ show similar performance on real hardware. So it's decided to stick with _Lossless LUT_.

So:

- Swapchain format has been changed to `UNORM`
- It was added 3 types of primitives at pixel shader level: text, images and geometry
- _LUT_ is faster then _Newton_ approximation by 18.5%🟢
- Uncompress _LUT_ is universal solution (_ASTC_ is not widely supported on _PC_)

[↬ table of content ⇧](#table-of-content)
