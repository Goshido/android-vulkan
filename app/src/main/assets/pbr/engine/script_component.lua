require "av://engine/component.lua"


ScriptComponent = {}

-- metamethods
local function Constructor ( self, handle )
    return Component ( eObjectType.ScriptComponent, handle )
end

setmetatable ( ScriptComponent, { __call = Constructor } )

-- module contract
return nil
