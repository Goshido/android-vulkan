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

    local animationUpdateScripts = self._animationUpdateScripts
    local inputScripts = self._inputScripts
    local postPhysicsScripts = self._postPhysicsScripts
    local prePhysicsScripts = self._prePhysicsScripts
    local renderTargetChangeScripts = self._renderTargetChangeScripts
    local updateScripts = self._updateScripts

    for groupKey, group in pairs ( actor._components ) do
        for k, v in pairs ( group ) do
            if v._type == eObjectType.ScriptComponent then
                if type ( v.OnAnimationUpdated ) == "function" then
                    animationUpdateScripts[ v ] = v
                end

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

local function AppendUILayer ( self, uiLayer )
    av_SceneAppendUILayer ( self._handle, uiLayer._handle )
end

local function DetachActor ( self, actor )
    table.insert ( self._actorToDestroy, actor )
end

local function DetachUILayer ( self, uiLayer )
    av_SceneDetachUILayer ( self._handle, uiLayer._handle )
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
        [[Scene:GetPenetrationBox - "localMatrix" is not GXMat4.]]
    )

    assert ( type ( size ) == "table" and size._type == eObjectType.GXVec3,
        [[Scene:GetPenetrationBox - "size" is not a GXVec3.]]
    )

    assert ( type ( groups ) == "table" and groups._type == eObjectType.BitField,
        [[Scene:GetPenetrationBox - "groups" is not BitField.]]
    )

    return av_SceneGetPenetrationBox ( self._handle, localMatrix._handle, size._handle, groups._handle )
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

local function OnAnimationUpdated ( self, inputEvent )
    for k, v in pairs ( self._animationUpdateScripts ) do
        v:OnAnimationUpdated ( inputEvent )
    end
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

    local animationUpdateScripts = self._animationUpdateScripts
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
                    animationUpdateScripts[ v ] = nil
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
        [[Scene:OverlapTestBoxBox - "localMatrixA" is not GXMat4.]]
    )

    assert ( type ( sizeA ) == "table" and sizeA._type == eObjectType.GXVec3,
        [[Scene:OverlapTestBoxBox - "sizeA" is not GXVec3.]]
    )

    assert ( type ( localMatrixB ) == "table" and localMatrixB._type == eObjectType.GXMat4,
        [[Scene:OverlapTestBoxBox - "localMatrixB" is not GXMat4.]]
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

local function Raycast ( self, from, to, groups )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:Raycast - Calling not via ":" syntax.]]
    )

    assert ( type ( from ) == "table" and from._type == eObjectType.GXVec3,
        [[Scene:Raycast - "from" is not GXVec3.]]
    )

    assert ( type ( to ) == "table" and to._type == eObjectType.GXVec3,
        [[Scene:Raycast - "to" is not GXVec3.]]
    )

    assert ( type ( groups ) == "table" and groups._type == eObjectType.BitField,
        [[Scene:Raycast - "groups" is not BitField.]]
    )

    return av_SceneRaycast ( self._handle, from._handle, to._handle, groups._handle )
end

local function SetActiveCamera ( self, camera )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetActiveCamera - Calling not via ":" syntax.]]
    )

    assert ( type ( camera ) == "table" and camera._type == eObjectType.CameraComponent,
        [[Scene:SetActiveCamera - "camera" is not CameraComponent.]]
    )

    av_SceneSetActiveCamera ( self._handle, camera._handle )
end

local function SetBrightness ( self, brightnessBalance )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetBrightness - Calling not via ":" syntax.]]
    )

    assert ( type ( brightnessBalance ) == "number", [[Scene:SetBrightness - "brightnessBalance" is not number.]] )
    av_SceneSetBrightness ( self._handle, brightnessBalance )
end

local function SetExposureCompensation ( self, exposureValue )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetExposureCompensation - Calling not via ":" syntax.]]
    )

    assert ( type ( exposureValue ) == "number", [[Scene:SetExposureCompensation - "exposureValue" is not number.]] )
    av_SceneSetExposureCompensation ( self._handle, exposureValue )
end

local function SetExposureMaximumBrightness ( self, exposureValue )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetExposureMaximumBrightness - Calling not via ":" syntax.]]
    )

    assert ( type ( exposureValue ) == "number",
        [[Scene:SetExposureMaximumBrightness - "exposureValue" is not number.]]
    )

    av_SceneSetExposureMaximumBrightness ( self._handle, exposureValue )
end

local function SetExposureMinimumBrightness ( self, exposureValue )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetExposureMinimumBrightness - Calling not via ":" syntax.]]
    )

    assert ( type ( exposureValue ) == "number",
        [[Scene:SetExposureMinimumBrightness - "exposureValue" is not number.]]
    )

    av_SceneSetExposureMinimumBrightness ( self._handle, exposureValue )
