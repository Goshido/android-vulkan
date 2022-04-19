require "av://engine/object.lua"


local Scene = {}

-- methods
local function AppendActor ( self, actor )
    local name = actor._name
    local actors = self._actors
    local list = actors[ name ]

    if not list then
        list = {}
        actors[ name ] = list
    end

    table.insert ( list, actor )
end

local function FindActor ( self, name )
    local query = self._actors[ name ]

    -- Lua style ternary operator
    return query and query[ 1 ] or nil
end

local function FindActors ( self, name )
    return self._actors[ name ]
end

-- metamethods
local function Constructor ( self, handle )
    local obj = Object ( eObjectType.Scene )

    -- data
    obj._actors = {}
    obj._handle = handle

    -- methods
    obj.AppendActor = AppendActor
    obj.FindActor = FindActor
    obj.FindActors = FindActors

    return obj
end

setmetatable ( Scene, { __call = Constructor } )

-- module contract
function CreateScene ( handle )
    g_scene = Scene ( handle )
end

return nil
