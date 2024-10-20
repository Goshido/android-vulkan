# _sRGB_

## <a id="table-of-content">Table of content</a>

- [_Linear _RGB_ to sRGB_](#linear-to-srgb)
- [_sRGB to Linear RGB_](#srgb-to-linear)
- [_New gamma value for sRGB_](#new-gamma)

## <a id="linear-to-srgb">Linear _RGB_ to _sRGB_</a>

Accoring to this [source](https://entropymine.com/imageworsener/srgbformula/) we have this equation:

$$
RGB_{linear}=
\begin{cases}
    \dfrac{RGB_{srgb}}{12.92} & 0 \leq RGB_{srgb} \leq 4.045 \times 10^{-2} \\
\\
    \left(\dfrac{RGB_{srgb} + 5.5 \times 10^{-2}}{1.055}\right)^{2.4} & 4.045 \times 10^{-2} \lt RGB_{srgb} \leq 1
\end{cases}
$$

Both divisions by $12.92$ and $1.055$ could be replaced by multiplications:

$$
RGB_{linear}=
\begin{cases}
    7.74 \times 10^{-2} \cdot RGB_{srgb} & 0 \leq RGB_{srgb} \leq 4.045 \times 10^{-2} \\
\\
    \left({9.477 \times 10^{-1} \cdot RGB_{srgb} + 5.213 \times 10^{-2}}\right)^{2.4} & 4.045 \times 10^{-2} \lt RGB_{srgb} \leq 1
\end{cases}
$$

[↬ table of content ⇧](#table-of-content)

## <a id="srgb-to-linear">_sRGB_ to Linear _RGB_</a>

Accoring to this [source](https://entropymine.com/imageworsener/srgbformula/) we have this equation:

$$
RGB_{srgb}=
\begin{cases}
    12.92 \cdot RGB_{linear} & 0 \leq RGB_{linear} \leq 3.1308 \times 10^{-3} \\
\\
    1.055 \cdot {RGB_{linear}}^{\frac{1}{2.4}} - 5.5 \times 10^{-2} & 3.1308 \times 10^{-3} \lt RGB_{linear} \leq 1
\end{cases}
$$

[↬ table of content ⇧](#table-of-content)

## <a id="new-gamma">New gamma value for _sRGB_</a>

The idea is to move _sRGB_ value to Linear _RGB_ and move to _sRGB_ with new gamma. Analyzing two equation above it's possible to see that for _"linear"_ range we have:

$$ 4.045 \times 10^{-2} \Longrightarrow RGB_{linear}$$

will produce

$$ 3.1308 \times 10^{-3} \Longrightarrow RGB_{srgb} $$

And vice versa.

To deal with _"exponential"_ range let's substitute Linear _RGB_ into _sRGB_ equation. Assume new gamma as $\gamma$:

$$ 1.055 \cdot \left(\left({9.477 \times 10^{-1} \cdot RGB_{srgb} + 5.213 \times 10^{-2}}\right)^{2.4}\right)^{\frac{1}{\gamma}} - 5.5 \times 10^{-2} $$

Raising power to another power could be simplified:

$$ 1.055 \cdot \left({9.477 \times 10^{-1} \cdot RGB_{srgb} + 5.213 \times 10^{-2}}\right)^{\frac{2.4}{\gamma}} - 5.5 \times 10^{-2} $$

And combining all together we have:

$$
RGB_{srgb_{new}}=
\begin{cases}
    RGB_{srgb_{old}} & 0 \leq RGB_{srgb_{old}} \leq 4.045 \times 10^{-2} \\
\\
    1.055 \cdot \left({9.477 \times 10^{-1} \cdot RGB_{srgb_{old}} + 5.213 \times 10^{-2}}\right)^{\frac{2.4}{\gamma}} - 5.5 \times 10^{-2} & 4.045 \times 10^{-2} \lt RGB_{srgb_{old}} \leq 1
\end{cases}
$$

[↬ table of content ⇧](#table-of-content)
