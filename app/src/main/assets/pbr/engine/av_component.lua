require "av://engine/av_object.lua"


AVComponent = {}

-- methods
local function GetName ( self )
    return av_GetNameAVComponent ( self._handle )
end

-- metamethods
local function Constructor ( self, objectType, handle )
    local obj = AVObject ( objectType )

    -- data
    obj._handle = handle

    -- methods
    obj.GetName = GetName

    return obj
end

setmetatable ( AVComponent, { __call = Constructor } )

-- module contract
return nil
