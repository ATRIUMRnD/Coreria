#include "gangbeast_avatar.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Simple replacement for GLUT functions
static void simple_solid_cube(float size) {
    float half = size * 0.5f;

    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0, 0, 1);
    glVertex3f(-half, -half, half);
    glVertex3f(half, -half, half);
    glVertex3f(half, half, half);
    glVertex3f(-half, half, half);

    // Back face
    glNormal3f(0, 0, -1);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, -half, -half);

    // Top face
    glNormal3f(0, 1, 0);
    glVertex3f(-half, half, -half);
    glVertex3f(-half, half, half);
    glVertex3f(half, half, half);
    glVertex3f(half, half, -half);

    // Bottom face
    glNormal3f(0, -1, 0);
    glVertex3f(-half, -half, -half);
    glVertex3f(half, -half, -half);
    glVertex3f(half, -half, half);
    glVertex3f(-half, -half, half);

    // Right face
    glNormal3f(1, 0, 0);
    glVertex3f(half, -half, -half);
    glVertex3f(half, half, -half);
    glVertex3f(half, half, half);
    glVertex3f(half, -half, half);

    // Left face
    glNormal3f(-1, 0, 0);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, -half, half);
    glVertex3f(-half, half, half);
    glVertex3f(-half, half, -half);
    glEnd();
}

#define glutSolidCube(size) simple_solid_cube(size)

// Global state
static bool g_avatar_system_initialized = false;
static GLUquadric* g_quadric = NULL;

// Predefined body proportions for each body type
static const BodyProportions g_body_proportions[BODY_TYPE_COUNT] = {
    // BODY_TYPE_SLIM
    {
        .head_scale = 0.9f,
        .head_offset_y = 0.1f,
        .torso_width = 0.4f,
        .torso_height = 1.0f,
        .torso_depth = 0.25f,
        .arm_length = 0.7f,
        .arm_thickness = 0.15f,
        .leg_length = 0.9f,
        .leg_thickness = 0.18f,
        .overall_scale = 0.95f,
        .mass_multiplier = 0.8f
    },
    // BODY_TYPE_NORMAL
    {
        .head_scale = 1.0f,
        .head_offset_y = 0.0f,
        .torso_width = 0.5f,
        .torso_height = 1.0f,
        .torso_depth = 0.3f,
        .arm_length = 0.8f,
        .arm_thickness = 0.2f,
        .leg_length = 1.0f,
        .leg_thickness = 0.22f,
        .overall_scale = 1.0f,
        .mass_multiplier = 1.0f
    },
    // BODY_TYPE_CHUNKY
    {
        .head_scale = 1.1f,
        .head_offset_y = -0.05f,
        .torso_width = 0.7f,
        .torso_height = 0.9f,
        .torso_depth = 0.4f,
        .arm_length = 0.75f,
        .arm_thickness = 0.28f,
        .leg_length = 0.85f,
        .leg_thickness = 0.3f,
        .overall_scale = 1.1f,
        .mass_multiplier = 1.4f
    },
    // BODY_TYPE_TALL
    {
        .head_scale = 0.95f,
        .head_offset_y = 0.2f,
        .torso_width = 0.45f,
        .torso_height = 1.2f,
        .torso_depth = 0.28f,
        .arm_length = 0.95f,
        .arm_thickness = 0.18f,
        .leg_length = 1.3f,
        .leg_thickness = 0.2f,
        .overall_scale = 1.15f,
        .mass_multiplier = 1.1f
    }
};

