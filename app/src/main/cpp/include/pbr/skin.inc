#ifndef SKIN_INC
#define SKIN_INC


#define THREADS_PER_GROUP           64U

// Must be equal THREADS_PER_GROUP * THREADS_PER_GROUP.
#define THREADS_PER_PLANE           4096U

// Must be equal THREADS_PER_GROUP * THREADS_PER_PLANE.
#define THREADS_PER_SLICE           262144U

#define SET_RESOURCE                0
#define BIND_POSE                   0
#define BIND_REFERENCE_POSITIONS    1
#define BIND_REFERENCE_REST         2
#define BIND_SKIN_VERTICES          3
#define BIND_SKIN_POSITIONS         4
#define BIND_SKIN_REST              5


#endif // SKIN_INC
