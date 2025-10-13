#define _USE_MATH_DEFINES
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "player_controller.h"
#include "combat_system.h"
#include "physics_manager.h"
#include "game_state.h"
#include "simple_glut.h"
#include "gangbeast_avatar.h"
#include "avatar_menu.h"

// Simple ragdoll handle for local physics
typedef struct {
    unsigned int id;
} RagdollHandle;

// Only include multiplayer if available
#ifdef ENABLE_MULTIPLAYER
#include "multiplayer_integration.h"
#endif

// Stub functions for when multiplayer is not available
#ifndef ENABLE_MULTIPLAYER
static inline int init_multiplayer_engine(void) { return 0; }
static inline void cleanup_multiplayer_engine(void) {}
static inline void update_multiplayer_engine(float dt) { (void)dt; }
static inline RagdollHandle coreria_spawn_ragdoll(int style, int player_id, float x, float y, float z) {
    (void)style; (void)player_id; (void)x; (void)y; (void)z;
    RagdollHandle handle = {0};
    return handle;
}
static inline int coreria_apply_limb_force(RagdollHandle handle, int limb, float fx, float fy, float fz, float tx, float ty, float tz) {
    (void)handle; (void)limb; (void)fx; (void)fy; (void)fz; (void)tx; (void)ty; (void)tz;
    return 0;
}
static inline int coreria_set_ragdoll_position(RagdollHandle handle, float x, float y, float z) {
    (void)handle; (void)x; (void)y; (void)z;
    return 0;
}
#endif

// Window and display constants
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define MAX_PLAYERS 4
#define ARENA_SIZE 20.0f

// Player input states
typedef struct {
    int move_forward, move_backward, move_left, move_right;
    int sprint, block, attack_light, attack_heavy;
    int jump, crouch;
    float mouse_dx, mouse_dy;
    int mouse_left, mouse_right;
} PlayerInput;

// Unified player state
typedef struct {
    // Position and physics
    float x, y, z;
    float vel_x, vel_y, vel_z;
    float facing_angle;
    
    // Combat state
    int health, max_health;
    int lives;
    int stamina, max_stamina;
    int combo_meter;
    int fighting_style;
    
    // Animation and effects
    int attack_animation;
    int block_animation;
    int respawn_animation;
    int stun_timer;
    
    // Input and controls
    PlayerInput input;
    int player_id;
    int is_local_player;
    
    // Physics integration
    RagdollHandle ragdoll;
    int ragdoll_active;

    // Avatar system
    GangbeastAvatar* avatar;
    int avatar_initialized;
} UnifiedPlayer;

// Global game state
typedef struct {
    UnifiedPlayer players[MAX_PLAYERS];
    int num_players;
    int local_player_id;
    
    // Physics world
    int physics_initialized;
    int multiplayer_initialized;
    
    // Arena state
    float arena_bounds;
    int arena_shrink_timer;
    
    // Camera system
    float cam_x, cam_y, cam_z;
    float cam_pitch, cam_yaw;
    float cam_distance;
    int follow_player;
    
    // Game flow
    int game_running;
    int in_menu;
    int selected_style;

    // Avatar customization
    AvatarMenu* avatar_menu;
    int in_avatar_menu;
    int customizing_player;

    // Performance tracking
    int frame_count;
    float delta_time;
    
} UnifiedGameState;

// Global variables
HWND g_hWnd;
HDC g_hDC;
HGLRC g_hRC;
UnifiedGameState game;
BOOL keys[256] = {FALSE};
int mouse_x = 0, mouse_y = 0;

// Function prototypes
BOOL init_unified_system(HWND hWnd);
void cleanup_unified_system(void);
void update_unified_game(float delta_time);
void render_unified_game(void);
void handle_player_input(int player_id, PlayerInput* input);
void update_player_physics(UnifiedPlayer* player, float delta_time);
void update_player_combat(UnifiedPlayer* player, float delta_time);
void render_player(UnifiedPlayer* player);
void render_arena(void);
void render_hud(void);
void check_player_collisions(void);
void apply_arena_physics(void);
LRESULT CALLBACK UnifiedWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Helper function to create avatar based on fighting style
GangbeastAvatar* create_avatar_for_style(int fighting_style) {
    switch (fighting_style) {
        case 0: return gangbeast_avatar_create_preset_brawler();
        case 1: return gangbeast_avatar_create_preset_striker();
        case 2: return gangbeast_avatar_create_preset_phantom();
        case 3: return gangbeast_avatar_create_preset_titan();
        default: return gangbeast_avatar_create(BODY_TYPE_NORMAL, COLOR_SCHEME_RED);
    }
}