// Predefined color schemes
static const AvatarColor g_color_schemes[COLOR_SCHEME_COUNT][3] = {
    // RED scheme: primary, secondary, accent
    {{1.0f, 0.2f, 0.2f, 1.0f}, {0.8f, 0.1f, 0.1f, 1.0f}, {1.0f, 0.6f, 0.6f, 1.0f}},
    // BLUE scheme
    {{0.2f, 0.2f, 1.0f, 1.0f}, {0.1f, 0.1f, 0.8f, 1.0f}, {0.6f, 0.6f, 1.0f, 1.0f}},
    // GREEN scheme
    {{0.2f, 1.0f, 0.2f, 1.0f}, {0.1f, 0.8f, 0.1f, 1.0f}, {0.6f, 1.0f, 0.6f, 1.0f}},
    // YELLOW scheme
    {{1.0f, 1.0f, 0.2f, 1.0f}, {0.8f, 0.8f, 0.1f, 1.0f}, {1.0f, 1.0f, 0.6f, 1.0f}},
    // PURPLE scheme
    {{0.8f, 0.2f, 1.0f, 1.0f}, {0.6f, 0.1f, 0.8f, 1.0f}, {1.0f, 0.6f, 1.0f, 1.0f}},
    // ORANGE scheme
    {{1.0f, 0.6f, 0.2f, 1.0f}, {0.8f, 0.4f, 0.1f, 1.0f}, {1.0f, 0.8f, 0.6f, 1.0f}},
    // PINK scheme
    {{1.0f, 0.4f, 0.8f, 1.0f}, {0.8f, 0.2f, 0.6f, 1.0f}, {1.0f, 0.7f, 0.9f, 1.0f}},
    // CYAN scheme
    {{0.2f, 1.0f, 1.0f, 1.0f}, {0.1f, 0.8f, 0.8f, 1.0f}, {0.6f, 1.0f, 1.0f, 1.0f}}
};

// Body type names
static const char* g_body_type_names[BODY_TYPE_COUNT] = {
    "Slim", "Normal", "Chunky", "Tall"
};

// Color scheme names
static const char* g_color_scheme_names[COLOR_SCHEME_COUNT] = {
    "Red", "Blue", "Green", "Yellow", "Purple", "Orange", "Pink", "Cyan"
};

void gangbeast_avatar_init(void) {
    if (g_avatar_system_initialized) return;
    
    g_quadric = gluNewQuadric();
    if (g_quadric) {
        gluQuadricNormals(g_quadric, GLU_SMOOTH);
        gluQuadricTexture(g_quadric, GL_TRUE);
    }
    
    g_avatar_system_initialized = true;
    printf("[Avatar System] Initialized gangbeast avatar system\n");
}

void gangbeast_avatar_cleanup(void) {
    if (!g_avatar_system_initialized) return;
    
    if (g_quadric) {
        gluDeleteQuadric(g_quadric);
        g_quadric = NULL;
    }
    
    g_avatar_system_initialized = false;
    printf("[Avatar System] Cleaned up gangbeast avatar system\n");
}

GangbeastAvatar* gangbeast_avatar_create(AvatarBodyType body_type, AvatarColorScheme color_scheme) {
    if (!g_avatar_system_initialized) {
        gangbeast_avatar_init();
    }
    
    GangbeastAvatar* avatar = (GangbeastAvatar*)malloc(sizeof(GangbeastAvatar));
    if (!avatar) return NULL;
    
    // Initialize with defaults
    memset(avatar, 0, sizeof(GangbeastAvatar));
    
    avatar->body_type = body_type;
    avatar->color_scheme = color_scheme;
    avatar->primary_color = g_color_schemes[color_scheme][0];
    avatar->secondary_color = g_color_schemes[color_scheme][1];
    avatar->accent_color = g_color_schemes[color_scheme][2];
    
    avatar->accessories = ACCESSORY_NONE;
    avatar->custom_proportions = NULL;
    avatar->animation_time = 0.0f;
    avatar->current_animation = ANIM_IDLE;
    avatar->ragdoll_active = false;
    avatar->ragdoll_stiffness = 1.0f;
    
    return avatar;
}

void gangbeast_avatar_destroy(GangbeastAvatar* avatar) {
    if (!avatar) return;
    
    if (avatar->custom_proportions) {
        free(avatar->custom_proportions);
    }
    
    free(avatar);
}

