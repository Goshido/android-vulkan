require "av://engine/component.lua"


ScriptComponent = {}

-- helper
local function MakeParams ( params )
    if ( not params ) then
        return nil
    end

    local func, errorMessage = load ( "return " .. params, "av://engine/script_component.lua", "t" )

    if ( type ( func ) ~= "function" ) then
        error ( "MakeParams - Can't load params: " .. errorMessage )
    end

    local status, result = pcall ( func )

    if ( not status ) then
        error ( "MakeParams - Can't compile params: " .. result )
    end

    return result
end

local function MakeScriptComponent ( handle, script, params )
    local fabric = require ( script )

    local obj = Component ( eObjectType.ScriptComponent, handle )
    obj._script = fabric ( params )

    return obj
end

-- metamethods
local function Constructor ( self, name, script, params )
    return MakeScriptComponent ( av_ScriptComponentCreate ( name ), script, params )
end

setmetatable ( ScriptComponent, { __call = Constructor } )

-- module contract
function RegisterScriptComponent ( handle, script, params )
    return MakeScriptComponent ( handle, script, MakeParams ( params ) )
end

return nil
