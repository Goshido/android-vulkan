# Data compression

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Final result_](#final-results)
- [_Benchmark environment_](#benchmark)
- [_Optimization 1 - Quaternion, float16, separate position data_](#optimization-1)

## <a id="brief">Brief</a>

Page is written in 2024 November 12<sup>th</sup>. After almost 4 years from project start it was decided to review data structures for handling vertex data. The point of interest is to send data in more compact form to better utilize hardware caches and reduce overall bandwidth.

[↬ table of content ⇧](#table-of-content)

## <a id="final-results">Final result</a>

TBD

[↬ table of content ⇧](#table-of-content)

## <a id="benchmark">Benchmark environment</a>

Testing device: [_Redmi Note 8 Pro_](https://vulkan.gpuinfo.org/displayreport.php?id=12030).

Benchmark scenes:

⁘ **_PBR_**

<img src="./images/compression-pbr.png" width="800"/>

Average _FPS_: 56

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

Average _FPS_: 57.6

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

Average _FPS_: 105.5

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

## <a id="optimization-1">Optimization 1 - _Quaternion_, float16, separate position data</a>

_FPS_ comparison with stock version:

**Scene** | **_FPS_** | **Absolute difference** | **Relative difference** | **Preview**
--- | --- | --- | --- | ---
_PBR_ | 59.9 | 3.9⏫ | 7.1%⏫ | <img src="./images/compression-pbr.png" width="100">
_Skeletal mesh_ | 64.5 | 6.9⏫ | 11.9%⏫ | <img src="./images/compression-skeletal.png" width="100">
_World 1-1_ | 105.4 | 0.1🔻 | 0.09%🔻 | <img src="./images/compression-world1x1.png" width="100">

Quaternion could represent local-view rotation and composite orientation for influence bones during skinning process. Computations are performed in `float16_t` for _TBN_ data. The bitangent is reconstructed using [_cross product_](https://en.wikipedia.org/wiki/Cross_product).

Separate position data utilizes tile _GPU_ specifics: every vertex shader is implicily cut into two versions:

- only computations which affect output vertex position
- rest computations

So having positions and matrices tightly packed together increases chances to get cache-hit.

⁘ **_3D_ scene workflow**

Vertex buffer layouts:

```cpp
// Vertex buffer #0
struct VertexInfo final
{                                           // Vertex buffer #0
    GXVec3      _vertex;                    GXVec3          _position;

                                            // Vertex buffer #1
                                            struct VertexInfo final
                            ----------->    {
    GXVec2      _uv;                            GXVec2      _uv;
    GXVec3      _normal;                        GXVec3      _normal;
    GXVec3      _tangent;                       GXVec3      _tangent;
    GXVec3      _bitangent;                     GXVec3      _bitangent;
}                                           }
```

Uniform buffer layouts:

```cpp
                                                            struct ColorData final
            ----------------------------------->            {
                                                                GXColorRGB      _color0;
                                                                GXColorRGB      _color1;
struct ObjectData final                                         GXColorRGB      _color2;
{                                                               GXColorRGB      _emission;
    GXQuat          _localViewQuat;                         };
    GXMat4          _localViewProjection;
    GXColorRGB      _color0;                                // Uniform buffer #0 (vertex stage)
    GXColorRGB      _color1;                                struct InstancePositionData final
    GXColorRGB      _color2;                                {
    GXColorRGB      _emission;                                  GXMat4          _localViewProj[ MAX_INSTANCE_COUNT ];
};                                                          };

// Uniform buffer #0 (vertex, fragment stages)              // Uniform buffer #1 (vertex stage)
struct InstanceData final                                   struct InstanceNormalData final
{                                                           {
    ObjectData      _instanceData[ MAX_INSTANCE_COUNT ];        GXQuat          _localView[ MAX_INSTANCE_COUNT ];
};                                                          };

                                                            // Uniform buffer #2 (frament stage)
                                                            struct InstanceColorData final
            ----------------------------------->            {
                                                                ColorData       _colorData[ MAX_INSTANCE_COUNT ];
                                                            };
```

---

⁘ **_UI_ workflow**

Vertex buffer layouts:

```cpp
// Vertex buffer #0
struct UIVertexInfo final
{                                           // Vertex buffer #0
    GXVec2          _vertex;                GXVec2              _position;

                                            // Vertex buffer #1
                                ------->    struct UIVertex final
                                            {
    GXColorRGB      _color;                     GXColorRGB      _color;
    GXVec3          _atlas;                     GXVec3          _atlas;
    GXVec2          _imageUV;                   GXVec2          _imageUV;
};                                          };
```
[↬ table of content ⇧](#table-of-content)