void gangbeast_avatar_copy(const GangbeastAvatar* src, GangbeastAvatar* dst) {
    if (!src || !dst) return;
    
    // Save custom proportions pointer
    BodyProportions* old_proportions = dst->custom_proportions;
    
    // Copy main structure
    memcpy(dst, src, sizeof(GangbeastAvatar));
    
    // Handle custom proportions
    if (src->custom_proportions) {
        if (!old_proportions) {
            dst->custom_proportions = (BodyProportions*)malloc(sizeof(BodyProportions));
        } else {
            dst->custom_proportions = old_proportions;
        }
        
        if (dst->custom_proportions) {
            memcpy(dst->custom_proportions, src->custom_proportions, sizeof(BodyProportions));
        }
    } else {
        if (old_proportions) {
            free(old_proportions);
        }
        dst->custom_proportions = NULL;
    }
}

void gangbeast_avatar_set_body_type(GangbeastAvatar* avatar, AvatarBodyType body_type) {
    if (!avatar || body_type >= BODY_TYPE_COUNT) return;
    avatar->body_type = body_type;
}

void gangbeast_avatar_set_color_scheme(GangbeastAvatar* avatar, AvatarColorScheme scheme) {
    if (!avatar || scheme >= COLOR_SCHEME_COUNT) return;
    
    avatar->color_scheme = scheme;
    avatar->primary_color = g_color_schemes[scheme][0];
    avatar->secondary_color = g_color_schemes[scheme][1];
    avatar->accent_color = g_color_schemes[scheme][2];
}

void gangbeast_avatar_set_custom_color(GangbeastAvatar* avatar, AvatarColor primary, AvatarColor secondary, AvatarColor accent) {
    if (!avatar) return;
    
    avatar->primary_color = primary;
    avatar->secondary_color = secondary;
    avatar->accent_color = accent;
}

void gangbeast_avatar_add_accessory(GangbeastAvatar* avatar, AvatarAccessory accessory) {
    if (!avatar) return;
    avatar->accessories |= accessory;
}

void gangbeast_avatar_remove_accessory(GangbeastAvatar* avatar, AvatarAccessory accessory) {
    if (!avatar) return;
    avatar->accessories &= ~accessory;
}

BodyProportions gangbeast_avatar_get_proportions(AvatarBodyType body_type) {
    if (body_type >= BODY_TYPE_COUNT) {
        return g_body_proportions[BODY_TYPE_NORMAL];
    }
    return g_body_proportions[body_type];
}

AvatarColor gangbeast_avatar_get_color(AvatarColorScheme scheme, int color_index) {
    if (scheme >= COLOR_SCHEME_COUNT || color_index < 0 || color_index > 2) {
        AvatarColor default_color = {0.8f, 0.8f, 0.8f, 1.0f};
        return default_color;
    }
    return g_color_schemes[scheme][color_index];
}

const char* gangbeast_avatar_get_body_type_name(AvatarBodyType body_type) {
    if (body_type >= BODY_TYPE_COUNT) return "Unknown";
    return g_body_type_names[body_type];
}

const char* gangbeast_avatar_get_color_scheme_name(AvatarColorScheme scheme) {
    if (scheme >= COLOR_SCHEME_COUNT) return "Unknown";
    return g_color_scheme_names[scheme];
}

// Helper function to set OpenGL color
static void set_avatar_color(const AvatarColor* color) {
    glColor4f(color->r, color->g, color->b, color->a);
}

// Helper function to draw a rounded cube (gangbeast style)
static void draw_rounded_cube(float width, float height, float depth, int segments) {
    if (!g_quadric) return;

    float radius = fminf(fminf(width, height), depth) * 0.1f;

    glPushMatrix();

    // Main body (slightly smaller to accommodate rounded edges)
    glPushMatrix();
    glScalef(width - radius*2, height - radius*2, depth - radius*2);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Rounded edges (simplified - just add some spheres at corners)
    float offsets[][3] = {
        {(width-radius)/2, (height-radius)/2, (depth-radius)/2},
        {-(width-radius)/2, (height-radius)/2, (depth-radius)/2},
        {(width-radius)/2, -(height-radius)/2, (depth-radius)/2},
        {-(width-radius)/2, -(height-radius)/2, (depth-radius)/2},
        {(width-radius)/2, (height-radius)/2, -(depth-radius)/2},
        {-(width-radius)/2, (height-radius)/2, -(depth-radius)/2},
        {(width-radius)/2, -(height-radius)/2, -(depth-radius)/2},
        {-(width-radius)/2, -(height-radius)/2, -(depth-radius)/2}
    };

    for (int i = 0; i < 8; i++) {
        glPushMatrix();
        glTranslatef(offsets[i][0], offsets[i][1], offsets[i][2]);
        gluSphere(g_quadric, radius, segments/2, segments/2);
        glPopMatrix();
    }

    glPopMatrix();
}

