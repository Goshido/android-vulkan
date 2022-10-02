require "av://engine/object.lua"


Material = {}

-- Metamethods
local mt = {
    __gc = function ( self )
        av_MaterialDestroy ( self._handle )
    end
}

local function Constructor ( self, materialFile )
    assert ( type ( materialFile ) == "string", [[Material:Constructor - "materialFile" is not a string.]] )
    local handle = av_MaterialCreate ( materialFile )

    if handle == nil then
    return;

    local obj = Object ( eObjectType.Material )

    -- Data
    obj._handle = handle

    return setmetatable ( obj, mt )
end

setmetatable ( Material, { __call = Constructor } )

-- Module contract
return nil
