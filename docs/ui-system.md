# _UI_ system

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_CSS Units_ and hardware _DPI_](#css-units-and-dpi)
- [_HTML constraints_](#html-constraints)
  - [_Head_](#head)
    - [_Link_](#link)
  - [_Body_ ](#body)
    - [_DIV_ ](#div)
    - [_IMG_ ](#img)
    - [_Text_ ](#text)
- [_CSS constraints_](#css-constraints)
  - [`@font-face`](#font-face)
  - [`backgroground-color`](#background-color)
  - [`background-size`](#background-size)
  - [`bottom`](#bottom)
  - [`left`](#left)
  - [`right`](#right)
  - [`top`](#top)
  - [`color`](#color)
  - [`display`](#display)
  - [`font-family`](#font-family)
  - [`font-size`](#font-size)
  - [`line-height`](#line-height)
  - [`margin-bottom`](#margin-bottom)
  - [`margin-left`](#margin-left)
  - [`margin-right`](#margin-right)
  - [`margin-top`](#margin-top)
  - [`margin`](#margin)
  - [`padding-bottom`](#padding-bottom)
  - [`padding-left`](#padding-left)
  - [`padding-right`](#padding-right)
  - [`padding-top`](#padding-top)
  - [`padding`](#padding)
  - [`position`](#position)
  - [`text-align`](#text-align)
  - [`vertical-align`](#vertical-align)
  - [`width`](#width)
  - [`height`](#height)

## <a id="brief">Brief</a>

Framework is using [_HTML_](https://en.wikipedia.org/wiki/HTML) + [_CSS_](https://en.wikipedia.org/wiki/CSS) files for _UI_ assets. The system very similar to _Web_-development. The main difference is _Lua_ scripting instead of _JavaScript_. It's needed to understand that frameworks is using subset of [_HTML_](https://en.wikipedia.org/wiki/HTML) + [_CSS_](https://en.wikipedia.org/wiki/CSS) entities as common sense. Such limited set of elements allows to create robust user interfaces without legacy complexity of modern _Web_ technology.

In addition framework provides [_HTML_ + _CSS_ validator](./html-validator.md). The validator could work in background and notify content creator about detected problems at exact file and line. Usually content creation workflow looks like this:

<img src="./images/html-validator.png"/>

Such approach allows to use powerful browser's development tools. For example [_Chrome DevTool_](https://developer.chrome.com/docs/devtools/overview/). Content creator could access such tools by drag
n'drop main _HTML_ file onto browser window and press _F12_ in case of [_Google Chrome_](https://www.google.com/chrome/).

In order to create _UI_ elements the content creator has to create [_UILayer_](./ui-layer.md) instance from _lua_ script.

[↬ table of content ⇧](#table-of-content)

## <a id="css-units-and-dpi">_CSS Units_ and hardware _DPI_</a>

Information is based on [_CSS specification_](https://www.w3.org/TR/css3-values/#absolute-lengths) and [practical manual](https://web.dev/high-dpi/). `px`, `pt` and `mm` will be measured in _device specific_ pixels.

$$DPI_{hardware} = \textit{device specific}$$

$$distance_{meters} = \textit{user preference}$$

$$\hspace{1cm}$$

$$DPI_{spec} = 96$$

$$distance_{spec} = 28 (\textit{inches})$$

$$meterToInch = 3.93701e+1$$

$$\hspace{1cm}$$

$$DPI_{ideal} = DPI_{spec} \cdot \dfrac{distance_{spec}}{distance_{comfort}}$$

$$distance_{comfort} = distance_{meters} \cdot meterToInch$$

$$DPI_{ideal} = DPI_{spec} \cdot \dfrac{distance_{spec}}{distance_{meters} \cdot meterToInch} = \dfrac{DPI_{spec} \cdot distance_{spec}}{distance_{meters} \cdot meterToInch}$$

$$px = \dfrac{DPI_{hardware}}{DPI_{ideal}} = \dfrac{DPI_{hardware} \cdot distance_{meters} \cdot meterToInch}{DPI_{spec} \cdot distance_{spec}}$$

$$DPI_{factor} = \dfrac{meterToInch}{DPI_{spec} \cdot distance_{spec}}$$

$$px = DPI_{hardware} \cdot distance_{meters} \cdot DPI_{factor}$$

---

$$inch = 96px$$

$$inch = 72pt$$

$$inch = 25.4mm$$


$$\hspace{1cm}$$

$$25.4mm = 96px$$

$$mm = \dfrac{96px}{25.4}$$

$$\hspace{1cm}$$

$$96px = 72pt$$

$$pt = \dfrac{96px}{72}$$

[↬ table of content ⇧](#table-of-content)

## <a id="html-constraints">_HTML_ constraints</a>

The _HTML_ document **MUST HAVE** the following structure:

```html
<!DOCTYPE html>
<html>
    <head>
        <link rel="stylesheet" href="...">
    </head>

    <body>
        ...
    </body>
</html>
```

_UI_ system only supports the following entities: [`head`](#head), [`link`](#link), [`body`](#body), [`div`](#div), [`img`](#img) and [`text`](#text).

[↬ table of content ⇧](#table-of-content)

### <a id="head">_Head_</a>

Supported child element: [`link`](#link).

[↬ table of content ⇧](#table-of-content)

### <a id="link">_Link_</a>

Current implementation supports **ONLY ONE** _link_ element.

Attribute | Constraint
--- | ---
`rel` | Must be `stylesheet`.
`href` | _UTF-8_ path to _CSS_ file. The path is relative from current _HTML_ file.

[↬ table of content ⇧](#table-of-content)

### <a id="body">_Body_</a>

Supported child elements: [`div`](#div), [`img`](#img) and [`text`](#text).

Supported attributes:

Attribute | Constraint
--- | ---
`id` | _UTF-8_ string. Must be unique. _CSS_ will be applied **AFTER** styles from `class` attribute.
`class` | _UTF-8_ string. Supports list of classes. _CSS_ will be applied in order of declaration of the classes and **BEFORE** styles from `id` attribute.

Default style:

Property | Value
--- | ---
[`background-color`](#background-color) | `transparent`
[`background-size`](#background-size) | `100%`
[`bottom`](#bottom) | `auto`
[`left`](#left) | `auto`
[`right`](#right) | `auto`
[`top`](#top) | `auto`
[`color`](#color) | `black`
[`display`](#display) | `block`
[`font-family`](#font-family) | `inherit`
[`font-size`](#font-size) | `16px`
[`line-height`](#line-height) | `normal`
[`margin-bottom`](#margin-bottom) | `8px`
[`margin-left`](#margin-left) | `8px`
[`margin-right`](#margin-right) | `8px`
[`margin-top`](#margin-top) | `8px`
[`padding-bottom`](#padding-bottom) | `0px`
[`padding-left`](#padding-left) | `0px`
[`padding-right`](#padding-right) | `0px`
[`padding-top`](#padding-top) | `0px`
[`position`](#position) | `static`
[`text-align`](#text-align) | `left`
[`vertical-align`](#vertical-align) | `top`
[`width`](#width) | `100%`
[`height`](#height) | `100%`

[↬ table of content ⇧](#table-of-content)

#### <a id="div">_DIV_</a>

Supported child elements: [`div`](#div), [`img`](#img) and [`text`](#text).

Supported attributes:

Attribute | Constraint
--- | ---
`id` | _UTF-8_ string. Must be unique. _CSS_ will be applied **AFTER** styles from `class` attribute.
`class` | _UTF-8_ string. Supports list of classes. _CSS_ will be applied in order of declaration of the classes and **BEFORE** styles from `id` attribute.

Default style:

Property | Value
--- | ---
[`background-color`](#background-color) | `transparent`
[`background-size`](#background-size) | `100%`
[`bottom`](#bottom) | `auto`
[`left`](#left) | `auto`
[`right`](#right) | `auto`
[`top`](#top) | `auto`
[`color`](#color) | `transparent`
[`display`](#display) | `block`
[`font-family`](#font-family) | `inherit`
[`font-size`](#font-size) | `1em`
[`line-height`](#line-height) | `inherit`
[`margin-bottom`](#margin-bottom) | `0px`
[`margin-left`](#margin-left) | `0px`
[`margin-right`](#margin-right) | `0px`
[`margin-top`](#margin-top) | `0px`
[`padding-bottom`](#padding-bottom) | `0px`
[`padding-left`](#padding-left) | `0px`
[`padding-right`](#padding-right) | `0px`
[`padding-top`](#padding-top) | `0px`
[`position`](#position) | `static`
[`text-align`](#text-align) | `left`
[`vertical-align`](#vertical-align) | `top`
[`width`](#width) | `100%`
[`height`](#height) | `auto`

[↬ table of content ⇧](#table-of-content)

#### <a id="img">_IMG_</a>

Supported child elements: _N/A_.

Supported attributes:

Attribute | Constraint
--- | ---
`id` | _UTF-8_ string. Must be unique. _CSS_ will be applied **AFTER** styles from `class` attribute.
`class` | _UTF-8_ string. Supports list of classes. _CSS_ will be applied in order of declaration of the classes and **BEFORE** styles from `id` attribute.
`src` | _UTF-8_ path to image asset. The path is relative from main _HTML_ file. Asset could be `.ktx`, `.png`, `.jpeg`, `.bmp` and `.tga`. In case of `.png` the framework will try to find `.ktx` version of the file as top priority. Such feature was implemented  for browser compatibility as a preview tool. It's because modern browsers **[2023-05-05]** are not able to render `.ktx` images which are highly optimized for mobile hardware.

Default style:

Property | Value
--- | ---
[`background-color`](#background-color) | `transparent`
[`background-size`](#background-size) | `100%`
[`bottom`](#bottom) | `auto`
[`left`](#left) | `auto`
[`right`](#right) | `auto`
[`top`](#top) | `auto`
[`color`](#color) | `transparent`
[`display`](#color) | `inline-block`
[`font-family`](#font-family) | `inherit`
[`font-size`](#font-size) | `1em`
[`line-height`](#line-height) | `inherit`
[`margin-bottom`](#margin-bottom) | `0px`
[`margin-left`](#margin-left) | `0px`
[`margin-right`](#margin-right) | `0px`
[`margin-top`](#margin-top) | `0px`
[`padding-bottom`](#padding-bottom) | `0px`
[`padding-left`](#padding-left) | `0px`
[`padding-right`](#padding-right) | `0px`
[`padding-top`](#padding-top) | `0px`
[`position`](#position) | `static`
[`text-align`](#text-align) | `left`
[`vertical-align`](#vertical-align) | `top`
[`width`](#vertical-align) | `auto`
[`height`](#vertical-align) | `auto`

[↬ table of content ⇧](#table-of-content)

#### <a id="text">Text</a>

Supported child elements: _N/A_.

Supported attributes: _N/A_.

Default style: _N/A_, all _CSS_ properties are `inherit`.

[↬ table of content ⇧](#table-of-content)

### <a id="css-constraints">_CSS_ constraints</a>

_UI_ system only supports the following _CSS_ entities:

### <a id="font-face">`@font-face`</a>

Property | Value
--- | ---
`font-family` | _UTF-8_ identifier. Must be unique.
`src` | _UTF-8_ path to font asset. The path is relative current _CSS_ file. Supported formats: `.ttf`, `.otf`, `.woff` and numerous font formats which [_FreeType_](https://freetype.org/freetype2/docs/index.html) supports. Note current [_FreeType_](https://freetype.org/freetype2/docs/index.html) version is specified at [main page](../README.md) of the current project.

[↬ table of content ⇧](#table-of-content)

### <a id="background-color">`backgroground-color`</a>

Supported values: [_HEX_ color](https://www.w3schools.com/css/css_colors_hex.asp), [_RGB_ color](https://www.w3schools.com/css/css_colors_rgb.asp), [_HSL_ color](https://www.w3schools.com/css/css_colors_hsl.asp) and [named color](https://developer.mozilla.org/en-US/docs/Web/CSS/named-color).

[↬ table of content ⇧](#table-of-content)

### <a id="background-size">`background-size`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="bottom">`bottom`</a>

Supported values: `em`, `px`, `pt`, `mm`, `%` and `auto`.

[↬ table of content ⇧](#table-of-content)

### <a id="left">`left`</a>

Supported values: `em`, `px`, `pt`, `mm`, `%` and `auto`.

[↬ table of content ⇧](#table-of-content)

### <a id="right">`right`</a>

Supported values: `em`, `px`, `pt`, `mm`, `%` and `auto`.

[↬ table of content ⇧](#table-of-content)

### <a id="top">`top`</a>

Supported values: `em`, `px`, `pt`, `mm`, `%` and `auto`.

[↬ table of content ⇧](#table-of-content)

### <a id="color">`color`</a>

Supported values: [_HEX_ color](https://www.w3schools.com/css/css_colors_hex.asp), [_RGB_ color](https://www.w3schools.com/css/css_colors_rgb.asp), [_HSL_ color](https://www.w3schools.com/css/css_colors_hsl.asp) and [named color](https://developer.mozilla.org/en-US/docs/Web/CSS/named-color).

[↬ table of content ⇧](#table-of-content)

### <a id="display">`display`</a>

Supported values: `block`, `inline-block` and `none`.

[↬ table of content ⇧](#table-of-content)

### <a id="font-family">`font-family`</a>

Supported values: _UTF-8_ identifier. The font itself must be specified via [`@font-face`](#font-face) rule. There are **NO ANY DEFAULT FONT** for _UI_ system. Every font **MUST BE** specified explicitly.

[↬ table of content ⇧](#table-of-content)

### <a id="font-size">`font-size`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="line-height">`line-height`</a>

Supported values: `normal`, `inherit`, `em`, `px`, `pt`, `mm`, `%`, `<unitless number>`.

[↬ table of content ⇧](#table-of-content)

### <a id="margin-bottom">`margin-bottom`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="margin-left">`margin-left`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="margin-right">`margin-right`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="margin-top">`margin-top`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="margin">`margin`</a>

Supported values: From 1 to 4 values according to [_CSS_ specification](https://developer.mozilla.org/en-US/docs/Web/CSS/margin). Units: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="padding-bottom">`padding-bottom`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="padding-left">`padding-left`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="padding-right">`padding-right`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="padding-top">`padding-top`</a>

Supported values: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="padding">`padding`</a>

Supported values: From 1 to 4 values according to [_CSS_ specification](https://developer.mozilla.org/en-US/docs/Web/CSS/padding). Units: `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="position">`position`</a>

Supported values: `absolute`, `relative` and `static`.

[↬ table of content ⇧](#table-of-content)

### <a id="text-align">`text-align`</a>

Supported values: `center`, `left`, `right` and `inherit`.

[↬ table of content ⇧](#table-of-content)

### <a id="vertical-align">`vertical-align`</a>

Supported values: `bottom`, `middle`, `top` and `inherit`.

[↬ table of content ⇧](#table-of-content)

### <a id="width">`width`</a>

Supported values: `auto`, `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)

### <a id="height">`height`</a>

Supported values: `auto`, `em`, `px`, `pt`, `mm` and `%`.

[↬ table of content ⇧](#table-of-content)
