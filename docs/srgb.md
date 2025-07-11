# _sRGB_

## <a id="table-of-content">Table of content</a>

- [_sRGB to Linear_](#srgb-to-linear)
- [_Linear to sRGB_](#linear-to-srgb)
- [_Brightness for sRGB_](#new-gamma)

## <a id="srgb-to-linear">_sRGB_ to _Linear_</a>

According to this [source](https://entropymine.com/imageworsener/srgbformula/) we have this equation:

$$
C_{linear}=
\begin{cases}
    \dfrac{C_{srgb}}{12.92} & 0 \leq C_{srgb} \leq 4.045 \times 10^{-2} \\
\\
    \left(\dfrac{C_{srgb} + 5.5 \times 10^{-2}}{1.055}\right)^{2.4} & 4.045 \times 10^{-2} \lt C_{srgb} \leq 1
\end{cases}
$$

Both divisions by $12.92$ and $1.055$ could be replaced by multiplications:

$$
C_{linear}=
\begin{cases}
    7.74 \times 10^{-2} \cdot C_{srgb} & 0 \leq C_{srgb} \leq 4.045 \times 10^{-2} \\
\\
    \left({9.477 \times 10^{-1} \cdot C_{srgb} + 5.213 \times 10^{-2}}\right)^{2.4} & 4.045 \times 10^{-2} \lt C_{srgb} \leq 1
\end{cases}
$$

[↬ table of content ⇧](#table-of-content)

## <a id="linear-to-srgb">_Linear_ to _sRGB_</a>

According to this [source](https://entropymine.com/imageworsener/srgbformula/) we have this equation:

$$
C_{srgb}=
\begin{cases}
    12.92 \cdot C_{linear} & 0 \leq C_{linear} \leq 3.1308 \times 10^{-3} \\
\\
    1.055 \cdot {C_{linear}}^{\frac{1}{2.4}} - 5.5 \times 10^{-2} & 3.1308 \times 10^{-3} \lt C_{linear} \leq 1
\end{cases}
$$

$\frac{1}{2.4}$ could be precomputed:

$$
C_{srgb}=
\begin{cases}
    12.92 \cdot C_{linear} & 0 \leq C_{linear} \leq 3.1308 \times 10^{-3} \\
\\
    1.055 \cdot {C_{linear}}^{4.1667 \times 10^{-1}} - 5.5 \times 10^{-2} & 3.1308 \times 10^{-3} \lt C_{linear} \leq 1
\end{cases}
$$

[↬ table of content ⇧](#table-of-content)

## <a id="new-gamma">Brightness for _sRGB_</a>

The idea is to raise color in _Linear_ space in some power. Then move result into _sRGB_ space.

Note if original color is in _sRGB_ space it's needed to move it into _Linear_ space first. Such situation happens in _UI_ pass for example.

It was decided to use power ($P$) which lies in range $[0.2, 5]$.

From user perspective it is controlled by brightness balance ($B$) which lies in range $[-1, 1]$.

Value | Meaning
--- | ---
-1 | darker
1 | brighter

This formula describes connection between power and brightness balance:

$$P = 5^{-B}$$

[↬ table of content ⇧](#table-of-content)
