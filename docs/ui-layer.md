# _UILayer_

```lua
require "av://engine/ui_layer.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Metamethods_](#metamethods)
- [_Constructor_](#constructor)
- [`Find ( id )`](#method-find)
- [`Hide ()`](#method-hide)
- [`Show ()`](#method-show)
- [`IsVisible ()`](#method-is-visiable)

## <a id="brief">Brief</a>

[_UILayer_](./ui-layer.md) is a fundamental building block of the [_UI_ system](ui-system.md). Class allows to load _UI_ assets from [_HTML_](https://en.wikipedia.org/wiki/HTML) + [_CSS_](https://en.wikipedia.org/wiki/CSS) files. Additionaly class provides methods for searching child elements.

```lua
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )
uiLayer:Show ()
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
`__call` | ✔️
`__close` | ❌
`__concat` | ❌
`__div` | ❌
`__eq` | ❌
`__gc` | ✔️
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

Constructor loads _UI_ asset from file. Note that _UI_ asset will be invisible until the user explicitly shows it.

**Parameters:**

- `path` [_required, readonly, string_]: path to main _HTML_ file of the asset

**Example:**

```lua
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-find">`Find ( id )`</a>

Method tries to find element with `id` attribute.

**Parameters:**

- `id` [_required, readonly, string_]: search target

**Return values:**

- `#1` [_required, UIElement or nil_]: _UIElement_ with specified `id` if it exists. Otherwise `nil` if there is no such _UIElement_

**Example:**

```lua
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )
local score = uiLayer:Find ( "score" )
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-hide">`Hide ()`</a>

Method hides _UI_ asset from the screen.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )
uiLayer:Hide ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-show">`Show ()`</a>

Method makes _UI_ asset visible on the screen.

**Parameters:**

- none

**Return values:**

- none

**Example:**

```lua
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )
uiLayer:Show ()
```

[↬ table of content ⇧](#table-of-content)

## <a id="method-is-visiable">`IsVisible ()`</a>

Method returns `true` if _UI_ asset is visible on the screen. Otherwise method returns `false`.

**Parameters:**

- none

**Return values:**

- `#1` [_required, boolean_]: `true` if the _UI_ asset is visible, otherwise `false`

**Example:**

```lua
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "example/ui/index.html" )
local visible = uiLayer:IsVisible ()
```

[↬ table of content ⇧](#table-of-content)
