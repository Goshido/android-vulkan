require "av://engine/scene.lua"


local g_scripts = {}
local g_postPhysicsScripts = {}
local g_prePhysicsScripts = {}
local g_updateScripts = {}

-- Util functions
local function MakeParams ( params, where )
    if ( not params ) then
        return nil
    end

    local func, errorMessage = load ( "return " .. params, where, "t" )

    if ( type ( func ) ~= "function" ) then
        error ( "MakeParams - Can't load params: " .. errorMessage )
    end

    local status, result = pcall ( func )

    if ( not status ) then
        error ( "MakeParams - Can't compile params: " .. result )
    end

    return result
end

local function FUCK ()
    -- TODO
end

-- Entry points
function OnRegisterScript ( handle, class, params )
    local fabric = require ( class )
    local script = fabric ( handle, MakeParams ( params ) )
    table.insert ( g_scripts, script )

    if type ( script.OnPostPhysics ) == "function" then
        table.insert ( g_postPhysicsScripts, script )
    end

    if type ( script.OnPrePhysics ) == "function" then
        table.insert ( g_prePhysicsScripts, script )
    end

    if type ( script.OnUpdate ) == "function" then
        table.insert ( g_updateScripts, script )
    end

    FUCK ()
end
