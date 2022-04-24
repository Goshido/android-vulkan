require "av://engine/component.lua"


ScriptComponent = {}

-- helper
local function MakeScriptComponent ( handle )
    return Component ( eObjectType.ScriptComponent, handle )
end

-- metamethods
local function Constructor ( self )
    return Component ( eObjectType.ScriptComponent, "TODO:INVALID" )
end

setmetatable ( ScriptComponent, { __call = Constructor } )

-- module contract
RegisterScriptComponent = MakeScriptComponent
return nil