// Initialize the unified system
BOOL init_unified_system(HWND hWnd) {
    // Initialize OpenGL
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    g_hDC = GetDC(hWnd);
    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    if (!pixelFormat) return FALSE;
    
    if (!SetPixelFormat(g_hDC, pixelFormat, &pfd)) return FALSE;
    
    g_hRC = wglCreateContext(g_hDC);
    if (!g_hRC) return FALSE;
    
    if (!wglMakeCurrent(g_hDC, g_hRC)) return FALSE;
    
    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
    
    // Initialize game state
    memset(&game, 0, sizeof(UnifiedGameState));
    game.arena_bounds = ARENA_SIZE;
    game.cam_distance = 15.0f;
    game.cam_pitch = -20.0f;
    game.cam_y = 10.0f;
    game.in_menu = 1;
    game.selected_style = 0;
    game.game_running = 1;

    // Initialize avatar system
    printf("[UNIFIED] Initializing avatar system...\n");
    gangbeast_avatar_init();
    avatar_menu_init();
    
    // Initialize physics system
    printf("[UNIFIED] Initializing physics system...\n");
    game.physics_initialized = 1;
    
    // Initialize multiplayer engine
    printf("[UNIFIED] Initializing multiplayer engine...\n");
    if (init_multiplayer_engine()) {
        game.multiplayer_initialized = 1;
        printf("[UNIFIED] Multiplayer engine ready\n");
    } else {
        printf("[UNIFIED] Warning: Multiplayer engine failed to initialize\n");
    }
    
    // Create initial local player
    game.num_players = 1;
    game.local_player_id = 0;
    
    UnifiedPlayer* player = &game.players[0];
    player->player_id = 0;
    player->is_local_player = 1;
    player->x = 0.0f;
    player->y = 0.0f;
    player->z = 5.0f;
    player->max_health = 100;
    player->health = player->max_health;
    player->lives = 3;
    player->max_stamina = 100;
    player->stamina = player->max_stamina;
    player->fighting_style = 0; // Will be set from menu

    // Initialize avatar with default settings
    player->avatar = gangbeast_avatar_create(BODY_TYPE_NORMAL, COLOR_SCHEME_RED);
    player->avatar_initialized = (player->avatar != NULL);

    // Create avatar customization menu
    game.avatar_menu = avatar_menu_create(0);
    game.in_avatar_menu = 0;
    game.customizing_player = 0;

    printf("[UNIFIED] System initialized successfully\n");
    return TRUE;
}

// Handle player input and movement
void handle_player_input(int player_id, PlayerInput* input) {
    if (player_id >= game.num_players) return;
    
    UnifiedPlayer* player = &game.players[player_id];
    
    // Movement calculations
    float move_speed = 0.1f;
    float sprint_multiplier = 1.5f;
    float move_x = 0.0f, move_z = 0.0f;
    
    // Calculate movement based on input
    if (input->move_forward) move_z -= 1.0f;
    if (input->move_backward) move_z += 1.0f;
    if (input->move_left) move_x -= 1.0f;
    if (input->move_right) move_x += 1.0f;
    
    // Apply sprint if available
    if (input->sprint && player->stamina > 10) {
        move_speed *= sprint_multiplier;
        player->stamina -= 1;
    } else if (player->stamina < player->max_stamina) {
        player->stamina += 1;
    }
    
    // Apply movement to velocity
    player->vel_x = move_x * move_speed;
    player->vel_z = move_z * move_speed;
    
    // Handle combat input
    if (input->attack_light && player->stamina > 15) {
        player->attack_animation = 30; // 30 frame animation
        player->stamina -= 15;
        player->combo_meter += 10;
        printf("[PLAYER %d] Light attack! Stamina: %d\n", player_id, player->stamina);
    }
    
    if (input->attack_heavy && player->stamina > 25) {
        player->attack_animation = 45; // 45 frame animation
        player->stamina -= 25;
        player->combo_meter += 20;
        printf("[PLAYER %d] Heavy attack! Stamina: %d\n", player_id, player->stamina);
    }
    
    if (input->block && player->stamina > 5) {
        player->block_animation = 10;
        player->stamina -= 1;
    }
    
    // Handle facing direction based on mouse input
    if (input->mouse_dx != 0.0f) {
        player->facing_angle += input->mouse_dx * 0.02f;
    }
}