// Helper function to draw a capsule (cylinder with rounded ends)
static void draw_capsule(float radius, float height, int segments) {
    if (!g_quadric) return;

    glPushMatrix();

    // Main cylinder
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    gluCylinder(g_quadric, radius, radius, height, segments, 1);
    glPopMatrix();

    // Top sphere
    glPushMatrix();
    glTranslatef(0, height/2, 0);
    gluSphere(g_quadric, radius, segments, segments/2);
    glPopMatrix();

    // Bottom sphere
    glPushMatrix();
    glTranslatef(0, -height/2, 0);
    gluSphere(g_quadric, radius, segments, segments/2);
    glPopMatrix();

    glPopMatrix();
}

// Render individual body parts
static void render_head(const GangbeastAvatar* avatar, const BodyProportions* props, RenderDetail detail) {
    set_avatar_color(&avatar->primary_color);

    glPushMatrix();
    glTranslatef(0, props->head_offset_y, 0);

    float head_size = 0.35f * props->head_scale;

    if (detail >= DETAIL_MEDIUM) {
        // More detailed head with slight oval shape
        glPushMatrix();
        glScalef(1.0f, 1.1f, 0.9f);
        gluSphere(g_quadric, head_size, 12, 8);
        glPopMatrix();

        // Simple face features
        if (detail >= DETAIL_HIGH) {
            set_avatar_color(&avatar->accent_color);

            // Eyes
            glPushMatrix();
            glTranslatef(-0.1f, 0.05f, head_size * 0.8f);
            gluSphere(g_quadric, 0.03f, 6, 4);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(0.1f, 0.05f, head_size * 0.8f);
            gluSphere(g_quadric, 0.03f, 6, 4);
            glPopMatrix();
        }
    } else {
        // Simple sphere head
        gluSphere(g_quadric, head_size, 8, 6);
    }

    // Accessories
    if (avatar->accessories & ACCESSORY_HAT) {
        set_avatar_color(&avatar->secondary_color);
        glPushMatrix();
        glTranslatef(0, head_size * 0.7f, 0);
        glScalef(1.2f, 0.3f, 1.2f);
        gluSphere(g_quadric, head_size, 8, 4);
        glPopMatrix();
    }

    glPopMatrix();
}