end

local function SetEyeAdaptationSpeed ( self, speed )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetEyeAdaptationSpeed - Calling not via ":" syntax.]]
    )

    assert ( type ( speed ) == "number", [[Scene:SetEyeAdaptationSpeed - "speed" is not number.]] )
    av_SceneSetEyeAdaptationSpeed ( self._handle, speed )
end

local function SetSoundChannelVolume ( self, soundChannel, volume )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetSoundChannelVolume - Calling not via ":" syntax.]]
    )

    assert ( type ( soundChannel ) == "number", [[Scene:SetSoundChannelVolume - "soundChannel" is not number."]] )

    assert ( soundChannel >= 0 and soundChannel < eSoundChannel.TOTAL,
        [[Scene:SetSoundChannelVolume - Incorrect sound channel.]]
    )

    assert ( type ( volume ) == "number", [[Scene:SetSoundChannelVolume - "volume" is not number.]] )
    av_SceneSetSoundChannelVolume ( self._handle, soundChannel, volume )
end

local function SetSoundListenerTransform ( self, localMatrix )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetSoundListenerTransform - Calling not via ":" syntax.]]
    )

    assert ( type ( localMatrix ) == "table" and localMatrix._type == eObjectType.GXMat4,
        [[Scene:SetSoundListenerTransform - "localMatrix" is not GXMat4.]]
    )

    av_SceneSetSoundListenerTransform ( self._handle, localMatrix._handle )
end

local function SetSoundMasterVolume ( self, volume )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SetSoundMasterVolume - Calling not via ":" syntax.]]
    )

    assert ( type ( volume ) == "number", [[Scene:SetSoundMasterVolume - "volume" is not number.]] )
    av_SceneSetSoundMasterVolume ( self._handle, volume )
end

local function SweepTestBox ( self, localMatrix, size, groups )
    assert ( type ( self ) == "table" and self._type == eObjectType.Scene,
        [[Scene:SweepTestBox - Calling not via ":" syntax.]]
    )

    assert ( type ( localMatrix ) == "table" and localMatrix._type == eObjectType.GXMat4,
        [[Scene:SweepTestBox - "localMatrix" is not GXMat4.]]
    )

    assert ( type ( size ) == "table" and size._type == eObjectType.GXVec3,
        [[Scene:SweepTestBox - "size" is not a GXVec3.]]
    )

    assert ( type ( groups ) == "table" and groups._type == eObjectType.BitField,
        [[Scene:SweepTestBox - "groups" is not BitField.]]
    )

    return av_SceneSweepTestBox ( self._handle, localMatrix._handle, size._handle, groups._handle )
end

-- Metamethods
local function Constructor ( self, handle )
    local obj = Object ( eObjectType.Scene )

    -- Data
    obj._actors = {}
    obj._actorToDestroy = {}
    obj._animationUpdateScripts = {}
    obj._handle = handle
    obj._inputScripts = {}
    obj._postPhysicsScripts = {}
    obj._prePhysicsScripts = {}
    obj._renderTargetChangeScripts = {}
    obj._updateScripts = {}

    -- Methods
    obj.AppendActor = AppendActor
    obj.AppendActorFromNative = AppendActorFromNative
    obj.AppendUILayer = AppendUILayer
    obj.DetachActor = DetachActor
    obj.DetachUILayer = DetachUILayer
    obj.FindActor = FindActor
    obj.FindActors = FindActors
    obj.GetPenetrationBox = GetPenetrationBox
    obj.GetPhysicsToRendererScaleFactor = GetPhysicsToRendererScaleFactor
    obj.GetRendererToPhysicsScaleFactor = GetRendererToPhysicsScaleFactor
    obj.GetRenderTargetAspectRatio = GetRenderTargetAspectRatio
    obj.GetRenderTargetWidth = GetRenderTargetWidth
    obj.GetRenderTargetHeight = GetRenderTargetHeight
    obj.OnAnimationUpdated = OnAnimationUpdated
    obj.OnInput = OnInput
    obj.OnPostPhysics = OnPostPhysics
    obj.OnPrePhysics = OnPrePhysics
    obj.OnRenderTargetChanged = OnRenderTargetChanged
    obj.OnUpdate = OnUpdate
    obj.OverlapTestBoxBox = OverlapTestBoxBox
    obj.Quit = Quit
    obj.Raycast = Raycast
    obj.SetActiveCamera = SetActiveCamera
    obj.SetBrightness = SetBrightness
    obj.SetExposureCompensation = SetExposureCompensation
    obj.SetExposureMaximumBrightness = SetExposureMaximumBrightness
    obj.SetExposureMinimumBrightness = SetExposureMinimumBrightness
    obj.SetEyeAdaptationSpeed = SetEyeAdaptationSpeed
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
