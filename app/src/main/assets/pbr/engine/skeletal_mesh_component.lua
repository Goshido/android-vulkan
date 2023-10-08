require "av://engine/component.lua"


SkeletalMeshComponent = {}

-- Methods
local function SetColor0 ( self, color )
    assert ( type ( self ) == "table" and self._type == eObjectType.SkeletalMeshComponent,
        [[SkeletalMeshComponent:SetColor0 - Calling not via ":" syntax.]]
    )

    assert ( type ( color ) == "table" and color._type == eObjectType.GXVec4,
        [[SkeletalMeshComponent:SetColor0 - "color" is not GXVec4.]]
    )

    av_SkeletalMeshComponentSetColor0 ( self._handle, color._handle )
end

local function SetColor1 ( self, color )
    assert ( type ( self ) == "table" and self._type == eObjectType.SkeletalMeshComponent,
        [[SkeletalMeshComponent:SetColor1 - Calling not via ":" syntax.]]
    )

    assert ( type ( color ) == "table" and color._type == eObjectType.GXVec3,
        [[SkeletalMeshComponent:SetColor1 - "color" is not GXVec3.]]
    )

    av_SkeletalMeshComponentSetColor1 ( self._handle, color._handle )
end

local function SetColor2 ( self, color )
    assert ( type ( self ) == "table" and self._type == eObjectType.SkeletalMeshComponent,
        [[SkeletalMeshComponent:SetColor2 - Calling not via ":" syntax.]]
    )

    assert ( type ( color ) == "table" and color._type == eObjectType.GXVec3,
        [[SkeletalMeshComponent:SetColor2 - "color" is not GXVec3.]]
    )

    av_SkeletalMeshComponentSetColor2 ( self._handle, color._handle )
end

local function SetEmission ( self, emission )
    assert ( type ( self ) == "table" and self._type == eObjectType.SkeletalMeshComponent,
        [[SkeletalMeshComponent:SetEmission - Calling not via ":" syntax.]]
    )

    assert ( type ( emission ) == "table" and emission._type == eObjectType.GXVec3,
        [[SkeletalMeshComponent:SetEmission - "emission" is not GXVec3.]]
    )

    av_SkeletalMeshComponentSetEmission ( self._handle, emission._handle )
end

local function SetMaterial ( self, material )
    assert ( type ( self ) == "table" and self._type == eObjectType.SkeletalMeshComponent,
        [[SkeletalMeshComponent:SetMaterial - Calling not via ":" syntax.]]
    )

    assert ( type ( material ) == "table" and material._type == eObjectType.Material,
        [[SkeletalMeshComponent:SetMaterial - "material" is not a Material entity.]]
    )

    av_SkeletalMeshComponentSetMaterial ( self._handle, material._handle )
end

-- Engine event handlers
local function OnDestroy ( self )
    av_SkeletalMeshComponentDestroy ( self._handle )
end

-- Metamethods
local mt = {
    __gc = function ( self )
        av_SkeletalMeshComponentCollectGarbage ( self._handle )
    end
}

local function Constructor ( self, name, meshFile, skinFile, materialFile )
    local obj = Component ( eObjectType.SkeletalMeshComponent,
        av_SkeletalMeshComponentCreate ( name, meshFile, skinFile, materialFile )
    )

    -- Methods
    obj.SetColor0 = SetColor0
    obj.SetColor1 = SetColor1
    obj.SetColor2 = SetColor2
    obj.SetEmission = SetEmission
    obj.SetMaterial = SetMaterial

    -- Engine events
    obj.OnDestroy = OnDestroy

    return setmetatable ( obj, mt )
end

setmetatable ( SkeletalMeshComponent, { __call = Constructor } )

-- Module contract
return nil
