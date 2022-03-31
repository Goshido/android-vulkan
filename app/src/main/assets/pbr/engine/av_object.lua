-- IIFE pattern
local MakeUniqueID = ( function ()
    local c = -1

    return function ()
        c = c + 1
        return c
    end
end ) ()

eAVObjectType = {
    GXMat4 = MakeUniqueID (),
    ScriptComponent = MakeUniqueID (),
    Unknown = MakeUniqueID ()
}

------------------------------------------------------------------------------------------------------------------------

AVObject = {
    _type = eAVObjectType.Unknown
}

-- metamethods
local function Constructor ( self, objectType )
    return {
        _type = objectType
    }
end

setmetatable ( AVObject, { __call = Constructor } )

-- module contract
return nil