// Update player physics
void update_player_physics(UnifiedPlayer* player, float delta_time) {
    // Apply gravity
    if (player->y > 0.0f) {
        player->vel_y -= 9.8f * delta_time;
    }
    
    // Update position
    player->x += player->vel_x;
    player->y += player->vel_y * delta_time;
    player->z += player->vel_z;
    
    // Ground collision
    if (player->y <= 0.0f) {
        player->y = 0.0f;
        player->vel_y = 0.0f;
    }
    
    // Arena boundaries
    if (player->x < -game.arena_bounds) {
        player->x = -game.arena_bounds;
        player->vel_x = 0.0f;
    }
    if (player->x > game.arena_bounds) {
        player->x = game.arena_bounds;
        player->vel_x = 0.0f;
    }
    if (player->z < -game.arena_bounds) {
        player->z = -game.arena_bounds;
        player->vel_z = 0.0f;
    }
    if (player->z > game.arena_bounds) {
        player->z = game.arena_bounds;
        player->vel_z = 0.0f;
    }
    
    // Apply friction
    player->vel_x *= 0.9f;
    player->vel_z *= 0.9f;
    
    // Update ragdoll physics if active
    if (game.multiplayer_initialized && player->ragdoll_active) {
        // Sync position with ragdoll system
        coreria_set_ragdoll_position(player->ragdoll, player->x, player->y, player->z);
        
        if (player->attack_animation > 0) {
            // Apply force during attack
            float force_x = sin(player->facing_angle) * 100.0f;
            float force_z = cos(player->facing_angle) * 100.0f;
            coreria_apply_limb_force(player->ragdoll, 2, force_x, 0.0f, force_z, 0.0f, 0.0f, 0.0f); // Arm
        }
    }
}

// Update combat system
void update_player_combat(UnifiedPlayer* player, float delta_time) {
    // Decay animations
    if (player->attack_animation > 0) player->attack_animation--;
    if (player->block_animation > 0) player->block_animation--;
    if (player->respawn_animation > 0) player->respawn_animation--;
    if (player->stun_timer > 0) player->stun_timer--;
    
    // Decay combo meter
    if (player->combo_meter > 0) player->combo_meter--;
    if (player->combo_meter > 100) player->combo_meter = 100;
    
    // Health regeneration
    if (player->health < player->max_health && player->health > 0) {
        static int regen_timer = 0;
        regen_timer++;
        if (regen_timer > 300) { // 5 seconds at 60fps
            player->health += 1;
            regen_timer = 0;
        }
    }
}

// Check collisions between players
void check_player_collisions(void) {
    for (int i = 0; i < game.num_players; i++) {
        for (int j = i + 1; j < game.num_players; j++) {
            UnifiedPlayer* p1 = &game.players[i];
            UnifiedPlayer* p2 = &game.players[j];
            
            float dx = p1->x - p2->x;
            float dz = p1->z - p2->z;
            float distance = sqrt(dx*dx + dz*dz);
            
            // Attack collision (range = 2.0f)
            if (distance < 2.0f) {
                if (p1->attack_animation > 20 && p2->block_animation == 0) {
                    int damage = (p1->attack_animation > 35) ? 30 : 20; // Heavy vs light
                    p2->health -= damage;
                    p2->stun_timer = 15;
                    printf("[COMBAT] Player %d hits Player %d for %d damage! Health: %d\n", 
                           i, j, damage, p2->health);
                    
                    if (p2->health <= 0) {
                        p2->lives--;
                        if (p2->lives > 0) {
                            p2->health = p2->max_health;
                            p2->respawn_animation = 60;
                            printf("[RESPAWN] Player %d respawns! Lives remaining: %d\n", j, p2->lives);
                        } else {
                            printf("[ELIMINATED] Player %d eliminated!\n", j);
                        }
                    }
                }
                
                // Push players apart to prevent overlap
                float push_force = 0.1f;
                float push_x = (dx / distance) * push_force;
                float push_z = (dz / distance) * push_force;
                
                p1->x += push_x;
                p1->z += push_z;
                p2->x -= push_x;
                p2->z -= push_z;
            }
        }
    }
}

