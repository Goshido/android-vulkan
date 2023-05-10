# _TextUIElement_

```lua
require "av://engine/text_ui_element.lua"
```


## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [`Constructor`](#constructor)
- [`Hide ()`](#method-hide)
- [`Show ()`](#method-show)
- [`SetColorHSV ( h, s, v, a )`](#method-set-color-hsv)
- [`SetColorRGB ( r, g, b, a )`](#method-set-color-rgb)
- [`SetText ( text )`](#method-set-text)

## <a id="brief">Brief</a>

Class allows to manipulate text content inside [_UI_ system](ui-system.md) assets.

```lua
require "av://engine/text_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )
local text = uiLayer:Find ( "score" )
```

[↬ table of content ⇧](#table-of-content)

## <a id="metamethods">Metamethods</a>

Metamethod | Used
--- | ---
`__add` | ❌
`__band` | ❌
`__bnot` | ❌
`__bor` | ❌
`__bxor` | ❌
`__call` | ❌
`__close` | ❌
`__concat` | ❌
`__div` | ❌
`__eq` | ❌
`__gc` | ❌
`__idiv` | ❌
`__index` | ❌
`__le` | ❌
`__len` | ❌
`__lt` | ❌
`__mod` | ❌
`__mode` | ❌
`__mul` | ❌
`__name` | ❌
`__newindex` | ❌
`__pow` | ❌
`__shl` | ❌
`__shr` | ❌
`__sub` | ❌
`__tostring` | ❌
`__unm` | ❌

[↬ table of content ⇧](#table-of-content)

## <a id="constructor">`Constructor`</a>

_N/A_

[↬ table of content ⇧](#table-of-content)

## <a id="method-hide">`Hide ()`</a>

Method hides text element.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/text_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local text = uiLayer:Find ( "score" )
text:Hide ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-show">`Show ()`</a>

Method shows text element.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/text_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local text = uiLayer:Find ( "score" )
text:Show ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-color-hsv">`SetColorHSV ( h, s, v, a )`</a>

Method sets text color using [_HSV_ model](https://en.wikipedia.org/wiki/HSL_and_HSV).

**Parameters:**

- `h` [_required, readonly, number_]: hue component in range `0 - 360`
- `s` [_required, readonly, number_]: saturation component in range `0 - 100`
- `v` [_required, readonly, number_]: value component in range `0 - 100`
- `a` [_required, readonly, number_]: alpha component in range `0 - 100`

**Return values:**

- none

**Example:**

```lua
require "av://engine/text_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local text = uiLayer:Find ( "score" )
text:SetColorHSV ( 82.7, 100.0, 72.5, 100.0 )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-color-rgb">`SetColorRGB ( r, g, b, a )`</a>

Method sets text color using [_RGB_ model](https://en.wikipedia.org/wiki/RGB_color_model).

**Parameters:**

- `r` [_required, readonly, number_]: red component in range `0 - 255`
- `g` [_required, readonly, number_]: green component in range `0 - 255`
- `b` [_required, readonly, number_]: blue component in range `0 - 255`
- `a` [_required, readonly, number_]: alpha component in range `0 - 255`

**Return values:**

- none

**Example:**

```lua
require "av://engine/text_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local text = uiLayer:Find ( "score" )
text:SetColorRGB ( 115, 185, 0, 255 )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-set-text">`SetText ( text )`</a>

Method sets text to the element.

**Parameters:**

- `text` [_required, readonly, string_]: new text

**Return values:**

- none

**Example:**

```lua
require "av://engine/text_ui_element.lua"
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )

local text = uiLayer:Find ( "score" )
text:SetText ( "42" )
```

[↬ table of content ⇧](#table-of-content)
