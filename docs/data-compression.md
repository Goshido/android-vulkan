# Data compression

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Final result_](#final-results)
- [_Benchmark environment_](#benchmark)
- [_Optimizations #1_](#optimizations-1)
  - [_Quaternion and `float16_t`_](#opt-1-quat-float16)
  - [_Separate position data_](#opt-1-separate-positions)
- [_Optimizations #2_](#optimizations-2)
  - [_Compressing TBN in 4 bytes_](#opt-2-tbn-in-4-bytes)
  - [_Storing local-view information in 8 bytes_](#opt-2-local-view-8-bytes)
  - [_Using `uint16_t` index buffers_](#opt-2-using-uint16-indices)
  - [_Further vertex data compression_](#opt-2-further-compression)
- [_Optimization #3_](#optimization-3)
  - [_UV coordinates as `float16_t2`_](#opt-3-uv-as-float16)
- [_Optimizations #4_](#optimizations-4)
  - [_Using UMA_](#opt-4-uma)
  - [_Further UI vertex compression_](#opt-4-ui-compression)
- [_Optimization #5_](#optimization-5)
  - [_Proper UI_](#opt-5-proper-ui)
- [_Optimization #6_](#optimization-6)
  - [_Using more UMA_](#opt-6-more-uma)

## <a id="brief">Brief</a>

Page was written in 2024 November 21<sup>th</sup>. After almost 4 years from project start it was decided to review data structures for handling vertex data. The point of interest is to send data in more compact form to better utilize hardware caches and reduce overall bandwidth.

[↬ table of content ⇧](#table-of-content)

## <a id="final-results">Final result</a>

It was not detected any visible quality degradation on benchmark scenes.

⁘ Frame time:

**Scene** | **Stock** | **Optimized** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | --- | ---
_PBR_ | 17.846 ms | 16.661 ms | -1.185 ms🟢 | -6.6%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 17.344 ms | 15.055 ms | -2.289 ms🟢 | -13.2%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 9.477 ms | 10.504 ms | +1.027🔺 | +10.8%🔺 | <img src="./images/compression-world1x1.png" width="100">

⁘ Maximum instances:

**Stock** | **Optimized** | **Absolute difference** | **Relative difference**
--- | --- | --- | ---
42 | 84 | +42🟢 | +100%🟢

⁘ Bytes per scene vertex:

**Stock** | **Optimized** | **Absolute difference** | **Relative difference**
--- | --- | --- | ---
248 | 108 | -140🟢 | -56.5%🟢

⁘ Bytes per _UI_ vertex:

**Stock** | **Optimized** | **Absolute difference** | **Relative difference**
--- | --- | --- | ---
50 | 18 | -32🟢 | -64%🟢

[↬ table of content ⇧](#table-of-content)

## <a id="benchmark">Benchmark environment</a>

Testing device: [_Redmi Note 8 Pro_](https://vulkan.gpuinfo.org/displayreport.php?id=12030).

Benchmark scenes:

⁘ **_PBR_**

<img src="./images/compression-pbr.png" width="800"/>

Frame time: 17.846 ms, _vsync_ off - `VK_PRESENT_MODE_MAILBOX_KHR`

**Metric** | **Submitted** | **Rendered** | **Culled**
--- | --- | --- | ---
Vertices | 718173 | 444954 | 39%
Opaque meshes | 417 | 243 | 42%
Stipple meshes | 2 | 1 | 50%
Point lights | 1 | 1 |0%
Local reflections | 0 | 0 | N/A
Global reflections | 1 | 1 | 0%
_UI_ vertices | 0 | 0 | N/A

---

⁘ **_Skeletal mesh_**

<img src="./images/compression-skeletal.png" width="800"/>

Frame time: 17.344 ms, _vsync_ off - `VK_PRESENT_MODE_MAILBOX_KHR`

**Metric** | **Submitted** | **Rendered** | **Culled**
--- | --- | --- | ---
Vertices | 64674 | 64674 | 0%
Opaque meshes | 2 | 2 | 0%
Stipple meshes | 0 | 0 | N/A
Point lights | 2 | 2 | 0%
Local reflections | 0 | 0 | N/A
Global reflections | 0 | 0 | N/A
_UI_ vertices | 0 | 0 | N/A

---

⁘ **_World 1-1_**

<img src="./images/compression-world1x1.png" width="800"/>

Frame time: 9.477 ms, _vsync_ off - `VK_PRESENT_MODE_MAILBOX_KHR`

**Metric** | **Submitted** | **Rendered** | **Culled**
--- | --- | --- | ---
Vertices | 10182 | 1392 | 87%
Opaque meshes | 70 | 13 | 82%
Stipple meshes | 4 | 0 | 100%
Point lights | 0 | 0 | N/A
Local reflections | 0 | 0 | N/A
Global reflections | 0 | 0 | N/A
_UI_ vertices | 288 | 288 | 0%

[↬ table of content ⇧](#table-of-content)

## <a id="optimizations-1">Optimizations #1</a>

⁘ Frame time comparison with stock version:

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.669 ms | -1.177 ms🟢 | -6.6%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.496 ms | -1.847 ms🟢 | -10.7%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 9.485 ms | +0.008 ms🔺 | +0.1%🔺 | <img src="./images/compression-world1x1.png" width="100">

---

⁘ Maximum instances comparison with stock version:

**Optimizations #1** | **Absolute difference** | **Relative difference**
--- | --- | ---
56 | +14🟢 | +33.3%🟢

---

⁘ Bytes per scene vertex comparison with stock version:

**Optimizations #1** | **Absolute difference** | **Relative difference**
--- | --- | ---
200 | -48🟢 | -19.4%🟢

---

⁘ Bytes per _UI_ vertex comparison with stock version:

**Optimizations #1** | **Absolute difference** | **Relative difference**
--- | --- | ---
50 | 0 | 0%

[↬ table of content ⇧](#table-of-content)

### <a id="opt-1-quat-float16">_Quaternion_ and `float16_t`</a>

Quaternion could represent local-view rotation and composite orientation for influence bones during skinning process. Computations are performed in `float16_t` for _TBN_ data. The _bitangent_ is reconstructed using [_cross product_](https://en.wikipedia.org/wiki/Cross_product).

[↬ table of content ⇧](#table-of-content)

### <a id="opt-1-separate-positions">Separate position data</a>

Separate position data utilizes tile _GPU_ specifics: every vertex shader is implicitly cut into two versions:

- only computations which affect output vertex position
- rest computations

So having positions and matrices tightly packed together increases chances to get cache-hit.

⁘ **_3D_ scene workflow**

Vertex buffer layout:

```cpp
// Vertex buffer #0
struct VertexInfo
{                                           // Vertex buffer #0
    float32_t3      _vertex;                float32_t3          _position;

                                            // Vertex buffer #1
                                            struct VertexInfo
                            ----------->    {
    float32_t2      _uv;                        float32_t2      _uv;
    float32_t3      _normal;                    float32_t3      _normal;
    float32_t3      _tangent;                   float32_t3      _tangent;
    float32_t3      _bitangent;                 float32_t3      _bitangent;
}                                           }
```

Uniform buffer layout:

```cpp
                                                            struct ColorData
            ----------------------------------->            {
                                                                float32_t4      _color0;
                                                                float32_t4      _color1;
struct ObjectData                                               float32_t4      _color2;
{                                                               float32_t4      _emission;
    float32_t4x4    _localView;                             };
    float32_t4x4    _localViewProjection;
    float32_t4      _color0;                                // Uniform buffer #0 (vertex stage)
    float32_t4      _color1;                                struct InstancePositionData
    float32_t4      _color2;                                {
    float32_t4      _emission;                                  float32_t4x4    _localViewProj[ 56U ];
};                                                          };

// Uniform buffer #0 (vertex, fragment stages)              // Uniform buffer #1 (vertex stage)
struct InstanceData                                         struct InstanceNormalData
{                                                           {
    ObjectData      _instanceData[ 42U ];                       float32_t4      _localView[ 56U ];
};                                                          };

                                                            // Uniform buffer #2 (frament stage)
                                                            struct InstanceColorData
            ----------------------------------->            {
                                                                ColorData       _colorData[ 56U ];
                                                            };
```

---

⁘ **_UI_ workflow**

Vertex buffer layout:

```cpp
// Vertex buffer #0
struct UIVertexInfo
{                                           // Vertex buffer #0
    float32_t2      _vertex;                float32_t2          _position;

                                            // Vertex buffer #1
                                ------->    struct UIVertex
                                            {
    float32_t4      _color;                     float32_t4      _color;
    float32_t3      _atlas;                     float32_t3      _atlas;
    float32_t2      _imageUV;                   float32_t2      _imageUV;
};                                          };
```
[↬ table of content ⇧](#table-of-content)

## <a id="optimizations-2">Optimizations #2</a>

⁘ Frame time comparison with stock version:

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.667 ms | -1.178 ms🟢 | -6.6%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.136 ms | -2.207 ms🟢 | -12.7%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 9.519 ms | +0.042 ms🔺 | +0.4%🔺 | <img src="./images/compression-world1x1.png" width="100">

⁘ Frame time comparison with _Optimizations #1_:

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.667 ms | -0.001 ms🟢 | -0.008%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.136 ms | -0.36 ms🟢 | -2.3%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 9.519 ms | +0.033 ms🔺 | +0.3%🔺 | <img src="./images/compression-world1x1.png" width="100">

---

⁘ Maximum instances comparison with stock version:

**Optimizations #2** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | +42🟢 | +100%🟢

⁘ Maximum instances comparison with _Optimizations #1_:

**Optimizations #2** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | +28🟢 | +50%🟢

---

⁘ Bytes per scene vertex comparison with stock version:

**Optimizations #2** | **Absolute difference** | **Relative difference**
--- | --- | ---
112 | -136🟢 | -54.8%🟢

⁘ Bytes per scene vertex comparison with _Optimizations #1_:

**Optimizations #2** | **Absolute difference** | **Relative difference**
--- | --- | ---
112 | -88🟢 | -44%🟢

---

⁘ Bytes per _UI_ vertex comparison with stock/_Optimizations #1_ versions:

**Optimizations #2** | **Absolute difference** | **Relative difference**
--- | --- | ---
32 | -18🟢 | -36%🟢

[↬ table of content ⇧](#table-of-content)

### <a id="opt-2-tbn-in-4-bytes">Compressing _TBN_ in 4 bytes</a>

For orthogonal _TBNs_ it's possible to represent it via unit-quaternion plus information about mirroring of the _bitangent_ vector.

First step is to make sure that _TBN_ is orthogonal. Unfortunately _3ds Max_ provides non-orthogonal _TBNs_. Good news that there is de-facto industry standard convention/library called [_Mikkt_](https://github.com/mmikk/MikkTSpace). This library provides orthogonal _tangents_ using mesh _normals_ and _UVs_.

Second step is further data compression. Quaternion is 4 numbers. Mirroring information is single number. So it's needed 5 numbers. Good news that rendering system is using unit quaternions. So it's possible to store 3 components and recover 4<sup>th</sup> component using formula:

$$r = \sqrt{1 - a^2 - b^2 - c^2}$$

But there is a catch. Fundamental flaw of quaternion: duality. That means that exact same rotation is always described by two quaternions:

$$
q = r + ai + bj + ck
$$
$$
-q = -r - ai - bj - ck
$$

To solve this issue the implementation will always select quaternion with nonnegative $r$ component. Good news this selection can be done offline at asset exporting stage.

Last issue comes from computations in `float16_t` precision. Sometimes square root will be negative due to rounding errors. Taking square root from negative value will produce [_NaN_](https://en.wikipedia.org/wiki/NaN) on _GPU_. To solve this issue the recovering process uses absolute value under square root:

$$r = \sqrt{|1 - a^2 - b^2 - c^2|}$$

And last step is optimal data format: `VK_FORMAT_A2R10G10B10_UNORM`:

- $a$ → _R_ 10 bits
- $b$ → _G_ 10 bits
- $c$ → _B_ 10 bits
- _mirroring_ scalar → _A_ 2 bits

[↬ table of content ⇧](#table-of-content)

### <a id="opt-2-local-view-8-bytes">Storing local-view information in 8 bytes</a>

_TBN_ must be rotated by _local-view_ matrix for proper shading of the 3D model. This matrix is also orthogonal and could be represented by quaternion. It was decided to use more precise compression such so every component of the quaternion is described by 16 bits. So encoding one _local-view_ matrix requires 8 bytes. The catch is passing this information via uniform buffers. Such data must be aligned by 16 byte boundary. The solution is to store two quaternions instead of one. Together they occupy 16 bytes:

```cpp
struct TBN64
{
    uint32_t    _q0High;
    uint32_t    _q0Low;
    uint32_t    _q1High;
    uint32_t    _q1Low;
};

```

[↬ table of content ⇧](#table-of-content)

### <a id="opt-2-using-uint16-indices">Using `uint16_t` index buffers</a>

When the number of unique vertices of the mesh is less than or equal to $2^{16}$ or 65536 it's possible to use `uint16_t` type for index buffers.

[↬ table of content ⇧](#table-of-content)

### <a id="opt-2-further-compression">Further vertex data compression</a>

The storing color information in `float32_t4` is suboptimal. It's possible to store it in `uint32_t` instead. The catch is emission which must use intensity scaler which allows to use values beyond [0.0, 1.0] range.

⁘ **_3D_ scene workflow**

Vertex buffer layout:

```cpp
// Vertex buffer #0
struct VertexInfo
{                                           // Vertex buffer #0
    float32_t3      _vertex;                float32_t3          _position;

                                            // Vertex buffer #1
                                            struct VertexInfo
                            ----------->    {
    float32_t2      _uv;                        float32_t2      _uv;
    float32_t3      _normal;                    uint32_t        _tbn;
    float32_t3      _tangent;               };
    float32_t3      _bitangent;
}
```

Uniform buffer layout:

```cpp
                                                            struct TBN64
                                                            {
                                                                uint32_t        _q0High;
                                                                uint32_t        _q0Low;
                                                                uint32_t        _q1High;
                                                                uint32_t        _q1Low;
                                                            };

                                                            struct ColorData
            ----------------------------------->            {
                                                                uint32_t        _emiRcol0rgb;
                                                                uint32_t        _emiGcol1rgb;
struct ObjectData                                               uint32_t        _emiBcol2rgb;
{                                                               uint32_t        _col0aEmiIntens;
    float32_t4x4    _localView;                             };
    float32_t4x4    _localViewProjection;
    float32_t4      _color0;                                // Uniform buffer #0 (vertex stage)
    float32_t4      _color1;                                struct InstancePositionData
    float32_t4      _color2;                                {
    float32_t4      _emission;                                  float32_t4x4    _localViewProj[ 84U ];
};                                                          };

// Uniform buffer #0 (vertex, fragment stages)              // Uniform buffer #1 (vertex stage)
struct InstanceData                                         struct InstanceNormalData
{                                                           {
    ObjectData      _instanceData[ 42U ];                       TBN64           _localView[ 84U / 2U ];
};                                                          };

                                                            // Uniform buffer #2 (frament stage)
                                                            struct InstanceColorData
            ----------------------------------->            {
                                                                ColorData       _colorData[ 84U ];
                                                            };
```

---

⁘ **_UI_ workflow**

Vertex buffer layout:

```cpp
// Vertex buffer #0
struct UIVertexInfo
{                                           // Vertex buffer #0
    float32_t2      _vertex;                float32_t2          _position;

                                            struct UIAtlas
                                            {
                                                float32_t2      _uv;
                                                uint8_t         _layer;
                                                uint8_t         _padding[ 3U ];
                                            };

                                            // Vertex buffer #1
                                ------->    struct UIVertex
                                            {
    float32_t4      _color;                     float32_t2      _image;
    float32_t3      _atlas;                     UIAtlas         _atlas;
    float32_t2      _imageUV;                   uint32_t        _color;
};                                          };
```

**Note:** 2024 November 30<sup>th</sup>. According to _Vertex Input Extraction_ from _Vulkan_ spec vertex element must be aligned by 4 bytes. Having vertex element non-multiple of 4 bytes causes runtime artefacts on [_XIAOMI Redmi Note 8 Pro_](https://vulkan.gpuinfo.org/displayreport.php?id=12030) looking like data race or missing barrier or so. _VVL 1.3.299_ does not detect any core or sync validation issues. Running same code on [_NVIDIA RTX 4080_](https://vulkan.gpuinfo.org/displayreport.php?id=34593) does not have any artifacts. To solve the issue it was added 3 byte padding after `UIAtlas::_layer` field.

[↬ table of content ⇧](#table-of-content)

## <a id="optimization-3">Optimization #3</a>

⁘ Frame time comparison with stock version:

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.662 ms | -1.184 ms🟢 | -6.6%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.142 ms | -2.202 ms🟢 | -12.7%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 9.585 ms | +0.108 ms🔺 | +1.1%🔺 | <img src="./images/compression-world1x1.png" width="100">

⁘ Frame time comparison with _Optimizations #2_:

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.662 ms | -0.005 ms🟢 | -0.03%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.142 ms | +0.006 ms🔺 | +0.04%🔺 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 9.585 ms | +0.066 ms🔺 | +0.69%🔺 | <img src="./images/compression-world1x1.png" width="100">

---

⁘ Maximum instances comparison with stock version:

**Optimization #3** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | +42🟢 | +100%🟢

⁘ Maximum instances comparison with _Optimizations #2_:

**Optimization #3** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | 0 | 0%

---

⁘ Bytes per scene vertex comparison with stock version:

**Optimization #3** | **Absolute difference** | **Relative difference**
--- | --- | ---
108 | -140🟢 | -56.5%🟢

⁘ Bytes per scene vertex comparison with _Optimizations #2_:

**Optimization #3** | **Absolute difference** | **Relative difference**
--- | --- | ---
108 | -4🟢 | -3.6%🟢

---

⁘ Bytes per _UI_ vertex comparison with stock version:

**Optimization #3** | **Absolute difference** | **Relative difference**
--- | --- | ---
24 | -26🟢 | -52%🟢

⁘ Bytes per _UI_ vertex comparison with _Optimizations #2_:

**Optimization #3** | **Absolute difference** | **Relative difference**
--- | --- | ---
24 | -8🟢 | -25%🟢

[↬ table of content ⇧](#table-of-content)

### <a id="opt-3-uv-as-float16">_UV_ coordinates as `float16_t2`</a>

The most critical part is text rendering which must be pixel perfect. In order to achieve that it was decided to set text atlas resolution to 1024x1024 which is fit perfectly to `float16_t` precision.

⁘ **_3D_ scene workflow**

Vertex buffer layout:

```cpp
// Vertex buffer #0
struct VertexInfo
{                                           // Vertex buffer #0
    float32_t3      _vertex;                float32_t3          _position;

                                            // Vertex buffer #1
                                            struct VertexInfo
                            ----------->    {
    float32_t2      _uv;                        float16_t2      _uv;
    float32_t3      _normal;                    uint32_t        _tbn;
    float32_t3      _tangent;               };
    float32_t3      _bitangent;
}
```

Uniform buffer layout:

```cpp
                                                            struct TBN64
                                                            {
                                                                uint32_t        _q0High;
                                                                uint32_t        _q0Low;
                                                                uint32_t        _q1High;
                                                                uint32_t        _q1Low;
                                                            };

                                                            struct ColorData
            ----------------------------------->            {
                                                                uint32_t        _emiRcol0rgb;
                                                                uint32_t        _emiGcol1rgb;
struct ObjectData                                               uint32_t        _emiBcol2rgb;
{                                                               uint32_t        _col0aEmiIntens;
    float32_t4x4    _localView;                             };
    float32_t4x4    _localViewProjection;
    float32_t4      _color0;                                // Uniform buffer #0 (vertex stage)
    float32_t4      _color1;                                struct InstancePositionData
    float32_t4      _color2;                                {
    float32_t4      _emission;                                  float32_t4x4    _localViewProj[ 84U ];
};                                                          };

// Uniform buffer #0 (vertex, fragment stages)              // Uniform buffer #1 (vertex stage)
struct InstanceData                                         struct InstanceNormalData
{                                                           {
    ObjectData      _instanceData[ 42U ];                       TBN64           _localView[ 84U / 2U ];
};                                                          };

                                                            // Uniform buffer #2 (frament stage)
                                                            struct InstanceColorData
            ----------------------------------->            {
                                                                ColorData       _colorData[ 84U ];
                                                            };
```

---

⁘ **_UI_ workflow**

Vertex buffer layout:

```cpp
// Vertex buffer #0
struct UIVertexInfo
{                                           // Vertex buffer #0
    float32_t2      _vertex;                float32_t2          _position;

                                            struct UIAtlas
                                            {
                                                float16_t2      _uv;
                                                float32_t       _layer;
                                            };

                                            // Vertex buffer #1
                                ------->    struct UIVertex
                                            {
    float32_t4      _color;                     float16_t2      _image;
    float32_t3      _atlas;                     UIAtlas         _atlas;
    float32_t2      _imageUV;                   uint32_t        _color;
};                                          };
```

**Note:** 2024 December 13<sup>th</sup>. Turns out that `Adreno 730` does not support `VK_FORMAT_R8_USCALED` for vertex buffers at all. So it was decided to make `UIAtlas::_layer` field as `float32_t` because `VK_FORMAT_R32_SFLOAT` is widely supported for vertex buffers. At vertex shader side this value must be used as `float32_t` and aligned to 4-byte boundary anyway. As a bonus `UIAtlas::_padding` field is not needed anymore.

[↬ table of content ⇧](#table-of-content)

## <a id="optimizations-4">Optimizations #4</a>

⁘ Frame time comparison with stock version:

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.645 ms | -1.2 ms🟢 | -6.7%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.07 ms | -2.273 ms🟢 | -13.1%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 9.576 ms | +0.099 ms🔺 | +1.05%🔺 | <img src="./images/compression-world1x1.png" width="100">

⁘ Frame time comparison with _Optimization #3_:

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.645 ms | -0.017 ms🟢 | -0.1%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.07 ms | -0.072 ms🟢 | -0.5%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 9.576 ms | -0.009 ms🟢 | -0.1%🟢 | <img src="./images/compression-world1x1.png" width="100">

---

⁘ Maximum instances comparison with stock version:

**Optimizations #4** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | +42🟢 | +100%🟢

⁘ Maximum instances comparison with _Optimization #3_:

**Optimizations #4** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | 0 | 0%

---

⁘ Bytes per scene vertex comparison with stock version:

**Optimizations #4** | **Absolute difference** | **Relative difference**
--- | --- | ---
108 | -140🟢 | -56.5%🟢

⁘ Bytes per scene vertex comparison with _Optimization #3_:

**Optimizations #4** | **Absolute difference** | **Relative difference**
--- | --- | ---
108 | 0 | 0%

---

⁘ Bytes per _UI_ vertex comparison with stock version:

**Optimizations #4** | **Absolute difference** | **Relative difference**
--- | --- | ---
22 | -28🟢 | -56%🟢

⁘ Bytes per _UI_ vertex comparison with _Optimization #3_:

**Optimizations #4** | **Absolute difference** | **Relative difference**
--- | --- | ---
22 | -2🟢 | -8.3%🟢

[↬ table of content ⇧](#table-of-content)

### <a id="opt-4-uma">Using _UMA_</a>

_Android_ devices mostly use unified memory architecture (_UMA_). It could be used for uniform buffers and storage buffers. This significantly simplifies _Vulkan_ code and reduces memory traffic:

- `vkCmdPipelineBarrier` for transfer operations are removed
- no copy overhead related to staging buffers
- no implicit copy overhead related to `vkCmdUpdateBuffer`
- upload operations do not require `VkCommandBuffer` handle

[↬ table of content ⇧](#table-of-content)

### <a id="opt-4-ui-compression">Further _UI_ vertex compression</a>

The `UIAtlas::_layer` was changed from `float32_t` to `float16_t` type.

⁘ **_UI_ workflow**

Vertex buffer layout:

```cpp
// Vertex buffer #0
struct UIVertexInfo
{                                           // Vertex buffer #0
    float32_t2      _vertex;                float32_t2          _position;

                                            struct UIAtlas
                                            {
                                                float16_t2      _uv;
                                                float16_t       _layer;
                                            };

                                            // Vertex buffer #1
                                ------->    struct UIVertex
                                            {
    float32_t4      _color;                     float16_t2      _image;
    float32_t3      _atlas;                     UIAtlas         _atlas;
    float32_t2      _imageUV;                   uint32_t        _color;
};                                          };
```

[↬ table of content ⇧](#table-of-content)

## <a id="optimization-5">Optimization #5</a>

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.911 ms | -0.934 ms🟢 | -5.2%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.301 ms | -2.043 ms🟢 | -11.8%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 11.152 ms | +1.675 ms🔺 | +17.7%🔺 | <img src="./images/compression-world1x1.png" width="100">

⁘ Frame time comparison with _Optimization #4_:

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.911 ms | +0.267 ms🔺 | +1.6%🔺 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.301 ms | +0.231 ms🔺 | +1.5%🔺 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 11.152 ms | +1.576 ms🔺 | +16.5%🔺 | <img src="./images/compression-world1x1.png" width="100">

---

⁘ Maximum instances comparison with stock version:

**Optimizations #5** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | +42🟢 | +100%🟢

⁘ Maximum instances comparison with _Optimization #4_:

**Optimizations #5** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | 0 | 0%

---

⁘ Bytes per scene vertex comparison with stock version:

**Optimizations #5** | **Absolute difference** | **Relative difference**
--- | --- | ---
108 | -140🟢 | -56.5%🟢

⁘ Bytes per scene vertex comparison with _Optimization #4_:

**Optimizations #5** | **Absolute difference** | **Relative difference**
--- | --- | ---
108 | 0 | 0%

---

⁘ Bytes per _UI_ vertex comparison with stock version:

**Optimizations #5** | **Absolute difference** | **Relative difference**
--- | --- | ---
18 | -32🟢 | -64%🟢

⁘ Bytes per _UI_ vertex comparison with _Optimization #4_:

**Optimizations #5** | **Absolute difference** | **Relative difference**
--- | --- | ---
18 | -4🟢 | -18.2%🟢

[↬ table of content ⇧](#table-of-content)

### <a id="opt-5-proper-ui">Proper _UI_</a>

Proper _UI_ implementation required some refactoring. The changes are described [here](./proper-ui.md) in more details.

⁘ **_UI_ workflow**

Vertex buffer layout:

```cpp
// Vertex buffer #0
struct UIVertexInfo
{                                           // Vertex buffer #0
    float32_t2      _vertex;                float32_t2          _position;

                                            // Vertex buffer #1
                                ------->    struct UIVertex
                                            {
    float32_t4      _color;                     float16_t2      _uv;
    float32_t3      _atlas;                     uint8_t         _atlasLayer;
    float32_t2      _imageUV;                   uint8_t         _primitiveType;
                                                uint32_t        _color;
};                                          };
```

[↬ table of content ⇧](#table-of-content)

## <a id="optimization-6">Optimization #6</a>

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.661 ms | -1.185 ms🟢 | -6.6%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.055 ms | -2.289 ms🟢 | -13.2%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 10.504 ms | +1.027🔺 | +10.8%🔺 | <img src="./images/compression-world1x1.png" width="100">

⁘ Frame time comparison with _Optimization #5_:

**Scene** | **Frame time** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 16.661 ms | -0.25 ms🟢 | -1.5%🟢 | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 15.055 ms | -0.246 ms🟢 | -1.6%🟢 | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 10.504 ms | -0.647 ms🟢 | -5.8%🟢 | <img src="./images/compression-world1x1.png" width="100">

---

⁘ Maximum instances comparison with stock version:

**Optimization #6** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | +42🟢 | +100%🟢

⁘ Maximum instances comparison with _Optimization #5_:

**Optimization #6** | **Absolute difference** | **Relative difference**
--- | --- | ---
84 | 0 | 0%

---

⁘ Bytes per scene vertex comparison with stock version:

**Optimization #6** | **Absolute difference** | **Relative difference**
--- | --- | ---
108 | -140🟢 | -56.5%🟢

⁘ Bytes per scene vertex comparison with _Optimization #5_:

**Optimization #6** | **Absolute difference** | **Relative difference**
--- | --- | ---
108 | 0 | 0%

---

⁘ Bytes per _UI_ vertex comparison with stock version:

**Optimization #6** | **Absolute difference** | **Relative difference**
--- | --- | ---
18 | -32🟢 | -64%🟢

⁘ Bytes per _UI_ vertex comparison with _Optimization #5_:

**Optimization #6** | **Absolute difference** | **Relative difference**
--- | --- | ---
18 | 0 | 0%🟢

[↬ table of content ⇧](#table-of-content)

### <a id="opt-6-more-uma">Using more _UMA_</a>

_Android_ devices mostly use unified memory architecture (_UMA_). It could be used for index, vertex and storage buffers. This significantly simplifies _Vulkan_ code and allows zero-copy patterns:

- `vkCmdPipelineBarrier` for transfer operations are removed
- no copy overhead related to staging buffers
- upload operations do not require `VkCommandBuffer` handle

Previously the following systems used _PC_ approach with staging buffers, transfer command and additional pipeline barrier to make sure that data was uploaded:

- static meshes
- skeletal meshes
- _UI_ rendering

Now all systems above were refactored to _UMA_ usage.

[↬ table of content ⇧](#table-of-content)
