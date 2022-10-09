require "av://engine/component.lua"
require "av://engine/input_event.lua"
require "av://engine/logger.lua"


ScriptComponent = {}

-- Helper
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

-- Engine event handlers
local function OnDestroy ( self )
    if type ( self.OnAboutToDestroy ) == "function" then
        self:OnAboutToDestroy ()
    end

    av_ScriptComponentDestroy ( self._handle )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_ScriptComponentCollectGarbage ( self._handle )
    end
}

local function MakeScriptComponent ( handle, script, params )
    local fabric = require ( script )
    obj = fabric ( handle, params )

    if not obj then
        return nil
    end

    -- Engine events
    obj.OnDestroy = OnDestroy

    return setmetatable ( obj, mt )
end

local function Constructor ( self, ... )
    local argv = { ... }
    local argc = #argv

    if argc == 1 then
        -- Call originated from ScriptComponent successor constructor.
        local handle = argv[ 1 ]
        return setmetatable ( Component ( eObjectType.ScriptComponent, handle ), mt )
    end

    if argc ~= 2 and argc ~= 3 then
        error ( "ScriptComponent:Constructor - Unexpected amount of arguments. Must be 1 or 3. Got " .. argc .. "." )
        return nil
    end

    -- Call originated from Lua VM side.
    local name = argv[ 1 ]
    local script = argv[ 2 ]
    local params = argv[ 3 ]
    return MakeScriptComponent ( av_ScriptComponentCreate ( name ), script, params )
end

setmetatable ( ScriptComponent, { __call = Constructor } )

-- Module contract
function RegisterScriptComponent ( handle, script, params )
    return MakeScriptComponent ( handle, script, MakeParams ( params ) )
end

return nil
