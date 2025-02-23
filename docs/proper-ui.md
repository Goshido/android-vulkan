# Proper _UI_

## <a id="table-of-content">Table of content</a>

- [_Proper blending_](#blending)
- [_Proper text rendering_](#text)
- [_Benchmarking_](#benchmark)
- [_Conclusion_](#conclusion)

## <a id="blending">Proper blending</a>

Let's start from the issue demonstration:



[↬ table of content ⇧](#table-of-content)

## <a id="text">Proper text rendering</a>

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
