#ifndef GANGBEAST_AVATAR_H
#define GANGBEAST_AVATAR_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <stdbool.h>

// Avatar body type variations
typedef enum {
    BODY_TYPE_SLIM = 0,     // Lean and agile
    BODY_TYPE_NORMAL = 1,   // Standard proportions
    BODY_TYPE_CHUNKY = 2,   // Stocky and strong
    BODY_TYPE_TALL = 3,     // Tall and lanky
    BODY_TYPE_COUNT
} AvatarBodyType;

// Avatar color schemes
typedef struct {
    float r, g, b, a;
} AvatarColor;

// Predefined color palettes
typedef enum {
    COLOR_SCHEME_RED = 0,
    COLOR_SCHEME_BLUE = 1,
    COLOR_SCHEME_GREEN = 2,
    COLOR_SCHEME_YELLOW = 3,
    COLOR_SCHEME_PURPLE = 4,
    COLOR_SCHEME_ORANGE = 5,
    COLOR_SCHEME_PINK = 6,
    COLOR_SCHEME_CYAN = 7,
    COLOR_SCHEME_COUNT
} AvatarColorScheme;

// Avatar accessories
typedef enum {
    ACCESSORY_NONE = 0,
    ACCESSORY_HAT = 1,
    ACCESSORY_GLASSES = 2,
    ACCESSORY_MASK = 4,
    ACCESSORY_CAPE = 8,
    ACCESSORY_GLOVES = 16,
    ACCESSORY_BOOTS = 32
} AvatarAccessory;

// Body part proportions for different body types
typedef struct {
    // Head proportions
    float head_scale;
    float head_offset_y;
    
    // Torso proportions
    float torso_width;
    float torso_height;
    float torso_depth;
    
    // Limb proportions
    float arm_length;
    float arm_thickness;
    float leg_length;
    float leg_thickness;
    
    // Overall scaling
    float overall_scale;
    float mass_multiplier;
} BodyProportions;

// Avatar customization data
typedef struct {
    AvatarBodyType body_type;
    AvatarColorScheme color_scheme;
    AvatarColor primary_color;
    AvatarColor secondary_color;
    AvatarColor accent_color;
    
    // Accessories bitmask
    int accessories;
    
    // Custom proportions (overrides body type if set)
    BodyProportions* custom_proportions;
    
    // Animation state
    float animation_time;
    int current_animation;
    
    // Physics integration
    bool ragdoll_active;
    float ragdoll_stiffness;
} GangbeastAvatar;

// Animation states
typedef enum {
    ANIM_IDLE = 0,
    ANIM_WALKING = 1,
    ANIM_RUNNING = 2,
    ANIM_JUMPING = 3,
    ANIM_ATTACKING = 4,
    ANIM_BLOCKING = 5,
    ANIM_STUNNED = 6,
    ANIM_RAGDOLL = 7,
    ANIM_COUNT
} AvatarAnimation;

// Rendering detail levels
typedef enum {
    DETAIL_LOW = 0,     // Simple cubes and spheres
    DETAIL_MEDIUM = 1,  // Basic geometry with some detail
    DETAIL_HIGH = 2,    // Detailed meshes with accessories
    DETAIL_ULTRA = 3    // Full detail with textures and effects
} RenderDetail;

// Function declarations
void gangbeast_avatar_init(void);
void gangbeast_avatar_cleanup(void);

// Avatar creation and management
GangbeastAvatar* gangbeast_avatar_create(AvatarBodyType body_type, AvatarColorScheme color_scheme);
void gangbeast_avatar_destroy(GangbeastAvatar* avatar);
void gangbeast_avatar_copy(const GangbeastAvatar* src, GangbeastAvatar* dst);

// Customization functions
void gangbeast_avatar_set_body_type(GangbeastAvatar* avatar, AvatarBodyType body_type);
void gangbeast_avatar_set_color_scheme(GangbeastAvatar* avatar, AvatarColorScheme scheme);
void gangbeast_avatar_set_custom_color(GangbeastAvatar* avatar, AvatarColor primary, AvatarColor secondary, AvatarColor accent);
void gangbeast_avatar_add_accessory(GangbeastAvatar* avatar, AvatarAccessory accessory);
void gangbeast_avatar_remove_accessory(GangbeastAvatar* avatar, AvatarAccessory accessory);

// Rendering functions
void gangbeast_avatar_render(const GangbeastAvatar* avatar, float x, float y, float z, float rotation, RenderDetail detail);
void gangbeast_avatar_render_body_part(const GangbeastAvatar* avatar, int body_part, float x, float y, float z, float rx, float ry, float rz);

// Animation functions
void gangbeast_avatar_set_animation(GangbeastAvatar* avatar, AvatarAnimation animation);
void gangbeast_avatar_update_animation(GangbeastAvatar* avatar, float delta_time);
void gangbeast_avatar_get_bone_transform(const GangbeastAvatar* avatar, int bone_id, float* x, float* y, float* z, float* rx, float* ry, float* rz);

// Utility functions
BodyProportions gangbeast_avatar_get_proportions(AvatarBodyType body_type);
AvatarColor gangbeast_avatar_get_color(AvatarColorScheme scheme, int color_index);
const char* gangbeast_avatar_get_body_type_name(AvatarBodyType body_type);
const char* gangbeast_avatar_get_color_scheme_name(AvatarColorScheme scheme);

// Physics integration
void gangbeast_avatar_sync_with_ragdoll(GangbeastAvatar* avatar, void* ragdoll_handle);
void gangbeast_avatar_apply_physics_transform(GangbeastAvatar* avatar, int body_part, float x, float y, float z, float rx, float ry, float rz);

// Preset avatars for quick setup
GangbeastAvatar* gangbeast_avatar_create_preset_brawler(void);
GangbeastAvatar* gangbeast_avatar_create_preset_striker(void);
GangbeastAvatar* gangbeast_avatar_create_preset_phantom(void);
GangbeastAvatar* gangbeast_avatar_create_preset_titan(void);

#endif // GANGBEAST_AVATAR_H
