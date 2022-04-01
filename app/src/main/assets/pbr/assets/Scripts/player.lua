require "av://engine/av_script_component.lua"
require "av://engine/gx_math.lua"
require "av://engine/av_logger.lua"


Player = {}

-- engine events
local function OnPostPhysics ( self, deltaTime )
    -- TODO
end

local function OnPrePhysics ( self, deltaTime )
    -- TODO
end

local function OnUpdate ( self, deltaTime )
    -- TODO
end

-- metamethods
local function Constructor ( self, handle, params )
    local obj = AVScriptComponent ( handle )

    AVLogD ( ">>> Player params:" )

    for k, v in pairs ( params ) do
        AVLogD ( "    " .. k .. ": " .. v )
    end

    AVLogD ( "<<<" )

    -- data
    obj._health = 100
    obj._transform = GXMat4 ()

    -- engine events
    obj.OnPostPhysics = OnPostPhysics
    obj.OnPrePhysics = OnPrePhysics
    obj.OnUpdate = OnUpdate

    return obj
end

setmetatable ( Player, { __call = Constructor } )

-- module function: fabric callable for Player class.
return Player
