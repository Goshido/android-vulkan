require "av://engine/gx_mat3.lua"
require "av://engine/gx_mat4.lua"
require "av://engine/gx_quat.lua"
require "av://engine/gx_vec3.lua"
require "av://engine/input_event.lua"
require "av://engine/script_component.lua"


local Camera = {}

-- Constants
local TARGET_OFFSET = GXVec3 ()
TARGET_OFFSET:Init ( 0.358, 1.505, 0.0 )

local UP = GXVec3 ()
UP:Init ( 0.0, 1.0, 0.0 )

local RIGHT = GXVec3 ()
RIGHT:Init ( 1.0, 0.0, 0.0 )

local DISTANCE_CLOSE = 2.5

local PITCH_DEAD_ZONE = 0.2
local PITCH_SPEED = 3.777
local PITCH_MIN = math.rad ( -45.0 )
local PITCH_MAX = math.rad ( 75.0 )

local RIGHT_DEAD_ZONE = 0.2
local RIGHT_SPEED = 3.777

-- Forward declaration
local OnUpdate

-- Methods
local function Activate ( self, playerPosition, playerForward )
    local right = GXVec3 ()
    right:CrossProduct ( UP, playerForward )
    self._right = right

    self._pitch = 0.0
    self._pitchDir = 0.0

    self._rightDir = 0.0
    self._playerPosition = playerPosition
    self._distance = DISTANCE_CLOSE

    local c = self._camera
    c:SetAspectRatio ( g_scene:GetRenderTargetAspectRatio () )
    g_scene:SetActiveCamera ( c )

    self.OnUpdate = OnUpdate
end

local function AdjustPitch ( self, deltaTime )
    local pitch = self._pitch
    pitch = pitch + self._pitchDir * deltaTime * PITCH_SPEED

    if pitch > PITCH_MAX then
        pitch = PITCH_MAX
    end

    if pitch < PITCH_MIN then
        pitch = PITCH_MIN
    end

    self._pitch = pitch
    return pitch
end

local function AdjustRight ( self, deltaTime )
    local right = self._right

    local r = GXQuat ()
    r:FromAxisAngle ( UP, self._rightDir * deltaTime * RIGHT_SPEED )

    local alpha = GXVec3 ()
    r:TransformFast ( alpha, right )
    right:Clone ( alpha )

    return right
end

local function HandleDirection ( self, value, field, deadZone )
    if math.abs ( value ) < deadZone then
        value = 0.0
    end

    self[ field ] = value
end

-- Engine events
local function OnActorConstructed ( self, actor )
    self._camera = actor:FindComponent ( "Camera" )
end

local function OnInput ( self, inputEvent )
    if inputEvent._type ~= eEventType.RightStick then
        return
    end

    self:HandleDirection ( -inputEvent._y, "_pitchDir", PITCH_DEAD_ZONE )
    self:HandleDirection ( inputEvent._x, "_rightDir", RIGHT_DEAD_ZONE )
end

local function OnRenderTargetChanged ( self )
    self._camera:SetAspectRatio ( g_scene:GetRenderTargetAspectRatio () )
end

OnUpdate = function ( self, deltaTime )
    local pitch = self:AdjustPitch ( deltaTime )
    local right = self:AdjustRight ( deltaTime )

    local m = GXMat3 ()

    local transform = GXMat4 ()
    transform:Identity ()

    transform:SetX ( right )
    m:SetX ( right )

    m:SetY ( UP )

    local alpha = GXVec3 ()
    alpha:CrossProduct ( right, UP )
    m:SetZ ( alpha )

    local target = GXVec3 ()
    m:MultiplyVectorMatrix ( target, TARGET_OFFSET )
    target:Sum ( target, self._playerPosition )

    local r = GXQuat ()
    r:FromAxisAngle ( RIGHT, pitch )
    r:TransformFast ( alpha, UP )

    local beta = GXVec3 ()
    m:MultiplyVectorMatrix ( beta, alpha )
    transform:SetY ( beta )

    alpha:CrossProduct ( right, beta )
    transform:SetZ ( alpha )

    beta:SumScaled ( target, -self._distance, alpha )
    beta:MultiplyScalar ( beta, g_scene:GetPhysicsToRendererScaleFactor () )
    transform:SetW ( beta )

    self._camera:SetLocal ( transform )
end

local function OnUpdateIdle ( self, deltaTime )
    -- NOTHING
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Methods
    obj.Activate = Activate
    obj.AdjustPitch = AdjustPitch
    obj.AdjustRight = AdjustRight
    obj.HandleDirection = HandleDirection

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInput
    obj.OnRenderTargetChanged = OnRenderTargetChanged
    obj.OnUpdate = OnUpdateIdle

    return obj
end

setmetatable ( Camera, { __call = Constructor } )

-- Module function: fabric callable for Camera class.
return Camera
