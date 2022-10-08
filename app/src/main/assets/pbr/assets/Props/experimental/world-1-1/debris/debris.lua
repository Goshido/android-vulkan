Debris = {}


-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )
    -- TODO
    return obj;
end

setmetatable ( Debris, { __call = Constructor } )

-- Module function: fabric callable for Debris class
return Debris
