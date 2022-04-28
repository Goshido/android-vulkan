require "av://engine/gx_math.lua"
require "av://engine/logger.lua"


Player = {}

-- engine events
local function OnPostPhysics ( self, deltaTime )
    if not self._oneShot then
        return
    end

    LogD ( "Player:OnPostPhysics >>>" )

    for groupKey, group in pairs ( g_scene._actors ) do
        for k, v in pairs ( group ) do
            LogD ( "    %s", v:GetName () )
        end
    end

    LogD ( "<<<" )

    self._oneShot = false
end

local function OnPrePhysics ( self, deltaTime )
    -- TODO
end

local function OnUpdate ( self, deltaTime )
    -- TODO
end

-- metamethods
local function Constructor ( self, params )
    local obj = {}

    LogD ( ">>> Player params:" )

    for k, v in pairs ( params ) do
        LogD ( "    " .. k .. ": " .. v )
    end

    LogD ( "<<<" )

    -- data
    obj._health = 100
    obj._params = params
    obj._transform = GXMat4 ()
    obj._oneShot = true

    -- engine events
    obj.OnPostPhysics = OnPostPhysics
    obj.OnPrePhysics = OnPrePhysics
    obj.OnUpdate = OnUpdate

    return obj
end

setmetatable ( Player, { __call = Constructor } )

-- module function: fabric callable for Player class.
return Player
