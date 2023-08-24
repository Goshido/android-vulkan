# Auto exposure

## <a id="table-of-content">Table of content</a>

- [_Source_](#source)
- [_Keynotes_](#keynotes)
- [_Stuff which is unclear_](#unclear-stuff)

## <a id="source">Source</a>

Implementation is based on paper [_Automatic Exposure by Krzysztof Narkowicz, January 9, 2016_](https://knarkowicz.wordpress.com/2016/01/09/automatic-exposure/).

[↬ table of content ⇧](#table-of-content)

## <a id="keynotes">Keynotes</a>

18% middle gray is closely related to photography [_EV_](https://en.wikipedia.org/wiki/Exposure_value), exposure values. The formula to get [_EV_](https://en.wikipedia.org/wiki/Exposure_value) from luminance is the following:

$$L = \dfrac{2^{EV}}{8}$$

The idea behing average luminance of image is geometric mean. But not a strait forward. Rather that is using [_log2_ trick](https://en.wikipedia.org/wiki/Geometric_mean):

1. Convert all pixels of _HDR_ to log2 luminance using [_BT.709_](https://en.wikipedia.org/wiki/Relative_luminance#Relative_luminance_and_%22gamma_encoded%22_colorspaces):

$$l = log2(dot(pixel_{xyz}, [0.2126, 0.7152, 0.0722])$$

2. Make sure to not feed 0 values into _log2_! Clamp to mimum value:

$$log2(\dfrac{0.25}{256}) = -10$$

3. Find arithmetic mean of such dataset

$$L_{log2}$$

4. Uncocompress arithmetic mean to get geometric mean:

$$L_{average} = exp2(L_{log2})$$

Now to calculate exposure scalar you need to apply the formula:

$$Exposure = \dfrac{0.18}{clamp(L_{average}, L_{min}, L_{max}) - L_{EC}}$$

Where:

$$L_{min}, L_{max} - \text{expected values of maximum and minimum brightness of scene. Set up by artist.}$$

$$L_{EC} - \text{Scalar controll which allows the artist to bias exposure. Calculated on CPU using the first formula.}$$

To make eye adaptation it's needed to use exponental decay function which should be applied to average luminance:

$$L_{current} = L_{previous} + (L_{average} - L_{previous}) \cdot (1 - e^{\Delta time \cdot \tau})$$

Where:

$$L_{previous} -\text{Average luminance computed on previous frame. Initially equals zero.}$$

$$\Delta time -\text{Frame time in seconds. Uniform value from CPU.}$$

$$\tau -\text{Speed of eye adoptation. Positive float. Artistic scalar. Uniform value from CPU.}$$

[↬ table of content ⇧](#table-of-content)

## <a id="unclear-stuff">Stuff which is unclear</a>

In the article there is one more formula:

$$KeyValue = 1.03 - \dfrac{2}{log10(L + 1) + 2}$$

It deals with room + curtain state problem. It's unclear where it should be applied it in the main formula:

$$Exposure = \dfrac{0.18}{clamp(L_{current}, L_{min}, L_{max}) - L_{EC}}$$

[↬ table of content ⇧](#table-of-content)