static void render_torso(const GangbeastAvatar* avatar, const BodyProportions* props, RenderDetail detail) {
    set_avatar_color(&avatar->primary_color);

    if (detail >= DETAIL_MEDIUM) {
        draw_rounded_cube(props->torso_width, props->torso_height, props->torso_depth, 8);
    } else {
        glPushMatrix();
        glScalef(props->torso_width, props->torso_height, props->torso_depth);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Chest detail
    if (detail >= DETAIL_HIGH) {
        set_avatar_color(&avatar->secondary_color);
        glPushMatrix();
        glTranslatef(0, 0.1f, props->torso_depth * 0.51f);
        glScalef(props->torso_width * 0.6f, props->torso_height * 0.4f, 0.05f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

static void render_arm(const GangbeastAvatar* avatar, const BodyProportions* props, bool is_left, RenderDetail detail) {
    set_avatar_color(&avatar->primary_color);

    float arm_x = is_left ? -props->torso_width * 0.6f : props->torso_width * 0.6f;

    glPushMatrix();
    glTranslatef(arm_x, props->torso_height * 0.2f, 0);

    if (detail >= DETAIL_MEDIUM) {
        // Upper arm
        glPushMatrix();
        glTranslatef(0, -props->arm_length * 0.25f, 0);
        draw_capsule(props->arm_thickness, props->arm_length * 0.5f, 8);
        glPopMatrix();

        // Forearm
        set_avatar_color(&avatar->secondary_color);
        glPushMatrix();
        glTranslatef(0, -props->arm_length * 0.75f, 0);
        draw_capsule(props->arm_thickness * 0.8f, props->arm_length * 0.5f, 8);
        glPopMatrix();

        // Hand
        set_avatar_color(&avatar->accent_color);
        glPushMatrix();
        glTranslatef(0, -props->arm_length, 0);
        gluSphere(g_quadric, props->arm_thickness * 1.2f, 8, 6);
        glPopMatrix();
    } else {
        // Simple arm
        glPushMatrix();
        glTranslatef(0, -props->arm_length * 0.5f, 0);
        glScalef(props->arm_thickness * 2, props->arm_length, props->arm_thickness * 2);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Gloves accessory
    if (avatar->accessories & ACCESSORY_GLOVES) {
        set_avatar_color(&avatar->secondary_color);
        glPushMatrix();
        glTranslatef(0, -props->arm_length, 0);
        gluSphere(g_quadric, props->arm_thickness * 1.4f, 8, 6);
        glPopMatrix();
    }

    glPopMatrix();
}

static void render_leg(const GangbeastAvatar* avatar, const BodyProportions* props, bool is_left, RenderDetail detail) {
    set_avatar_color(&avatar->primary_color);

    float leg_x = is_left ? -props->torso_width * 0.25f : props->torso_width * 0.25f;

    glPushMatrix();
    glTranslatef(leg_x, -props->torso_height * 0.5f, 0);

    if (detail >= DETAIL_MEDIUM) {
        // Thigh
        glPushMatrix();
        glTranslatef(0, -props->leg_length * 0.25f, 0);
        draw_capsule(props->leg_thickness, props->leg_length * 0.5f, 8);
        glPopMatrix();

        // Shin
        set_avatar_color(&avatar->secondary_color);
        glPushMatrix();
        glTranslatef(0, -props->leg_length * 0.75f, 0);
        draw_capsule(props->leg_thickness * 0.9f, props->leg_length * 0.5f, 8);
        glPopMatrix();

        // Foot
        set_avatar_color(&avatar->accent_color);
        glPushMatrix();
        glTranslatef(0, -props->leg_length, props->leg_thickness);
        glScalef(props->leg_thickness * 1.5f, props->leg_thickness * 0.8f, props->leg_thickness * 2.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    } else {
        // Simple leg
        glPushMatrix();
        glTranslatef(0, -props->leg_length * 0.5f, 0);
        glScalef(props->leg_thickness * 2, props->leg_length, props->leg_thickness * 2);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Boots accessory
    if (avatar->accessories & ACCESSORY_BOOTS) {
        set_avatar_color(&avatar->secondary_color);
        glPushMatrix();
        glTranslatef(0, -props->leg_length, props->leg_thickness);
        glScalef(props->leg_thickness * 1.8f, props->leg_thickness * 1.2f, props->leg_thickness * 3.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    glPopMatrix();
}

// Main rendering function
void gangbeast_avatar_render(const GangbeastAvatar* avatar, float x, float y, float z, float rotation, RenderDetail detail) {
    if (!avatar || !g_avatar_system_initialized) return;

    BodyProportions props = avatar->custom_proportions ?
        *avatar->custom_proportions :
        gangbeast_avatar_get_proportions(avatar->body_type);

    glPushMatrix();

    // Position and scale
    glTranslatef(x, y, z);
    glRotatef(rotation * 180.0f / M_PI, 0, 1, 0);
    glScalef(props.overall_scale, props.overall_scale, props.overall_scale);

    // Enable lighting for better 3D appearance
    if (detail >= DETAIL_MEDIUM) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        GLfloat light_pos[] = {1.0f, 1.0f, 1.0f, 0.0f};
        GLfloat light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
        GLfloat light_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};

        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    }

    // Render body parts
    glPushMatrix();
    glTranslatef(0, 1.7f, 0);  // Head position
    render_head(avatar, &props, detail);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 1.0f, 0);  // Torso position
    render_torso(avatar, &props, detail);
    glPopMatrix();

    // Arms
    glPushMatrix();
    glTranslatef(0, 1.0f, 0);  // Arm base position
    render_arm(avatar, &props, true, detail);   // Left arm
    render_arm(avatar, &props, false, detail);  // Right arm
    glPopMatrix();

    // Legs
    glPushMatrix();
    glTranslatef(0, 1.0f, 0);  // Leg base position
    render_leg(avatar, &props, true, detail);   // Left leg
    render_leg(avatar, &props, false, detail);  // Right leg
    glPopMatrix();

    // Cape accessory
    if (avatar->accessories & ACCESSORY_CAPE && detail >= DETAIL_HIGH) {
        set_avatar_color(&avatar->secondary_color);
        glPushMatrix();
        glTranslatef(0, 1.2f, -props.torso_depth * 0.6f);
        glScalef(props.torso_width * 1.2f, props.torso_height * 0.8f, 0.05f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    if (detail >= DETAIL_MEDIUM) {
        glDisable(GL_LIGHTING);
        glDisable(GL_COLOR_MATERIAL);
    }

    glPopMatrix();
}

// Animation functions
void gangbeast_avatar_set_animation(GangbeastAvatar* avatar, AvatarAnimation animation) {
    if (!avatar || animation >= ANIM_COUNT) return;

    if (avatar->current_animation != (int)animation) {
        avatar->current_animation = animation;
        avatar->animation_time = 0.0f;
    }
}

void gangbeast_avatar_update_animation(GangbeastAvatar* avatar, float delta_time) {
    if (!avatar) return;

    avatar->animation_time += delta_time;

    // Loop animations after certain durations
    switch (avatar->current_animation) {
        case ANIM_IDLE:
            if (avatar->animation_time > 4.0f) avatar->animation_time = 0.0f;
            break;
        case ANIM_WALKING:
            if (avatar->animation_time > 1.0f) avatar->animation_time = 0.0f;
            break;
        case ANIM_RUNNING:
            if (avatar->animation_time > 0.6f) avatar->animation_time = 0.0f;
            break;
        case ANIM_ATTACKING:
            if (avatar->animation_time > 0.5f) {
                avatar->current_animation = ANIM_IDLE;
                avatar->animation_time = 0.0f;
            }
            break;
        default:
            break;
    }
}

void gangbeast_avatar_get_bone_transform(const GangbeastAvatar* avatar, int bone_id,
                                       float* x, float* y, float* z,
                                       float* rx, float* ry, float* rz) {
    if (!avatar || !x || !y || !z || !rx || !ry || !rz) return;

    // Default transforms
    *x = *y = *z = 0.0f;
    *rx = *ry = *rz = 0.0f;

    // Apply animation-based transforms
    float t = avatar->animation_time;

    switch (avatar->current_animation) {
        case ANIM_WALKING:
            if (bone_id == 4 || bone_id == 5) { // Legs
                *rx = sinf(t * 6.0f) * 20.0f; // Leg swing
            }
            if (bone_id == 2 || bone_id == 3) { // Arms
                *rx = sinf(t * 6.0f + M_PI) * 15.0f; // Arm swing
            }
            break;

        case ANIM_ATTACKING:
            if (bone_id == 3) { // Right arm
                *rx = -90.0f + sinf(t * 20.0f) * 45.0f;
            }
            break;

        case ANIM_IDLE:
            // Subtle breathing animation
            if (bone_id == 1) { // Torso
                *y = sinf(t * 2.0f) * 0.02f;
            }
            break;

        default:
            break;
    }
}

// Physics integration
void gangbeast_avatar_sync_with_ragdoll(GangbeastAvatar* avatar, void* ragdoll_handle) {
    if (!avatar || !ragdoll_handle) return;

    avatar->ragdoll_active = true;
    // This would integrate with the actual ragdoll physics system
    // For now, just mark as ragdoll active
}

void gangbeast_avatar_apply_physics_transform(GangbeastAvatar* avatar, int body_part,
                                            float x, float y, float z,
                                            float rx, float ry, float rz) {
    if (!avatar) return;

    // This would apply physics-driven transforms to body parts
    // Implementation depends on the physics integration
    (void)body_part; (void)x; (void)y; (void)z; (void)rx; (void)ry; (void)rz;
}

// Preset avatar creation functions
GangbeastAvatar* gangbeast_avatar_create_preset_brawler(void) {
    GangbeastAvatar* avatar = gangbeast_avatar_create(BODY_TYPE_CHUNKY, COLOR_SCHEME_RED);
    if (avatar) {
        gangbeast_avatar_add_accessory(avatar, ACCESSORY_GLOVES);
        avatar->ragdoll_stiffness = 1.2f; // Stiffer for brawler style
    }
    return avatar;
}

GangbeastAvatar* gangbeast_avatar_create_preset_striker(void) {
    GangbeastAvatar* avatar = gangbeast_avatar_create(BODY_TYPE_SLIM, COLOR_SCHEME_GREEN);
    if (avatar) {
        gangbeast_avatar_add_accessory(avatar, ACCESSORY_BOOTS);
        avatar->ragdoll_stiffness = 0.8f; // More flexible for agility
    }
    return avatar;
}

GangbeastAvatar* gangbeast_avatar_create_preset_phantom(void) {
    GangbeastAvatar* avatar = gangbeast_avatar_create(BODY_TYPE_TALL, COLOR_SCHEME_BLUE);
    if (avatar) {
        gangbeast_avatar_add_accessory(avatar, ACCESSORY_CAPE);
        gangbeast_avatar_add_accessory(avatar, ACCESSORY_MASK);
        avatar->ragdoll_stiffness = 0.9f;
    }
    return avatar;
}

GangbeastAvatar* gangbeast_avatar_create_preset_titan(void) {
    GangbeastAvatar* avatar = gangbeast_avatar_create(BODY_TYPE_CHUNKY, COLOR_SCHEME_YELLOW);
    if (avatar) {
        gangbeast_avatar_add_accessory(avatar, ACCESSORY_HAT);
        gangbeast_avatar_add_accessory(avatar, ACCESSORY_GLOVES);
        gangbeast_avatar_add_accessory(avatar, ACCESSORY_BOOTS);
        avatar->ragdoll_stiffness = 1.5f; // Very stiff for tank-like behavior
    }
    return avatar;
}

// Body part rendering for ragdoll integration
void gangbeast_avatar_render_body_part(const GangbeastAvatar* avatar, int body_part,
                                     float x, float y, float z,
                                     float rx, float ry, float rz) {
    if (!avatar || !g_avatar_system_initialized) return;

    BodyProportions props = avatar->custom_proportions ?
        *avatar->custom_proportions :
        gangbeast_avatar_get_proportions(avatar->body_type);

    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rx, 1, 0, 0);
    glRotatef(ry, 0, 1, 0);
    glRotatef(rz, 0, 0, 1);

    switch (body_part) {
        case 0: // Head
            render_head(avatar, &props, DETAIL_MEDIUM);
            break;
        case 1: // Torso
            render_torso(avatar, &props, DETAIL_MEDIUM);
            break;
        case 2: // Left arm
            render_arm(avatar, &props, true, DETAIL_MEDIUM);
            break;
        case 3: // Right arm
            render_arm(avatar, &props, false, DETAIL_MEDIUM);
            break;
        case 4: // Left leg
            render_leg(avatar, &props, true, DETAIL_MEDIUM);
            break;
        case 5: // Right leg
            render_leg(avatar, &props, false, DETAIL_MEDIUM);
            break;
        default:
            break;
    }

    glPopMatrix();
}
