require "av://engine/object.lua"


AnimationPlayerNode = {}

-- Methods
local function LoadAnimation ( self, animation )
    assert ( type ( self ) == "table" and self._type == eObjectType.AnimationPlayerNode,
        [[AnimationPlayerNode:LoadAnimation - Calling not via ":" syntax.]]
    )

    assert ( type ( animation ) == "string", [[AnimationPlayerNode:LoadAnimation - "animation" is not string.]] )
    return av_AnimationPlayerNodeLoadAnimation ( self._handle, animation )
end

local function SetPlaybackSpeed ( self, speed )
    assert ( type ( self ) == "table" and self._type == eObjectType.AnimationPlayerNode,
        [[AnimationPlayerNode:SetPlaybackSpeed - Calling not via ":" syntax.]]
    )

    assert ( type ( speed ) == "number", [[AnimationPlayerNode:SetPlaybackSpeed - "speed" is not number.]] )
    av_AnimationPlayerNodeSetPlaybackSpeed ( self._handle, speed )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_AnimationPlayerNodeDestroy ( self._handle )
    end
}

local function Constructor ( self )
    local handle = av_AnimationPlayerNodeCreate ()

    if handle == nil then
        return
    end

    local obj = Object ( eObjectType.AnimationPlayerNode )

    -- Data
    obj._handle = handle

    -- Methods
    obj.LoadAnimation = LoadAnimation
    obj.SetPlaybackSpeed = SetPlaybackSpeed

    return setmetatable ( obj, mt )
end

setmetatable ( AnimationPlayerNode, { __call = Constructor } )

-- Module contract
return nil