// Apply arena physics and hazards
void apply_arena_physics(void) {
    game.arena_shrink_timer++;
    
    // Shrink arena every 3600 frames (60 seconds at 60fps)
    if (game.arena_shrink_timer > 3600) {
        game.arena_bounds *= 0.9f;
        game.arena_shrink_timer = 0;
        printf("[ARENA] Arena shrinks! New bounds: %.1f\n", game.arena_bounds);
        
        // Damage players outside bounds
        for (int i = 0; i < game.num_players; i++) {
            UnifiedPlayer* player = &game.players[i];
            if (fabsf(player->x) > game.arena_bounds || fabsf(player->z) > game.arena_bounds) {
                player->health -= 5;
                printf("[HAZARD] Player %d takes arena damage! Health: %d\n", i, player->health);
            }
        }
    }
}

// Update the entire unified game
void update_unified_game(float delta_time) {
    // Update avatar menu if active
    if (game.in_avatar_menu && game.avatar_menu) {
        avatar_menu_update(game.avatar_menu, delta_time);
        return;
    }

    if (game.in_menu) return;

    game.frame_count++;
    
    // Update local player input
    UnifiedPlayer* local_player = &game.players[game.local_player_id];
    PlayerInput* input = &local_player->input;
    
    // Map keyboard to input
    input->move_forward = keys['W'];
    input->move_backward = keys['S'];
    input->move_left = keys['A'];
    input->move_right = keys['D'];
    input->sprint = keys[VK_SHIFT];
    input->block = keys[VK_SPACE];
    input->attack_light = keys[VK_LBUTTON];
    input->attack_heavy = keys[VK_RBUTTON];
    input->jump = keys[VK_CONTROL];
    
    // Handle input for all players
    for (int i = 0; i < game.num_players; i++) {
        if (game.players[i].is_local_player) {
            handle_player_input(i, &game.players[i].input);
        }
        update_player_physics(&game.players[i], delta_time);
        update_player_combat(&game.players[i], delta_time);

        // Update avatar animation
        if (game.players[i].avatar_initialized && game.players[i].avatar) {
            gangbeast_avatar_update_animation(game.players[i].avatar, delta_time);
        }
    }
    
    // Check collisions
    check_player_collisions();
    
    // Apply arena physics
    apply_arena_physics();
    
    // Update multiplayer engine
    if (game.multiplayer_initialized) {
        update_multiplayer_engine(delta_time);
    }
    
    // Update camera to follow local player
    if (game.follow_player && game.local_player_id < game.num_players) {
        UnifiedPlayer* target = &game.players[game.local_player_id];
        game.cam_x = target->x;
        game.cam_z = target->z + game.cam_distance;
    }
}

