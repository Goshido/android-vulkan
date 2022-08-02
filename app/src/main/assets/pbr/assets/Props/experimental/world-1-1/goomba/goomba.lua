require "av://engine/script_component.lua"


-- Constants
local SPEED = 1.3889

local GRAVITY = GXVec3 ()
GRAVITY:Init ( 0.0, -9.81, 0.0 )

local LANDING_FACTOR = -1.0e-3

-- Class declaration
local Goomba = {}

-- Methods
local function CheckMoveSensor ( self, size, offset, localMatrix, origin, velocity )
    local alpha = GXVec3 ()
    localMatrix:MultiplyAsNormal ( alpha, offset )
    alpha:Sum ( alpha, origin )

    local m = GXMat4 ()
    m:Clone ( localMatrix )
    m:SetW ( alpha )

    local sweep = g_scene:SweepTestBox ( m, size, 0xFFFFFFFF )

    if sweep._count ~= 0 and velocity:DotProduct ( self._horizontalVelocity ) < -0.5 then
        self._horizontalVelocity:Clone ( velocity )
    end
end

local function GetCenter ( self, min, max )
    local diff = GXVec3 ()
    diff:Subtract ( max, min )

    local result = GXVec3 ()
    result:SumScaled ( min, 0.5, diff )
    return result
end

local function GetOrigin ( self, origin, actor, component )
    local t = GXMat4 ()
    actor:FindComponent ( component ):GetTransform ( t )
    t:GetW ( origin )
end

local function GetSensorOffset ( self, parentOrigin, actor, min, max )
    local result = GXVec3 ()
    result:Subtract ( self:GetCenter ( actor, min, max ), parentOrigin )
    result:MultiplyScalar ( result, g_scene:GetRendererToPhysicsScaleFactor () )
    return result
end

local function GetSensorParentTransform ( self, sensor, toParentTransform )
    local m = GXMat4 ()
    sensor:GetLocal ( m )

    local result = GXMat4 ()
    result:Multiply ( m, toParentTransform )
    return result
end

local function GetSensorSize ( self, min, max )
    local result = GXVec3 ()
    result:Subtract ( max, min )
    result:MultiplyScalar ( result, g_scene:GetRendererToPhysicsScaleFactor () )
    return result
end

local function MoveTo ( self, origin )
    local m = self._localMatrix
    m:SetW ( origin )
    self._mesh:SetLocal ( m )

    local attachment = GXMat4 ()
    attachment:Multiply ( self._sensorTopParent, m )
    self._sensorTop:SetLocal ( attachment )

    attachment:Multiply ( self._sensorLeftParent, m )
    self._sensorLeft:SetLocal ( attachment )

    attachment:Multiply ( self._sensorRightParent, m )
    self._sensorRight:SetLocal ( attachment )
end

local function ResolvePenetrations ( self, localMatrix, origin )
    local alpha = GXVec3 ()
    localMatrix:MultiplyAsNormal ( alpha, self._bodyOffset )
    alpha:Sum ( alpha, origin )

    local m = GXMat4 ()
    m:Clone ( localMatrix )
    m:SetW ( alpha )

    local p = g_scene:GetPenetrationBox ( m, self._bodySize, 0xFFFFFFFF )
    local count = p._count
    local pns = p._penetrations

    for i = 1, count do
        local pn = pns[i];
        origin:SumScaled ( origin, pn._depth, pn._normal )

        if GRAVITY:DotProduct ( pn._normal ) < LANDING_FACTOR then
            self._verticalVelocity:Init ( 0.0, 0.0, 0.0 )
        end
    end

    origin:MultiplyScalar ( origin, g_scene:GetPhysicsToRendererScaleFactor () )
    self:MoveTo ( origin )
end

