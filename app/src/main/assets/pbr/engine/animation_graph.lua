require "av://engine/object.lua"


AnimationGraph = {}

-- Metamethods
local mt = {
    __gc = function ( self )
        av_AnimationGraphDestroy ( self._handle )
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

    return setmetatable ( obj, mt )
end

setmetatable ( AnimationGraph, { __call = Constructor } )

-- Module contract
return nil
