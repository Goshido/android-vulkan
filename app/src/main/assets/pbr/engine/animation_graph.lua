require "av://engine/object.lua"


AnimationGraph = {}

-- Methods
local function Awake ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.AnimationGraph,
        [[AnimationGraph:Awake - Calling not via ":" syntax.]]
    )

    av_AnimationGraphAwake ( self._handle )
end

local function SetInput ( self, inputNode )
    assert ( type ( self ) == "table" and self._type == eObjectType.AnimationGraph,
        [[AnimationGraph:SetInput - Calling not via ":" syntax.]]
    )

    assert ( type ( inputNode ) == "table", [[AnimationPlayerNode:SetInput - "inputNode" is not an object.]] )

    assert ( inputNode._type == eObjectType.AnimationPlayerNode,
        [[AnimationPlayerNode:SetInput - "inputNode" incompatible with input requirements.]]
    )

    av_AnimationGraphSetInput ( self._handle, inputNode._handle )
end

local function Sleep ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.AnimationGraph,
        [[AnimationGraph:Sleep - Calling not via ":" syntax.]]
    )

    av_AnimationGraphSleep ( self._handle )
    self._inputNode = inputNode
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_AnimationGraphCollectGarbage ( self._handle )
    end
}

local function Constructor ( self, skeletonFile )
    assert ( type ( skeletonFile ) == "string", [[AnimationGraph:Constructor - "skeletonFile" is not a string.]] )
    local handle = av_AnimationGraphCreate ( skeletonFile )

    if handle == nil then
        return
    end

    local obj = Object ( eObjectType.AnimationGraph )

    -- Data
    obj._handle = handle

    -- Methods
    obj.Awake = Awake
    obj.SetInput = SetInput
    obj.Sleep = Sleep

    return setmetatable ( obj, mt )
end

setmetatable ( AnimationGraph, { __call = Constructor } )

-- Module contract
return nil
