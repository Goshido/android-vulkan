require "av://engine/bit_field.lua"
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

local PITCH_DEAD_ZONE = 0.25
local PITCH_SPEED = 3.777
local PITCH_MIN = math.rad ( -45.0 )
local PITCH_MAX = math.rad ( 75.0 )

local RIGHT_DEAD_ZONE = 0.25
local RIGHT_SPEED = 3.777

local Z_NEAR = 6.4
local Z_FAR = 10000.0
local FOV_Y = math.rad ( 55.0 )

-- Optimization: the value should be negative.
local DELAY_CAMERA_SPEED = -4.0

-- Forward declaration
local OnInput
local OnUpdate

-- Free functions
local function MakePivotOrientation ( right )
    local m = GXMat3 ()
    m:SetX ( right )
    m:SetY ( UP )

    local alpha = GXVec3 ()
    alpha:CrossProduct ( right, UP )
    m:SetZ ( alpha )
    return m
end

-- Methods
local function Activate ( self, playerLocation, playerForward )
    local right = GXVec3 ()
    right:CrossProduct ( UP, playerForward )
    self._right = right

    local pitch = 0.0
    self._pitch = pitch
    self._pitchDir = 0.0

    self._rightDir = 0.0
    self._playerForward = playerForward
    self._playerLocation = playerLocation
    self._distance = DISTANCE_CLOSE
    self._isLookBack = false
    self._oldLookBack = false

    local hitGroups = BitField ()
    hitGroups:SetAllBits ()
    self._hitGroups = hitGroups

    local pivotOrientation = MakePivotOrientation ( right )
    self._location = self:GetIdealLocation ( pitch, right, self:GetTarget ( pivotOrientation ), pivotOrientation )

    self:OnRenderTargetChanged ()
    g_scene:SetActiveCamera ( self._camera )

    self.OnUpdate = OnUpdate
    self:OnUpdate ( 0.0 )

    self.OnInput = OnInput
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

local function GetIdealLocation ( self, pitch, right, target, pivotOrientation )
    local transform = GXMat4 ()
    transform:Identity ()
    transform:SetX ( right )
    transform:SetW ( target )

    local r = GXQuat ()
    r:FromAxisAngle ( RIGHT, pitch )

    local alpha = GXVec3 ()
    r:TransformFast ( alpha, UP )

    local beta = GXVec3 ()
    pivotOrientation:MultiplyVectorMatrix ( beta, alpha )
    transform:SetY ( beta )

    alpha:CrossProduct ( right, beta )
    transform:SetZ ( alpha )

    local d = self._distance
    local negD = -d
    d = d * d

    local gamma = GXVec3 ()
    local hitGroups = self._hitGroups
    local corners = self._corners

    for i = 1, 4 do
        transform:MultiplyAsPoint ( beta, corners[ i ] )
        gamma:SumScaled ( beta, negD, alpha )

        local hit = g_scene:Raycast ( beta, gamma, hitGroups )

        if hit then
            local newD = beta:SquaredDistance ( hit._point )

            if d > newD then
                d = newD
            end
        end
    end

    beta:SumScaled ( target, -math.sqrt ( d ), alpha )
    return beta
end

local function GetRight ( self )
    return self._right
end

local function GetTarget ( self, pivotOrientation )
    local target = GXVec3 ()
    pivotOrientation:MultiplyVectorMatrix ( target, TARGET_OFFSET )
    target:Sum ( target, self._playerLocation )
    return target
end

local function HandleDirection ( self, value, field, deadZone )
    if math.abs ( value ) < deadZone then
        value = 0.0
    end

    self[ field ] = value
end

-- Engine events
local function OnActorConstructed ( self, actor )
    local camera = actor:FindComponent ( "Camera" )
    self._camera = camera

    camera:SetProjection ( FOV_Y, g_scene:GetRenderTargetAspectRatio (), Z_NEAR, Z_FAR )

    local corners = {}
    table.insert ( corners, GXVec3 () )
    table.insert ( corners, GXVec3 () )
    table.insert ( corners, GXVec3 () )
    table.insert ( corners, GXVec3 () )
    self._corners = corners
end

OnInput = function ( self, inputEvent )
    local t = inputEvent._type
    local down = t == eEventType.KeyDown

    if ( down or t == eEventType.KeyUp ) and inputEvent._key == eKey.RightStick then
        self._isLookBack = down
        return
    end

    if t ~= eEventType.RightStick then
        return
    end

    self:HandleDirection ( -inputEvent._y, "_pitchDir", PITCH_DEAD_ZONE )
    self:HandleDirection ( inputEvent._x, "_rightDir", RIGHT_DEAD_ZONE )
end

local function OnInputIdle ( self, inputEvent )
    -- NOTHING
end

local function OnRenderTargetChanged ( self )
    local aspect = g_scene:GetRenderTargetAspectRatio ()
    self._camera:SetAspectRatio ( aspect )

    local dY = Z_NEAR * math.tan ( FOV_Y * 0.5 ) * g_scene:GetRendererToPhysicsScaleFactor ()
    local dX = dY * aspect

    local corners = self._corners
    corners[ 1 ]:Init ( -dX, -dY, 0.0 )
    corners[ 2 ]:Init ( dX, -dY, 0.0 )
    corners[ 3 ]:Init ( dX, dY, 0.0 )
    corners[ 4 ]:Init ( -dX, dY, 0.0 )
end

OnUpdate = function ( self, deltaTime )
    local right = self._right
    local isLookBack = self._isLookBack
    local isLookBackChanged = isLookBack ~= self._oldLookBack

    if isLookBackChanged then
        if isLookBack then
            right:CrossProduct ( self._playerForward, UP )
        else
            right:CrossProduct ( UP, self._playerForward )
        end

        self._pitch = 0.0
    end

    local pivotOrientation = MakePivotOrientation ( right )
    local target = self:GetTarget ( pivotOrientation )

    local location = self._location
    local alpha = self:GetIdealLocation ( self:AdjustPitch ( deltaTime ), right, target, pivotOrientation )

    if isLookBackChanged then
        location:Clone ( alpha )
        self._oldLookBack = isLookBack
    else
        alpha:Subtract ( alpha, location )
        location:SumScaled ( location, 1.0 - math.exp ( DELAY_CAMERA_SPEED * deltaTime ), alpha )
    end

    alpha:Subtract ( target, location )
    alpha:Normalize ()

    local transform = GXMat4 ()
    transform:Identity ()
    transform:SetZ ( alpha )

    right:CrossProduct ( UP, alpha )
    right:Normalize ()
    transform:SetX ( right )

    local beta = GXVec3 ()
    beta:CrossProduct ( alpha, right )
    transform:SetY ( beta )
    transform:SetW ( location )

    g_scene:SetSoundListenerTransform ( transform )

    alpha:MultiplyScalar ( location, g_scene:GetPhysicsToRendererScaleFactor () )
    transform:SetW ( alpha )
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
    obj.GetIdealLocation = GetIdealLocation
    obj.GetRight = GetRight
    obj.GetTarget = GetTarget
    obj.HandleDirection = HandleDirection

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnInput = OnInputIdle
    obj.OnRenderTargetChanged = OnRenderTargetChanged
    obj.OnUpdate = OnUpdateIdle

    return obj
end

setmetatable ( Camera, { __call = Constructor } )

-- Module function: fabric callable for Camera class.
return Camera
