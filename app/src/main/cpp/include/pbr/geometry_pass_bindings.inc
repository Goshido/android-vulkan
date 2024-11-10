#ifndef GEOMETRY_PASS_BINDINGS_INC
#define GEOMETRY_PASS_BINDINGS_INC


#define IN_BUFFER_POSITION              0
#define IN_SLOT_POSITION                0

#define IN_BUFFER_REST                  1
#define IN_SLOT_UV                      1
#define IN_SLOT_NORMAL                  2
#define IN_SLOT_TANGENT                 3

#define ATT_SLOT_UV                     0
#define ATT_SLOT_NORMAL_VIEW            1
#define ATT_SLOT_TANGENT_VIEW           2
#define ATT_SLOT_BITANGENT_VIEW         3
#define ATT_SLOT_INSTANCE_INDEX         4

#define SET_SAMPLER                     0
#define BIND_SAMPLER                    0

#define SET_MATERIAL                    1
#define BIND_DIFFUSE_TEXTURE            0
#define BIND_EMISSION_TEXTURE           1
#define BIND_MASK_TEXTURE               2
#define BIND_NORMAL_TEXTURE             3
#define BIND_PARAMS_TEXTURE             4

#define SET_INSTANCE_DATA               2
#define BIND_INSTANCE_POSITON_DATA      0
#define BIND_INSTANCE_NORMAL_DATA       1
#define BIND_INSTANCE_COLOR_DATA        2


#endif // GEOMETRY_PASS_BINDINGS_INC