// Render a single player
void render_player(UnifiedPlayer* player) {
    if (player->lives <= 0) return; // Don't render eliminated players

    // Use gangbeast avatar if available, otherwise fallback to simple rendering
    if (player->avatar_initialized && player->avatar) {
        // Update avatar animation based on player state
        if (player->attack_animation > 0) {
            gangbeast_avatar_set_animation(player->avatar, ANIM_ATTACKING);
        } else if (fabsf(player->vel_x) > 0.1f || fabsf(player->vel_z) > 0.1f) {
            gangbeast_avatar_set_animation(player->avatar, ANIM_WALKING);
        } else {
            gangbeast_avatar_set_animation(player->avatar, ANIM_IDLE);
        }

        // Render the gangbeast avatar
        gangbeast_avatar_render(player->avatar,
                              player->x, player->y, player->z,
                              player->facing_angle,
                              DETAIL_HIGH);
    } else {
        // Fallback to simple rendering
        glPushMatrix();
        glTranslatef(player->x, player->y + 1.0f, player->z);
        glRotatef(player->facing_angle * 180.0f / M_PI, 0, 1, 0);

        // Color based on fighting style
        switch (player->fighting_style) {
            case 0: glColor3f(1.0f, 0.2f, 0.2f); break; // Brawler - Red
            case 1: glColor3f(0.2f, 1.0f, 0.2f); break; // Striker - Green
            case 2: glColor3f(0.2f, 0.2f, 1.0f); break; // Phantom - Blue
            case 3: glColor3f(1.0f, 1.0f, 0.2f); break; // Titan - Yellow
            default: glColor3f(0.8f, 0.8f, 0.8f); break;
        }

        // Simple cube body
        glPushMatrix();
        glScalef(0.5f, 1.0f, 0.3f);
        glutSolidCube(1.0f);
        glPopMatrix();

        glPopMatrix();
    }
    
    // Render effects
    if (player->attack_animation > 0) {
        glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glPushMatrix();
        glTranslatef(0, 0, -0.8f);
        glScalef(2.0f, 0.1f, 2.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        glDisable(GL_BLEND);
    }
    
    if (player->block_animation > 0) {
        glColor4f(0.0f, 0.0f, 1.0f, 0.3f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glPushMatrix();
        glScalef(1.5f, 1.5f, 1.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        glDisable(GL_BLEND);
    }
    
    glPopMatrix();
}

// Render the arena
void render_arena(void) {
    // Ground plane
    glColor3f(0.3f, 0.5f, 0.3f);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-ARENA_SIZE*2, 0, -ARENA_SIZE*2);
    glVertex3f(ARENA_SIZE*2, 0, -ARENA_SIZE*2);
    glVertex3f(ARENA_SIZE*2, 0, ARENA_SIZE*2);
    glVertex3f(-ARENA_SIZE*2, 0, ARENA_SIZE*2);
    glEnd();
    
    // Arena boundaries
    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-game.arena_bounds, 0.1f, -game.arena_bounds);
    glVertex3f(game.arena_bounds, 0.1f, -game.arena_bounds);
    glVertex3f(game.arena_bounds, 0.1f, game.arena_bounds);
    glVertex3f(-game.arena_bounds, 0.1f, game.arena_bounds);
    glEnd();
    
    // Grid lines for reference
    glColor3f(0.4f, 0.4f, 0.4f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = -20; i <= 20; i += 2) {
        glVertex3f(i, 0.01f, -20);
        glVertex3f(i, 0.01f, 20);
        glVertex3f(-20, 0.01f, i);
        glVertex3f(20, 0.01f, i);
    }
    glEnd();
}

// Render HUD and UI
void render_hud(void) {
    // Switch to orthographic projection for HUD
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    // Render player stats
    UnifiedPlayer* player = &game.players[game.local_player_id];
    
    // Health bar
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(20, 20);
    glVertex2f(20 + (player->health * 200.0f / player->max_health), 20);
    glVertex2f(20 + (player->health * 200.0f / player->max_health), 40);
    glVertex2f(20, 40);
    glEnd();
    
    // Stamina bar
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(20, 50);
    glVertex2f(20 + (player->stamina * 200.0f / player->max_stamina), 50);
    glVertex2f(20 + (player->stamina * 200.0f / player->max_stamina), 70);
    glVertex2f(20, 70);
    glEnd();
    
    // Combo meter
    if (player->combo_meter > 0) {
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(20, 80);
        glVertex2f(20 + (player->combo_meter * 200.0f / 100.0f), 80);
        glVertex2f(20 + (player->combo_meter * 200.0f / 100.0f), 100);
        glVertex2f(20, 100);
        glEnd();
    }
    
    // Controls text
    glColor3f(1.0f, 1.0f, 1.0f);
    // Note: For simplicity, not implementing text rendering here
    // In a full implementation, you'd use bitmap fonts or texture-based text
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Render the complete unified game
void render_unified_game(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (game.in_avatar_menu && game.avatar_menu) {
        // Render avatar customization menu
        avatar_menu_render(game.avatar_menu);
    } else if (game.in_menu) {
        // Menu rendering
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        // Menu background
        glColor3f(0.1f, 0.1f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glVertex2f(0, WINDOW_HEIGHT);
        glEnd();

        // Style selection (simplified visual representation)
        glColor3f(1.0f, 1.0f, 1.0f);

        for (int i = 0; i < 4; i++) {
            if (i == game.selected_style) {
                glColor3f(1.0f, 1.0f, 0.0f); // Selected
            } else {
                glColor3f(1.0f, 1.0f, 1.0f); // Normal
            }

            // Draw selection indicator (simple rectangle)
            float y = 200 + i * 50;
            glBegin(GL_QUADS);
            glVertex2f(100, y);
            glVertex2f(300, y);
            glVertex2f(300, y + 30);
            glVertex2f(100, y + 30);
            glEnd();
        }
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    } else {
        // Game rendering
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Camera setup
        float cam_x = game.cam_x + sin(game.cam_yaw * M_PI / 180.0f) * game.cam_distance;
        float cam_z = game.cam_z + cos(game.cam_yaw * M_PI / 180.0f) * game.cam_distance;
        float cam_y = game.cam_y + sin(game.cam_pitch * M_PI / 180.0f) * game.cam_distance;
        
        gluLookAt(cam_x, cam_y, cam_z, game.cam_x, game.cam_y, game.cam_z, 0, 1, 0);
        
        // Lighting setup
        float light_pos[] = {10.0f, 20.0f, 10.0f, 1.0f};
        float light_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color);
        
        // Render arena
        render_arena();
        
        // Render all players
        for (int i = 0; i < game.num_players; i++) {
            render_player(&game.players[i]);
        }
        
        // Render HUD
        render_hud();
    }
    
    SwapBuffers(g_hDC);
}

// Window procedure for unified system
LRESULT CALLBACK UnifiedWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            return 0;
            
        case WM_SIZE:
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_KEYDOWN:
            keys[wParam] = TRUE;
            
            if (game.in_avatar_menu && game.avatar_menu) {
                // Handle avatar menu input
                switch (wParam) {
                    case VK_UP:
                        avatar_menu_handle_key_up(game.avatar_menu);
                        break;
                    case VK_DOWN:
                        avatar_menu_handle_key_down(game.avatar_menu);
                        break;
                    case VK_LEFT:
                        avatar_menu_handle_key_left(game.avatar_menu);
                        break;
                    case VK_RIGHT:
                        avatar_menu_handle_key_right(game.avatar_menu);
                        break;
                    case VK_RETURN:
                        avatar_menu_handle_key_enter(game.avatar_menu);
                        if (!avatar_menu_is_active(game.avatar_menu)) {
                            // Avatar customization complete, apply to player
                            UnifiedPlayer* player = &game.players[game.local_player_id];
                            if (player->avatar) {
                                gangbeast_avatar_destroy(player->avatar);
                            }
                            player->avatar = avatar_menu_create_final_avatar(game.avatar_menu);
                            player->avatar_initialized = (player->avatar != NULL);

                            game.in_avatar_menu = 0;
                            printf("[UNIFIED] Avatar customization complete\n");
                        }
                        break;
                    case VK_ESCAPE:
                        avatar_menu_handle_key_escape(game.avatar_menu);
                        if (!avatar_menu_is_active(game.avatar_menu)) {
                            game.in_avatar_menu = 0;
                        }
                        break;
                }
            } else if (game.in_menu) {
                switch (wParam) {
                    case VK_UP:
                        game.selected_style = (game.selected_style - 1 + 4) % 4;
                        break;
                    case VK_DOWN:
                        game.selected_style = (game.selected_style + 1) % 4;
                        break;
                    case VK_RETURN:
                        // Start avatar customization instead of directly starting game
                        game.in_avatar_menu = 1;
                        avatar_menu_show(game.avatar_menu);
                        printf("[UNIFIED] Entering avatar customization\n");
                        break;
                    case 'C':
                        // Quick start with current style (skip avatar customization)
                        game.players[game.local_player_id].fighting_style = game.selected_style;
                        game.in_menu = 0;
                        printf("[UNIFIED] Game started! Fighting style: %d\n", game.selected_style);

                        // Initialize ragdoll for local player if multiplayer is available
                        if (game.multiplayer_initialized) {
                            UnifiedPlayer* player = &game.players[game.local_player_id];
                            player->ragdoll = coreria_spawn_ragdoll(player->fighting_style, 1, player->x, player->y, player->z);
                            if (player->ragdoll.id != 0) {
                                player->ragdoll_active = 1;
                                printf("[UNIFIED] Ragdoll physics enabled for local player\n");
                            }
                        }
                        break;
                    case VK_ESCAPE:
                        PostQuitMessage(0);
                        break;
                }
            } else {
                switch (wParam) {
                    case VK_ESCAPE:
                        game.in_menu = 1;
                        break;
                    case VK_F1:
                        // Add AI opponent for testing
                        if (game.num_players < MAX_PLAYERS) {
                            int new_id = game.num_players++;
                            UnifiedPlayer* ai_player = &game.players[new_id];
                            ai_player->player_id = new_id;
                            ai_player->is_local_player = 0;
                            ai_player->x = (rand() % 20) - 10;
                            ai_player->z = (rand() % 20) - 10;
                            ai_player->y = 0.0f;
                            ai_player->max_health = 100;
                            ai_player->health = ai_player->max_health;
                            ai_player->lives = 3;
                            ai_player->max_stamina = 100;
                            ai_player->stamina = ai_player->max_stamina;
                            ai_player->fighting_style = rand() % 4;

                            // Create avatar for AI player
                            ai_player->avatar = create_avatar_for_style(ai_player->fighting_style);
                            ai_player->avatar_initialized = (ai_player->avatar != NULL);

                            printf("[UNIFIED] Added AI player %d at (%.1f, %.1f) with style %d\n",
                                   new_id, ai_player->x, ai_player->z, ai_player->fighting_style);
                        }
                        break;
                }
            }
            return 0;
            
        case WM_KEYUP:
            keys[wParam] = FALSE;
            return 0;
            
        case WM_LBUTTONDOWN:
            keys[VK_LBUTTON] = TRUE;
            return 0;
            
        case WM_LBUTTONUP:
            keys[VK_LBUTTON] = FALSE;
            return 0;
            
        case WM_RBUTTONDOWN:
            keys[VK_RBUTTON] = TRUE;
            return 0;
            
        case WM_RBUTTONUP:
            keys[VK_RBUTTON] = FALSE;
            return 0;
            
        case WM_MOUSEMOVE:
            mouse_x = LOWORD(lParam);
            mouse_y = HIWORD(lParam);
            return 0;
            
        case WM_DESTROY:
            cleanup_unified_system();
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

// Cleanup the unified system
void cleanup_unified_system(void) {
    printf("[UNIFIED] Cleaning up system...\n");

    // Cleanup avatar system
    for (int i = 0; i < game.num_players; i++) {
        if (game.players[i].avatar) {
            gangbeast_avatar_destroy(game.players[i].avatar);
            game.players[i].avatar = NULL;
        }
    }

    if (game.avatar_menu) {
        avatar_menu_destroy(game.avatar_menu);
        game.avatar_menu = NULL;
    }

    avatar_menu_cleanup();
    gangbeast_avatar_cleanup();

    // Cleanup multiplayer
    if (game.multiplayer_initialized) {
        cleanup_multiplayer_engine();
    }
    
    // Cleanup OpenGL
    if (g_hRC) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hRC);
        g_hRC = NULL;
    }
    
    if (g_hDC) {
        ReleaseDC(g_hWnd, g_hDC);
        g_hDC = NULL;
    }
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    printf("=== CHAOSFORGE UNIFIED ENGINE ===\n");
    printf("Real-time player-controlled combat arena\n");
    printf("Physics + Game + Movement engines integrated\n\n");
    
    // Register window class
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = UnifiedWndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = "ChaosForgeUnified";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Failed to register window class", "Error", MB_OK);
        return 1;
    }
    
    // Create window
    g_hWnd = CreateWindowA("ChaosForgeUnified", "ChaosForge Unified Arena",
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
    
    if (!g_hWnd) {
        MessageBoxA(NULL, "Failed to create window", "Error", MB_OK);
        return 1;
    }
    
    // Initialize unified system
    if (!init_unified_system(g_hWnd)) {
        MessageBoxA(NULL, "Failed to initialize unified system", "Error", MB_OK);
        return 1;
    }
    
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    printf("\n=== CONTROLS ===\n");
    printf("MENU: UP/DOWN - Select style, ENTER - Start game\n");
    printf("GAME: WASD - Move, SHIFT - Sprint, SPACE - Block\n");
    printf("      Left Click - Light attack, Right Click - Heavy attack\n");
    printf("      F1 - Add AI opponent, ESC - Back to menu\n\n");
    
    // Main game loop
    MSG msg;
    LARGE_INTEGER frequency, last_time, current_time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&last_time);
    
    while (game.game_running) {
        // Handle Windows messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                game.game_running = 0;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Calculate delta time
        QueryPerformanceCounter(&current_time);
        float delta_time = (float)(current_time.QuadPart - last_time.QuadPart) / (float)frequency.QuadPart;
        last_time = current_time;
        
        // Update and render
        update_unified_game(delta_time);
        render_unified_game();
        
        Sleep(1); // Prevent 100% CPU usage
    }
    
    cleanup_unified_system();
    return 0;
}