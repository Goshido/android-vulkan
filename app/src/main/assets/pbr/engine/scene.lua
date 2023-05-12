require "av://engine/actor.lua"
require "av://engine/camera_component.lua"
require "av://engine/div_ui_element.lua"
require "av://engine/image_ui_element.lua"
require "av://engine/rigid_body_component.lua"
require "av://engine/script_component.lua"
require "av://engine/sound_emitter_global_component.lua"
require "av://engine/sound_emitter_spatial_component.lua"
require "av://engine/static_mesh_component.lua"
require "av://engine/text_ui_element.lua"
require "av://engine/transform_component.lua"


local Scene = {}

-- Methods
local function AppendActorFromNative ( self, actor )
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
    local renderTargetChangeScripts = self._renderTargetChangeScripts
    local updateScripts = self._updateScripts

    for groupKey, group in pairs ( actor._components ) do
        for k, v in pairs ( group ) do
            if v._type == eObjectType.ScriptComponent then
                if type ( v.OnInput ) == "function" then
                    inputScripts[ v ] = v
                end

                if type ( v.OnPostPhysics ) == "function" then
                    postPhysicsScripts[ v ] = v
                end

                if type ( v.OnPrePhysics ) == "function" then
                    prePhysicsScripts[ v ] = v
                end

                if type ( v.OnRenderTargetChanged ) == "function" then
                    renderTargetChangeScripts[ v ] = v
                end

                if type ( v.OnUpdate ) == "function" then
                    updateScripts[ v ] = v
                end
            end
        end
    end

    actor:CommitComponents ()
end

local function AppendActor ( self, actor )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:AppendActor - Calling not via ":" syntax.]]
    )

    assert ( type ( actor ) == "table" and actor._type == eObjectType.Actor,
        [[Scene:AppendActor - "actor" is not actor object.]]
    )

    av_SceneAppendActor ( self._handle, actor._handle )
    AppendActorFromNative ( self, actor )
end

local function DetachActor ( self, actor )
    table.insert ( self._actorToDestroy, actor )
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

    local actorToDestroy = self._actorToDestroy

    if #actorToDestroy == 0 then
        return
    end

    local inputScripts = self._inputScripts
    local postPhysicsScripts = self._postPhysicsScripts
    local prePhysicsScripts = self._prePhysicsScripts
    local renderTargetChangeScripts = self._renderTargetChangeScripts
    local updateScripts = self._updateScripts
    local actors = self._actors

    for k, actor in pairs ( actorToDestroy ) do
        for groupKey, group in pairs ( actor._components ) do
            for k, v in pairs ( group ) do
                if v._type == eObjectType.ScriptComponent then
                    inputScripts[ v ] = nil
                    postPhysicsScripts[ v ] = nil
                    prePhysicsScripts[ v ] = nil
                    renderTargetChangeScripts[ v ] = nil
                    updateScripts[ v ] = nil
                end
            end
        end

        local list = actors[ actor:GetName () ]
        local count = #list

        for i = 1, count do
            if list[ i ] == actor then
                actor:OnDestroy ()
                table.remove ( list, i )
                break
            end
        end
    end

    self._actorToDestroy = {}
end

local function OverlapTestBoxBox ( self, localMatrixA, sizeA, localMatrixB, sizeB )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:OverlapTestBoxBox - Calling not via ":" syntax.]]
    )

    assert ( type ( localMatrixA ) == "table" and localMatrixA._type == eObjectType.GXMat4,
        [[Scene:OverlapTestBoxBox - "localMatrixA" is not a GXMat4.]]
    )

    assert ( type ( sizeA ) == "table" and sizeA._type == eObjectType.GXVec3,
        [[Scene:OverlapTestBoxBox - "sizeA" is not a GXVec3.]]
    )

    assert ( type ( localMatrixB ) == "table" and localMatrixB._type == eObjectType.GXMat4,
        [[Scene:OverlapTestBoxBox - "localMatrixB" is not a GXMat4.]]
    )

    assert ( type ( sizeB ) == "table" and sizeB._type == eObjectType.GXVec3,
        [[Scene:OverlapTestBoxBox - "sizeB" is not a GXVec3.]]
    )

    return av_SceneOverlapTestBoxBox ( self._handle,
        localMatrixA._handle,
        sizeA._handle,
        localMatrixB._handle,
        sizeB._handle
    )
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

local function SetSoundChannelVolume ( self, soundChannel, volume )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetSoundChannelVolume - Calling not via ":" syntax.]]
    )

    assert ( type ( soundChannel ) == "number", [[Scene:SetSoundChannelVolume - "soundChannel" is not a number."]] )

    assert ( soundChannel >= 0 and soundChannel < eSoundChannel.TOTAL,
        [[Scene:SetSoundChannelVolume - Incorrect sound channel.]]
    )

    assert ( type ( volume ) == "number", [[Scene:SetSoundChannelVolume - "volume" is not a number.]] )
    av_SceneSetSoundChannelVolume ( self._handle, soundChannel, volume )
end

local function SetSoundListenerTransform ( self, localMatrix )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetSoundListenerTransform - Calling not via ":" syntax.]]
    )

    assert ( type ( localMatrix ) == "table" and localMatrix._type == eObjectType.GXMat4,
        [[Scene:SetSoundListenerTransform - "localMatrix" is not a GXMat4.]]
    )

    av_SceneSetSoundListenerTransform ( self._handle, localMatrix._handle )
end

local function SetSoundMasterVolume ( self, volume )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetSoundMasterVolume - Calling not via ":" syntax.]]
    )

    assert ( type ( volume ) == "number", [[Scene:SetSoundMasterVolume - "volume" is not a number.]] )
    av_SceneSetSoundMasterVolume ( self._handle, volume )
end

local function SweepTestBox ( self, localMatrix, size, groups )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SweepTestBox - Calling not via ":" syntax.]]
    )

    assert ( type ( localMatrix ) == "table" and localMatrix._type == eObjectType.GXMat4,
        [[Scene:SweepTestBox - "localMatrix" is not a GXMat4.]]
    )

    assert ( type ( size ) == "table" and size._type == eObjectType.GXVec3,
        [[Scene:SweepTestBox - "size" is not a GXVec3.]]
    )

    assert ( type ( groups ) == "number", [[Scene:SweepTestBox - "groups" is not a number.]] )
    return av_SceneSweepTestBox ( self._handle, localMatrix._handle, size._handle, groups )
end

-- Metamethods
local function Constructor ( self, handle )
    local obj = Object ( eObjectType.Scene )

    -- Data
    obj._actors = {}
    obj._actorToDestroy = {}
    obj._handle = handle
    obj._inputScripts = {}
    obj._postPhysicsScripts = {}
    obj._prePhysicsScripts = {}
    obj._renderTargetChangeScripts = {}
    obj._updateScripts = {}

    -- Methods
    obj.AppendActor = AppendActor
    obj.AppendActorFromNative = AppendActorFromNative
    obj.DetachActor = DetachActor
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
    obj.OverlapTestBoxBox = OverlapTestBoxBox
    obj.Quit = Quit
    obj.SetActiveCamera = SetActiveCamera
    obj.SetSoundChannelVolume = SetSoundChannelVolume
    obj.SetSoundListenerTransform = SetSoundListenerTransform
    obj.SetSoundMasterVolume = SetSoundMasterVolume
    obj.SweepTestBox = SweepTestBox

    return obj
end

setmetatable ( Scene, { __call = Constructor } )

-- Module contract
function CreateScene ( handle )
    g_scene = Scene ( handle )
    return g_scene
end

return nil
