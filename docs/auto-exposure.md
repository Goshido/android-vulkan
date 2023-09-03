# Auto-exposure

## <a id="table-of-content">Table of content</a>

- [_Source_](#source)
- [_Implementation_](#implementation)

## <a id="source">Source</a>

Implementation is based on paper [_Automatic Exposure by Krzysztof Narkowicz, January 9, 2016_](https://knarkowicz.wordpress.com/2016/01/09/automatic-exposure/) and [_BakingLab_ project](https://github.com/TheRealMJP/BakingLab).

[↬ table of content ⇧](#table-of-content)

## <a id="implementation">Implementation</a>

The math is build around fundamental photography term - [_EV_](https://en.wikipedia.org/wiki/Exposure_value), exposure value. The formula to get [_EV_](https://en.wikipedia.org/wiki/Exposure_value) from luminance is the following:

$$EV = \log_2(8L)$$

End reverse formula:

$$L = \dfrac{2^{EV}}{8}$$

The idea behind average luminance of image is geometric mean. But not a strait forward. Rather that is using [_log2_ trick](https://en.wikipedia.org/wiki/Geometric_mean):

1. Convert all pixels of _HDR_ to log2 luminance using [_BT.709_](https://en.wikipedia.org/wiki/Relative_luminance#Relative_luminance_and_%22gamma_encoded%22_colorspaces):

$$l = log_2(dot(pixel_{xyz}, [0.2126, 0.7152, 0.0722]))$$

2. Make sure to not feed 0 values into _log2_! Clamp to minimum value:

$$log_2(\dfrac{0.25}{256}) = -10$$

3. Find arithmetic mean of such dataset

$$L_{log_2}$$

4. Uncompress arithmetic mean to get geometric mean:

$$L_{average} = 2^{L_{log_2}}$$

Now to calculate exposure scalar you need to apply the formula:

$$Exposure = \dfrac{KeyValue}{clamp(L_{current}, L_{min}, L_{max}) - L_{EC}}$$

Where:

$$L_{min}, L_{max} - \text{expected values of maximum and minimum brightness of scene. Scalars. Set up by artist.}$$

$$L_{EC} - \text{Scalar value which allows the artist to bias exposure. Calculated on CPU using the first formula.}$$

To make eye adaptation it's needed to use exponential decay function which should be applied to average luminance:

$$L_{current} = L_{previous} + (L_{average} - L_{previous}) \cdot (1 - e^{\Delta time \cdot \tau})$$

Where:

$$L_{previous} -\text{Average luminance computed on previous frame. Initially equals zero.}$$

$$\Delta time -\text{Frame time in seconds. Uniform value from CPU.}$$

$$\tau -\text{Speed of eye adaptation. Positive float. Artistic scalar. Uniform value from CPU.}$$

The article is using adaptive _KeyValue_ instead of using fixed [_middle gray_](https://en.wikipedia.org/wiki/Middle_gray) value of _18%_:

$$KeyValue = 1.03 - \dfrac{2}{log_{10}(L + 1) + 2}$$

It deals with room + curtain state problem.

[↬ table of content ⇧](#table-of-content)
