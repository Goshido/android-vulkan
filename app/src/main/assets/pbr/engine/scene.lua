require "av://engine/actor.lua"
require "av://engine/camera_component.lua"
require "av://engine/rigid_body_component.lua"
require "av://engine/script_component.lua"
require "av://engine/static_mesh_component.lua"
require "av://engine/transform_component.lua"


local Scene = {}

-- Methods
local function AppendActor ( self, actor )
    local name = actor:GetName ()
    local actors = self._actors
    local list = actors[ name ]

    if not list then
        list = {}
        actors[ name ] = list
    end

    table.insert ( list, actor )

    local inputScripts = self._inputScripts
    local postPhysicsScripts = self._postPhysicsScripts
    local prePhysicsScripts = self._prePhysicsScripts
    local renderTargetChangerScripts = self._renderTargetChangeScripts
    local updateScripts = self._updateScripts

    for groupKey, group in pairs ( actor._components ) do
        for k, v in pairs ( group ) do
            if v._type == eObjectType.ScriptComponent then
                if type ( v.OnInput ) == "function" then
                    table.insert ( inputScripts, v )
                end

                if type ( v.OnPostPhysics ) == "function" then
                    table.insert ( postPhysicsScripts, v )
                end

                if type ( v.OnPrePhysics ) == "function" then
                    table.insert ( prePhysicsScripts, v )
                end

                if type ( v.OnRenderTargetChanged ) == "function" then
                    table.insert ( renderTargetChangerScripts, v )
                end

                if type ( v.OnUpdate ) == "function" then
                    table.insert ( updateScripts, v )
                end
            end
        end
    end

    actor:CommitComponents ()
end

local function FindActor ( self, name )
    local query = self._actors[ name ]

    -- Lua style ternary operator
    return query and query[ 1 ] or nil
end

local function FindActors ( self, name )
    return self._actors[ name ]
end

local function GetPenetrationBox ( self, localMatrix, size, groups )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:GetPenetrationBox - Calling not via ":" syntax.]]
    )

    assert ( type ( localMatrix ) == "table" and localMatrix._type == eObjectType.GXMat4,
        [[Scene:GetPenetrationBox - "localMatrix" is not a GXMat4.]]
    )

    assert ( type ( size ) == "table" and size._type == eObjectType.GXVec3,
        [[Scene:GetPenetrationBox - "size" is not a GXVec3.]]
    )

    assert ( type ( groups ) == "number", [[Scene:GetPenetrationBox - "groups" is not a number.]] )
    return av_SceneGetPenetrationBox ( self._handle, localMatrix._handle, size._handle, groups )
end

local function GetPhysicsToRendererScaleFactor ( self )
    return av_SceneGetPhysicsToRendererScaleFactor ()
end

local function GetRendererToPhysicsScaleFactor ( self )
    return av_SceneGetRendererToPhysicsScaleFactor ()
end

local function GetRenderTargetAspectRatio ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:GetRenderTargetAspectRatio - Calling not via ":" syntax.]]
    )

    return av_SceneGetRenderTargetAspectRatio ( self._handle )
end

local function GetRenderTargetWidth ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:GetRenderTargetWidth - Calling not via ":" syntax.]]
    )

    return av_SceneGetRenderTargetWidth ( self._handle )
end

local function GetRenderTargetHeight ( self )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:GetRenderTargetHeight - Calling not via ":" syntax.]]
    )

    return av_SceneGetRenderTargetHeight ( self._handle )
end

local function OnInput ( self, inputEvent )
    for k, v in pairs ( self._inputScripts ) do
        v:OnInput ( inputEvent )
    end
end

local function OnPostPhysics ( self, deltaTime )
    for k, v in pairs ( self._postPhysicsScripts ) do
        v:OnPostPhysics ( deltaTime )
    end
end

local function OnPrePhysics ( self, deltaTime )
    for k, v in pairs ( self._prePhysicsScripts ) do
        v:OnPrePhysics ( deltaTime )
    end
end

local function OnRenderTargetChanged ( self )
    for k, v in pairs ( self._renderTargetChangeScripts ) do
        v:OnRenderTargetChanged ()
    end
end

local function OnUpdate ( self, deltaTime )
    for k, v in pairs ( self._updateScripts ) do
        v:OnUpdate ( deltaTime )
    end
end

local function Quit ( self )
    av_SceneQuit ()
end

local function SetActiveCamera ( self, camera )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetActiveCamera - Calling not via ":" syntax.]]
    )

    assert ( type ( camera ) == "table" and camera._type == eObjectType.CameraComponent,
        [[Scene:SetActiveCamera - "camera" is not a CameraComponent.]]
    )

    av_SceneSetActiveCamera ( self._handle, camera._handle )
end

-- Metamethods
local function Constructor ( self, handle )
    local obj = Object ( eObjectType.Scene )

    -- Data
    obj._actors = {}
    obj._handle = handle
    obj._inputScripts = {}
    obj._postPhysicsScripts = {}
    obj._prePhysicsScripts = {}
    obj._renderTargetChangeScripts = {}
    obj._updateScripts = {}

    -- Methods
    obj.AppendActor = AppendActor
    obj.FindActor = FindActor
    obj.FindActors = FindActors
    obj.GetPenetrationBox = GetPenetrationBox
    obj.GetPhysicsToRendererScaleFactor = GetPhysicsToRendererScaleFactor
    obj.GetRendererToPhysicsScaleFactor = GetRendererToPhysicsScaleFactor
    obj.GetRenderTargetAspectRatio = GetRenderTargetAspectRatio
    obj.GetRenderTargetWidth = GetRenderTargetWidth
    obj.GetRenderTargetHeight = GetRenderTargetHeight
    obj.OnInput = OnInput
    obj.OnPostPhysics = OnPostPhysics
    obj.OnPrePhysics = OnPrePhysics
    obj.OnRenderTargetChanged = OnRenderTargetChanged
    obj.OnUpdate = OnUpdate
    obj.Quit = Quit
    obj.SetActiveCamera = SetActiveCamera

    return obj
end

setmetatable ( Scene, { __call = Constructor } )

-- Module contract
function CreateScene ( handle )
    g_scene = Scene ( handle )
    return g_scene
end

return nil
