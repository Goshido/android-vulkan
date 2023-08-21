# _SPD_ algorithm

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Optimizations for average brightness calculation_](#average-brightness-optimizations)
- [_Stock implementation patterns_](#stock-patterns)
  - [_Mip 0_](#mip-0)
    - [_Thread mapping pattern_](#mip-0-thread-mappings)
    - [_Shared memory status_](#mip-0-shared-memory)
  - [_Mip 1_](#mip-1)
    - [_Thread mapping pattern_](#mip-1-thread-mappings)
    - [_Shared memory status_](#mip-1-shared-memory)
  - [_Mip 2_](#mip-2)
    - [_Thread mapping pattern_](#mip-2-thread-mappings)
    - [_Shared memory status_](#mip-2-shared-memory)
  - [_Mip 3_](#mip-3)
    - [_Thread mapping pattern_](#mip-3-thread-mappings)
    - [_Shared memory status_](#mip-3-shared-memory)
  - [_Mip 4_](#mip-4)
    - [_Thread mapping pattern_](#mip-4-thread-mappings)
    - [_Shared memory status_](#mip-4-shared-memory)
  - [_Mip 5_](#mip-5)
    - [_Thread mapping pattern_](#mip-5-thread-mappings)
    - [_Shared memory status_](#mip-5-shared-memory)

## <a id="brief">Brief</a>

_SPD_ means single pass downsampler. Algorithm is published by _AMD_ [here](https://gpuopen.com/fidelityfx-spd/). It uses compute shader to build full mip chain using single pass compute shader.

To syncronize whole process algorithm uses:

- shared memory
- [`GroupMemoryBarrierWithGroupSync` barriers](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/groupmemorybarrierwithgroupsync)
- special _UAV_ view for image's internal mip 5 which has [`globallycoherent`](https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#7.14.4%20Global%20vs%20Group/Local%20Coherency%20on%20Non-Atomic%20UAV%20Reads) specifier
- global atomic counter in _VRAM_ with [`globallycoherent`](https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#7.14.4%20Global%20vs%20Group/Local%20Coherency%20on%20Non-Atomic%20UAV%20Reads) specifier

Limitations for _Vulkan_:

- input image must be square
- input image resolution should be power of two
- maximum input image resolution `4096`

Original implementation is universal. Because of that there are a lot of branching statements to stop mip chain generation process.

[↬ table of content ⇧](#table-of-content)

## <a id="average-brightness-optimizations">Optimizations for average brightness calculation</a>

Final goal is a single value: average brightness. So intermidiate mip levels do not matter for algorithm. This allows to remove all unnecessary memory traffic with storing intermedite mip maps **except mip 5 of course**. Mip 5 is special because it syncronizes workng groups via [`globallycoherent`](https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#7.14.4%20Global%20vs%20Group/Local%20Coherency%20on%20Non-Atomic%20UAV%20Reads) specifier. Keep in mind that most of the operations happen in shared memory. Removing storing operations into global _VRAM_ is a big optimization for original algorithm.

Resulting average brightness will be stored in storage buffer rather than last mip level of the image.

Square and non power of two limitations could be resolved. This requires to modify compute shader. First it's resonable to add requirement for input image to be multiple of 64 pixels. This allows to avoid bound checking up to internal mip 5. For internal mip level greater that 5 it's needed to do bound checking before writing to mip images.

To reduce big amount of branching it's resonable to have several modifications of the algorithm. Main difference between versions - compatibility with fixed amout of mip levels:

- for 12 mips: 2048 - 4095 resolution range
- for 11 mips: 1024 - 2047 resolution range
- for 10 mips: 512 - 1023 resolution range

Last mip level pass does not need to write into shared memory.

For brightness computation it's possible to convert image to luma using [_BT.601_](https://en.wikipedia.org/wiki/Luma_(video)#Rec._601_luma_versus_Rec._709_luma_coefficients) at generation of internal mip 0. This allows to use `float16_t` instead of `float16_t4` for shared memory. This also allows to use `VK_FORMAT_R16_SFLOAT` format for final image.

[↬ table of content ⇧](#table-of-content)

## <a id="stock-patterns">Stock implementation patterns</a>

### <a id="mip-0">Mip 0</a>

#### <a id="mip-0-thread-mappings">Thread mapping pattern</a>

256 threads:

```txt
[00 00] [01 00] [00 01] [01 01] [00 02] [01 02] [00 03] [01 03] [02 00] [03 00] [02 01] [03 01] [02 02] [03 02] [02 03] [03 03]
[04 00] [05 00] [04 01] [05 01] [04 02] [05 02] [04 03] [05 03] [06 00] [07 00] [06 01] [07 01] [06 02] [07 02] [06 03] [07 03]
[00 04] [01 04] [00 05] [01 05] [00 06] [01 06] [00 07] [01 07] [02 04] [03 04] [02 05] [03 05] [02 06] [03 06] [02 07] [03 07]
[04 04] [05 04] [04 05] [05 05] [04 06] [05 06] [04 07] [05 07] [06 04] [07 04] [06 05] [07 05] [06 06] [07 06] [06 07] [07 07]
[08 00] [09 00] [08 01] [09 01] [08 02] [09 02] [08 03] [09 03] [10 00] [11 00] [10 01] [11 01] [10 02] [11 02] [10 03] [11 03]
[12 00] [13 00] [12 01] [13 01] [12 02] [13 02] [12 03] [13 03] [14 00] [15 00] [14 01] [15 01] [14 02] [15 02] [14 03] [15 03]
[08 04] [09 04] [08 05] [09 05] [08 06] [09 06] [08 07] [09 07] [10 04] [11 04] [10 05] [11 05] [10 06] [11 06] [10 07] [11 07]
[12 04] [13 04] [12 05] [13 05] [12 06] [13 06] [12 07] [13 07] [14 04] [15 04] [14 05] [15 05] [14 06] [15 06] [14 07] [15 07]
[00 08] [01 08] [00 09] [01 09] [00 10] [01 10] [00 11] [01 11] [02 08] [03 08] [02 09] [03 09] [02 10] [03 10] [02 11] [03 11]
[04 08] [05 08] [04 09] [05 09] [04 10] [05 10] [04 11] [05 11] [06 08] [07 08] [06 09] [07 09] [06 10] [07 10] [06 11] [07 11]
[00 12] [01 12] [00 13] [01 13] [00 14] [01 14] [00 15] [01 15] [02 12] [03 12] [02 13] [03 13] [02 14] [03 14] [02 15] [03 15]
[04 12] [05 12] [04 13] [05 13] [04 14] [05 14] [04 15] [05 15] [06 12] [07 12] [06 13] [07 13] [06 14] [07 14] [06 15] [07 15]
[08 08] [09 08] [08 09] [09 09] [08 10] [09 10] [08 11] [09 11] [10 08] [11 08] [10 09] [11 09] [10 10] [11 10] [10 11] [11 11]
[12 08] [13 08] [12 09] [13 09] [12 10] [13 10] [12 11] [13 11] [14 08] [15 08] [14 09] [15 09] [14 10] [15 10] [14 11] [15 11]
[08 12] [09 12] [08 13] [09 13] [08 14] [09 14] [08 15] [09 15] [10 12] [11 12] [10 13] [11 13] [10 14] [11 14] [10 15] [11 15]
[12 12] [13 12] [12 13] [13 13] [12 14] [13 14] [12 15] [13 15] [14 12] [15 12] [14 13] [15 13] [14 14] [15 14] [14 15] [15 15]
```

#### <a id="mip-0-shared-memory">Shared memory status</a>

```txt
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    0
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    1
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    2
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    3
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    4
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    5
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    6
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    7
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    8
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    9
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    10
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    11
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    12
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    13
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    14
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    15
```

[↬ table of content ⇧](#table-of-content)

### <a id="mip-1">Mip 1</a>

#### <a id="mip-1-thread-mappings">Thread mapping pattern</a>

64 threads:

```txt
[00 00] [01 00] [00 01] [01 01] [00 02] [01 02] [00 03] [01 03]
[02 00] [03 00] [02 01] [03 01] [02 02] [03 02] [02 03] [03 03]
[04 00] [05 00] [04 01] [05 01] [04 02] [05 02] [04 03] [05 03]
[06 00] [07 00] [06 01] [07 01] [06 02] [07 02] [06 03] [07 03]
[00 04] [01 04] [00 05] [01 05] [00 06] [01 06] [00 07] [01 07]
[02 04] [03 04] [02 05] [03 05] [02 06] [03 06] [02 07] [03 07]
[04 04] [05 04] [04 05] [05 05] [04 06] [05 06] [04 07] [05 07]
[06 04] [07 04] [06 05] [07 05] [06 06] [07 06] [06 07] [07 07]
```

#### <a id="mip-1-shared-memory">Shared memory status</a>

```txt
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    0
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    1
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    2
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    3
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    4
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    5
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    6
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    7
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    8
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    9
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    10
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    11
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    12
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    13
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    14
✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅✅    15
```

[↬ table of content ⇧](#table-of-content)

### <a id="mip-2">Mip 2</a>

#### <a id="mip-2-thread-mappings">Thread mapping pattern</a>

64 threads:

```txt
[00 00] [01 00] [00 01] [01 01] [00 02] [01 02] [00 03] [01 03]
[02 00] [03 00] [02 01] [03 01] [02 02] [03 02] [02 03] [03 03]
[04 00] [05 00] [04 01] [05 01] [04 02] [05 02] [04 03] [05 03]
[06 00] [07 00] [06 01] [07 01] [06 02] [07 02] [06 03] [07 03]
[00 04] [01 04] [00 05] [01 05] [00 06] [01 06] [00 07] [01 07]
[02 04] [03 04] [02 05] [03 05] [02 06] [03 06] [02 07] [03 07]
[04 04] [05 04] [04 05] [05 05] [04 06] [05 06] [04 07] [05 07]
[06 04] [07 04] [06 05] [07 05] [06 06] [07 06] [06 07] [07 07]
```

#### <a id="mip-2-shared-memory">Shared memory status</a>

```txt
✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥    0
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    1
🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅    2
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    3
✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥    4
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    5
🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅    6
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    7
✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥    8
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    9
🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅    10
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    11
✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥    12
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    13
🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅🟥✅    14
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    15
```

[↬ table of content ⇧](#table-of-content)

### <a id="mip-3">Mip 3</a>

#### <a id="mip-3-thread-mappings">Thread mapping pattern</a>

16 threads:

```txt
[00 00] [01 00] [00 01] [01 01]
[00 02] [01 02] [00 03] [01 03]
[02 00] [03 00] [02 01] [03 01]
[02 02] [03 02] [02 03] [03 03]
```

#### <a id="mip-3-shared-memory">Shared memory status</a>

```txt
✅🟥🟥🟥✅🟥🟥🟥✅🟥🟥🟥✅🟥🟥🟥    0
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    1
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    2
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    3
🟥✅🟥🟥🟥✅🟥🟥🟥✅🟥🟥🟥✅🟥🟥    4
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    5
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    6
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    7
🟥🟥✅🟥🟥🟥✅🟥🟥🟥✅🟥🟥🟥✅🟥    8
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    9
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    10
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    11
🟥🟥🟥✅🟥🟥🟥✅🟥🟥🟥✅🟥🟥🟥✅    12
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    13
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    14
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    15
```

[↬ table of content ⇧](#table-of-content)

### <a id="mip-4">Mip 4</a>

#### <a id="mip-4-thread-mappings">Thread mapping pattern</a>

4 threads:

```txt
[00 00] [01 00]
[00 01] [01 01]
```

#### <a id="mip-4-shared-memory">Shared memory status</a>

```txt
✅✅✅✅🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    0
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    1
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    2
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    3
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    4
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    5
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    6
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    7
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    8
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    9
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    10
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    11
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    12
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    13
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    14
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    15
```

[↬ table of content ⇧](#table-of-content)

### <a id="mip-5">Mip 5</a>

#### <a id="mip-5-thread-mappings">Thread mapping pattern</a>

1 thread:

```txt
[00 00]
```

#### <a id="mip-5-shared-memory">Shared memory status</a>

```txt
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    0
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    1
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    2
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    3
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    4
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    5
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    6
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    7
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    8
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    9
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    10
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    11
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    12
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    13
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    14
🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥🟥    15
```

[↬ table of content ⇧](#table-of-content)
