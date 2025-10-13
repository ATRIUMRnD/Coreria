#ifndef CHAOSFORGE_RAGDOLL_H
#define CHAOSFORGE_RAGDOLL_H

// Ragdoll handle structure matching Rust
typedef struct {
    unsigned int id;
} RagdollHandle;

// Fighting style constants matching Rust enum
#define STYLE_BRAWLER 0
#define STYLE_STRIKER 1  
#define STYLE_PHANTOM 2
#define STYLE_TITAN   3

// Limb indices matching Rust enum
#define LIMB_HEAD      0
#define LIMB_TORSO     1
#define LIMB_LEFT_ARM  2
#define LIMB_RIGHT_ARM 3
#define LIMB_LEFT_LEG  4
#define LIMB_RIGHT_LEG 5

// Helper macros for ragdoll comparison and initialization
#define RAGDOLL_INVALID_ID 0
#define RAGDOLL_IS_VALID(handle) ((handle).id != RAGDOLL_INVALID_ID)
#define RAGDOLL_INIT() ((RagdollHandle){RAGDOLL_INVALID_ID})

// Ragdoll API function declarations
#ifdef __cplusplus
extern "C" {
#endif

int coreria_init_engine(void);
int coreria_shutdown_engine(void);
int coreria_step_simulation(float delta_time);
RagdollHandle coreria_spawn_ragdoll(unsigned int match_id, int style_id, float x, float y, float z);
int coreria_apply_limb_force(RagdollHandle handle, int limb_index, float fx, float fy, float fz, float tx, float ty, float tz);
int coreria_get_ragdoll_position(RagdollHandle handle, float* x, float* y, float* z);
int coreria_destroy_ragdoll(RagdollHandle handle);
int coreria_get_ragdoll_count(void);

#ifdef __cplusplus
}
#endif

#endif // CHAOSFORGE_RAGDOLL_H