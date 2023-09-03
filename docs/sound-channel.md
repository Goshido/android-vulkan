# _eSoundChannel_

```lua
require "av://engine/sound_channel.lua"
```

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)

## <a id="brief">Brief</a>

The engine has several predefined sound channels. They are used as volume control of the logical sound groups. The main purpose is to allow the end used to set loudness of each sound channel as he wishes. For example completely disable game music, boost speech loudness, disable sound effects ans so on.

The script could set sound channel via predefined entity:

```lua
eSoundChannel = {
    Music = 0,
    Ambient = 1,
    SFX = 2,
    Speech = 3
}
```

[↬ table of content ⇧](#table-of-content)
