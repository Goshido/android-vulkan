require "av://engine/object.lua"


AnimationBlendNode = {}

-- Methods
local function SetBlendFactor ( self, factor )
    assert ( type ( self ) == "table" and self._type == eObjectType.AnimationBlendNode,
        [[AnimationBlendNode:SetBlendFactor - Calling not via ":" syntax.]]
    )

    assert ( type ( factor ) == "number", [[AnimationBlendNode:SetBlendFactor - "factor" is not number.]] )
    av_AnimationBlendNodeSetBlendFactor ( self._handle, factor )
end

local function SetInputA ( self, inputA )
    assert ( type ( self ) == "table" and self._type == eObjectType.AnimationBlendNode,
        [[AnimationBlendNode:SetInputA - Calling not via ":" syntax.]]
    )

    assert ( type ( inputA ) == "table", [[AnimationBlendNode:SetInputA - "inputA" is not an object.]] )

    assert ( inputA._type == eObjectType.AnimationPlayerNode or inputA._type == eObjectType.AnimationBlendNode,
        [[AnimationBlendNode:SetInputA - "inputA" incompatible with input requirements.]]
    )

    av_AnimationBlendNodeSetInputA ( self._handle, inputA._handle )
end

local function SetInputB ( self, inputB )
    assert ( type ( self ) == "table" and self._type == eObjectType.AnimationBlendNode,
        [[AnimationBlendNode:SetInputB - Calling not via ":" syntax.]]
    )

    assert ( type ( inputB ) == "table", [[AnimationBlendNode:SetInputB - "inputB" is not an object.]] )

    assert ( inputB._type == eObjectType.AnimationPlayerNode or inputB._type == eObjectType.AnimationBlendNode,
        [[AnimationBlendNode:SetInputB - "inputB" incompatible with input requirements.]]
    )

    av_AnimationBlendNodeSetInputB ( self._handle, inputB._handle )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_AnimationBlendNodeDestroy ( self._handle )
    end
}

local function Constructor ( self )
    local handle = av_AnimationBlendNodeCreate ()

    if handle == nil then
        return
    end

    local obj = Object ( eObjectType.AnimationBlendNode )

    -- Data
    obj._handle = handle

    -- Methods
    obj.SetBlendFactor = SetBlendFactor
    obj.SetInputA = SetInputA
    obj.SetInputB = SetInputB

    return setmetatable ( obj, mt )
end

setmetatable ( AnimationBlendNode, { __call = Constructor } )

-- Module contract
return nil