-- Engine event handlers
local function OnActorConstructed ( self, actor )
    local min = GXVec3 ()
    local max = GXVec3 ()

    self:GetOrigin ( min, actor, "BodyMin" )
    self:GetOrigin ( max, actor, "BodyMax" )
    self._bodySize = self:GetSensorSize ( min, max )

    self._bodyOffset = GXVec3 ()
    self._bodyOffset:Init ( 0.0, 0.0, 0.0 )

    local origin = self:GetCenter ( min, max )

    self._localMatrix = GXMat4 ()
    self._localMatrix:Identity ()
    self._localMatrix:SetW ( origin )

    self._mesh = actor:FindComponent ( "Mesh" )
    local p = GXMat4 ()
    self._mesh:GetLocal ( p )
    local toParentTransform = GXMat4 ()
    toParentTransform:Inverse ( p )

    self:GetOrigin ( min, actor, "SensorTopMin" )
    self:GetOrigin ( max, actor, "SensorTopMax" )
    self._sensorTop = actor:FindComponent ( "SensorTop" )
    self._sensorTopSize = self:GetSensorSize ( min, max )
    self._sensorTopOffset = self:GetSensorOffset ( origin, min, max )
    self._sensorTopParent = self:GetSensorParentTransform ( self._sensorTop, toParentTransform )

    self:GetOrigin ( min, actor, "SensorLeftMin" )
    self:GetOrigin ( max, actor, "SensorLeftMax" )
    self._sensorLeft = actor:FindComponent ( "SensorLeft" )
    self._sensorLeftSize = self:GetSensorSize ( min, max )
    self._sensorLeftOffset = self:GetSensorOffset ( origin, min, max )
    self._sensorLeftParent = self:GetSensorParentTransform ( self._sensorLeft, toParentTransform )

    self:GetOrigin ( min, actor, "SensorRightMin" )
    self:GetOrigin ( max, actor, "SensorRightMax" )
    self._sensorRight = actor:FindComponent ( "SensorRight" )
    self._sensorRightSize = self:GetSensorSize ( min, max )
    self._sensorRightOffset = self:GetSensorOffset ( origin, min, max )
    self._sensorRightParent = self:GetSensorParentTransform ( self._sensorRight, toParentTransform )

    self._horizontalVelocity:Init ( 0.0, 0.0, SPEED )
    self._verticalVelocity:Init ( 0.0, 0.0, 0.0 )
end

local function OnPostPhysics ( self, deltaTime )
    local localMatrix = self._localMatrix

    local origin = GXVec3 ()
    localMatrix:GetW ( origin )
    origin:MultiplyScalar ( origin, g_scene:GetRendererToPhysicsScaleFactor () )

    local velocity = GXVec3 ()
    velocity:Init ( 0.0, 0.0, -SPEED )
    self:CheckMoveSensor ( self._sensorRightSize, self._sensorRightOffset, localMatrix, origin, velocity )

    velocity:Reverse ()
    self:CheckMoveSensor ( self._sensorLeftSize, self._sensorLeftOffset, localMatrix, origin, velocity )

    self:ResolvePenetrations ( localMatrix, origin )
end

local function OnPrePhysics ( self, deltaTime )
    local vertical = self._verticalVelocity
    vertical:SumScaled ( vertical, deltaTime, GRAVITY )

    local velocity = GXVec3 ()
    velocity:Sum ( self._horizontalVelocity, vertical )

    local localMatrix = self._localMatrix
    local origin = GXVec3 ()
    localMatrix:GetW ( origin )

    origin:SumScaled ( origin, deltaTime * g_scene:GetPhysicsToRendererScaleFactor (), velocity )
    localMatrix:SetW ( origin )
end

-- Metamethods
local function Constructor ( self, handle, params )
    local obj = ScriptComponent ( handle )

    -- Data
    obj._horizontalVelocity = GXVec3 ()
    obj._verticalVelocity = GXVec3 ()

    -- Methods
    obj.CheckMoveSensor = CheckMoveSensor
    obj.GetCenter = GetCenter
    obj.GetOrigin = GetOrigin
    obj.GetSensorOffset = GetSensorOffset
    obj.GetSensorParentTransform = GetSensorParentTransform
    obj.GetSensorSize = GetSensorSize
    obj.MoveTo = MoveTo
    obj.ResolvePenetrations = ResolvePenetrations

    -- Engine events
    obj.OnActorConstructed = OnActorConstructed
    obj.OnPostPhysics = OnPostPhysics
    obj.OnPrePhysics = OnPrePhysics

    return obj
end

setmetatable ( Goomba, { __call = Constructor } )

-- Module function: fabric callable for Goomba class
return Goomba
