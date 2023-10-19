#ifndef SKIN_INC
#define SKIN_INC


#define THREADS_PER_GROUP           64U

// Must be equal THREADS_PER_GROUP * THREADS_PER_GROUP.
#define THREADS_PER_Z               4096U

#define PUSH_VERTEX_COUNT           0

#define SET_RESOURCE                0
#define BIND_POSE                   0
#define BIND_REFERENCE_MESH         1
#define BIND_SKIN                   2
#define BIND_SKIN_MESH              3


#endif // SKIN_INC
