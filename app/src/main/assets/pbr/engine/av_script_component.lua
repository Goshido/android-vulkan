require "av://engine/av_component.lua"


AVScriptComponent = {}

-- metamethods
local function Constructor ( self, handle )
    return AVComponent ( eAVObjectType.ScriptComponent, handle )
end

setmetatable ( AVScriptComponent, { __call = Constructor } )

-- module contract
return nil
