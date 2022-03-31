require "av://engine/av_script_component.lua"
require "av://engine/gx_math.lua"
require "av://engine/av_logger.lua"


Player = {}

-- engine events
local function OnPostPhysics ( self, deltaTime )
    local t = self._transform;
    t:Perspective ( 1.5, 16.0 / 9.0, 1.0, 7777.77 )
    AVLogD ( string.format ( "Player:OnPostPhysics - deltaTime %g, transform:\n%s", deltaTime, t ) )
end

local function OnPrePhysics ( self, deltaTime )
    local t = self._transform;
    t:Scale ( 77.7, 3.33, 42.0 )
    AVLogD ( string.format ( "Player:OnPrePhysics - deltaTime %g, transform:\n%s", deltaTime, t ) )
end

local function OnUpdate ( self, deltaTime )
    AVLogD ( "Player:OnUpdate - deltaTime is " .. deltaTime .. "." )
end

-- metamethods
local function Constructor ( self, handle, params )
    local obj = AVScriptComponent ( handle )

    AVLogD ( ">>> Player params:" )

    for k, v in pairs ( params ) do
        AVLogD ( "    " .. k .. ": " .. v )
    end

    AVLogD ( "<<<" )

    AVLogD ( "It's debug %s", '[debug string]' )
    AVLogE ( "It's error: %f", math.rad ( 90.0 ) )
    AVLogI ( "It's info %u", 156 )
    AVLogW ( "It's warning: %f %s", 1.2, "another" )

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
