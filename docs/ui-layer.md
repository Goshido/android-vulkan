# _UI_ layer

```lua
require "av://engine/ui_layer.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)

## <a id="brief">Brief</a>

[_UILayer_](./ui-layer.md) is a fundamental building block for [_UI_ system](ui-system.md). Class allows to load _UI_ assets from [_HTML_](https://en.wikipedia.org/wiki/HTML) + [_CSS_](https://en.wikipedia.org/wiki/CSS) files. Additionaly class provides methods for searching child elements.

```lua
require "av://engine/ui_layer.lua"


local uiLayer = UILayer ( "pbr/assets/Props/experimental/world-1-1/ui/index.html" )
uiLayer:Show ()
```

[↬ table of content ⇧](#table-of-content)
