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

// Simple GLUT functions
void glutSolidCube(GLdouble size) {
    GLdouble half = size * 0.5;
    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-half, -half,  half);
    glVertex3f( half, -half,  half);
    glVertex3f( half,  half,  half);
    glVertex3f(-half,  half,  half);
    // Back face  
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half,  half, -half);
    glVertex3f( half,  half, -half);
    glVertex3f( half, -half, -half);
    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-half,  half, -half);
    glVertex3f(-half,  half,  half);
    glVertex3f( half,  half,  half);
    glVertex3f( half,  half, -half);
    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-half, -half, -half);
    glVertex3f( half, -half, -half);
    glVertex3f( half, -half,  half);
    glVertex3f(-half, -half,  half);
    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f( half, -half, -half);
    glVertex3f( half,  half, -half);
    glVertex3f( half,  half,  half);
    glVertex3f( half, -half,  half);
    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-half, -half, -half);
    glVertex3f(-half, -half,  half);
    glVertex3f(-half,  half,  half);
    glVertex3f(-half,  half, -half);
    glEnd();
}

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
    
} UnifiedPlayer;

// Global game state
typedef struct {
    UnifiedPlayer players[MAX_PLAYERS];
    int num_players;
    int local_player_id;
    
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
int last_mouse_x = -1, last_mouse_y = -1;
int mouse_captured = 0;
float mouse_sensitivity = 0.15f;
float cam_move_speed = 0.3f;

// Function prototypes
BOOL init_unified_system(HWND hWnd);
void cleanup_unified_system(void);
void update_unified_game(float delta_time);
void render_unified_game(void);
void handle_player_input(int player_id, PlayerInput* input);
void update_player_physics(UnifiedPlayer* player, float delta_time);
void update_player_combat(UnifiedPlayer* player);
void render_player(UnifiedPlayer* player);
void render_arena(void);
void render_hud(void);
void check_player_collisions(void);
void apply_arena_physics(void);
void update_free_camera(float delta_time);
void handle_mouse_look(int x, int y);
LRESULT CALLBACK UnifiedWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
    
    // Free camera setup (not following player by default)
    game.cam_x = 0.0f;
    game.cam_y = 8.0f;
    game.cam_z = 15.0f;
    game.cam_pitch = -15.0f;  // Looking down slightly (non-inverted)
    game.cam_yaw = 0.0f;      // Facing north
    game.follow_player = 0;   // Free camera by default
    
    game.in_menu = 1;
    game.selected_style = 0;
    game.game_running = 1;
    
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
    
    printf("[UNIFIED] System initialized successfully\n");
    return TRUE;
}

// Handle player input and movement
void handle_player_input(int player_id, PlayerInput* input) {
    if (player_id >= game.num_players) return;
    
    UnifiedPlayer* player = &game.players[player_id];
    
    // Movement calculations
    float move_speed = 0.15f;
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
    if (input->attack_light && player->stamina > 15 && player->attack_animation == 0) {
        player->attack_animation = 30; // 30 frame animation
        player->stamina -= 15;
        player->combo_meter += 10;
        printf("[PLAYER %d] Light attack! Stamina: %d\n", player_id, player->stamina);
    }
    
    if (input->attack_heavy && player->stamina > 25 && player->attack_animation == 0) {
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
}

// Update combat system
void update_player_combat(UnifiedPlayer* player) {
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
            float distance = sqrtf(dx*dx + dz*dz);
            
            // Attack collision (range = 2.5f)
            if (distance < 2.5f) {
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
                            // Respawn at random position
                            p2->x = ((rand() % 200) - 100) / 10.0f;
                            p2->z = ((rand() % 200) - 100) / 10.0f;
                            printf("[RESPAWN] Player %d respawns! Lives remaining: %d\n", j, p2->lives);
                        } else {
                            printf("[ELIMINATED] Player %d eliminated!\n", j);
                        }
                    }
                }
                
                // Push players apart to prevent overlap
                if (distance < 1.5f && distance > 0.1f) {
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

// Free camera system
void update_free_camera(float delta_time) {
    // Camera movement with WASD (when not controlling player)
    float move_speed = cam_move_speed;
    
    // Speed modifiers
    if (keys[VK_SHIFT]) move_speed *= 3.0f;  // Fast mode
    if (keys[VK_CONTROL]) move_speed *= 0.3f; // Slow mode
    
    // Calculate movement vectors based on camera rotation
    float yaw_rad = game.cam_yaw * M_PI / 180.0f;
    float forward_x = sinf(yaw_rad);
    float forward_z = cosf(yaw_rad);
    float right_x = cosf(yaw_rad);
    float right_z = -sinf(yaw_rad);
    
    // WASD movement (only when not in menu and not controlling player directly)
    if (!game.in_menu && !game.follow_player) {
        if (keys['W']) {
            game.cam_x += forward_x * move_speed;
            game.cam_z += forward_z * move_speed;
        }
        if (keys['S']) {
            game.cam_x -= forward_x * move_speed;
            game.cam_z -= forward_z * move_speed;
        }
        if (keys['A']) {
            game.cam_x += right_x * move_speed;
            game.cam_z += right_z * move_speed;
        }
        if (keys['D']) {
            game.cam_x -= right_x * move_speed;
            game.cam_z -= right_z * move_speed;
        }
        if (keys['Q'] || keys[VK_SPACE]) {
            game.cam_y += move_speed;  // Up
        }
        if (keys['E'] || keys['C']) {
            game.cam_y -= move_speed;  // Down
        }
    }
    
    // Constrain camera position
    if (game.cam_y < 0.5f) game.cam_y = 0.5f;
    if (game.cam_y > 50.0f) game.cam_y = 50.0f;
}

// Handle mouse look (non-inverted)
void handle_mouse_look(int x, int y) {
    if (last_mouse_x == -1) {
        last_mouse_x = x;
        last_mouse_y = y;
        return;
    }
    
    // Calculate mouse movement
    int dx = x - last_mouse_x;
    int dy = y - last_mouse_y;
    
    // Only apply mouse look when right mouse button is held or mouse is captured
    if (keys[VK_RBUTTON] || mouse_captured) {
        // Update camera rotation (non-inverted controls)
        game.cam_yaw += dx * mouse_sensitivity;
        game.cam_pitch += dy * mouse_sensitivity;  // Non-inverted: + for down movement
        
        // Constrain pitch to prevent flipping
        if (game.cam_pitch > 89.0f) game.cam_pitch = 89.0f;
        if (game.cam_pitch < -89.0f) game.cam_pitch = -89.0f;
        
        // Wrap yaw around 360 degrees
        while (game.cam_yaw >= 360.0f) game.cam_yaw -= 360.0f;
        while (game.cam_yaw < 0.0f) game.cam_yaw += 360.0f;
    }
    
    last_mouse_x = x;
    last_mouse_y = y;
}

// Update the entire unified game
void update_unified_game(float delta_time) {
    // Always update free camera
    update_free_camera(delta_time);
    
    if (game.in_menu) return;
    
    game.frame_count++;
    
    // Update local player input
    UnifiedPlayer* local_player = &game.players[game.local_player_id];
    PlayerInput* input = &local_player->input;
    
    // Map keyboard to input (WASD controls player when following, combat always available)
    if (game.follow_player) {
        // When following player, WASD controls the player
        input->move_forward = keys['W'];
        input->move_backward = keys['S'];
        input->move_left = keys['A'];
        input->move_right = keys['D'];
        input->sprint = keys[VK_SHIFT];
    } else {
        // When in free camera mode, player doesn't move with WASD
        input->move_forward = 0;
        input->move_backward = 0;
        input->move_left = 0;
        input->move_right = 0;
        input->sprint = 0;
    }
    
    // Combat controls always available
    input->block = keys[VK_SPACE];
    input->attack_light = keys[VK_LBUTTON];
    input->attack_heavy = keys[VK_RBUTTON];
    
    // Handle input for all players
    for (int i = 0; i < game.num_players; i++) {
        if (game.players[i].is_local_player) {
            handle_player_input(i, &game.players[i].input);
        }
        update_player_physics(&game.players[i], delta_time);
        update_player_combat(&game.players[i]);
    }
    
    // Check collisions
    check_player_collisions();
    
    // Apply arena physics
    apply_arena_physics();
    
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
    
    // Render player body (simple character)
    
    // Torso
    glPushMatrix();
    glScalef(0.5f, 1.0f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Head
    glPushMatrix();
    glTranslatef(0, 0.7f, 0);
    GLUquadric* quad = gluNewQuadric();
    gluSphere(quad, 0.3f, 8, 8);
    gluDeleteQuadric(quad);
    glPopMatrix();
    
    // Arms
    glPushMatrix();
    glTranslatef(-0.4f, 0.2f, 0);
    if (player->attack_animation > 0) {
        glRotatef(45.0f, 1, 0, 0); // Attack pose
    }
    glScalef(0.2f, 0.6f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.4f, 0.2f, 0);
    if (player->attack_animation > 0) {
        glRotatef(45.0f, 1, 0, 0); // Attack pose
    }
    glScalef(0.2f, 0.6f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Legs
    glPushMatrix();
    glTranslatef(-0.15f, -0.8f, 0);
    glScalef(0.2f, 0.8f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.15f, -0.8f, 0);
    glScalef(0.2f, 0.8f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Render effects
    if (player->attack_animation > 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
        
        glPushMatrix();
        glTranslatef(0, 0, -0.8f);
        glScalef(2.0f, 0.1f, 2.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        glDisable(GL_BLEND);
    }
    
    if (player->block_animation > 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 1.0f, 0.3f);
        
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
    glDisable(GL_LIGHTING);
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
        glVertex3f((float)i, 0.01f, -20.0f);
        glVertex3f((float)i, 0.01f, 20.0f);
        glVertex3f(-20.0f, 0.01f, (float)i);
        glVertex3f(20.0f, 0.01f, (float)i);
    }
    glEnd();
    glEnable(GL_LIGHTING);
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
    
    // Health bar background
    glColor3f(0.3f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(20, 20);
    glVertex2f(220, 20);
    glVertex2f(220, 40);
    glVertex2f(20, 40);
    glEnd();
    
    // Health bar
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(20, 20);
    glVertex2f(20 + (player->health * 200.0f / player->max_health), 20);
    glVertex2f(20 + (player->health * 200.0f / player->max_health), 40);
    glVertex2f(20, 40);
    glEnd();
    
    // Stamina bar background
    glColor3f(0.0f, 0.3f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(20, 50);
    glVertex2f(220, 50);
    glVertex2f(220, 70);
    glVertex2f(20, 70);
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
        glColor3f(0.3f, 0.3f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(20, 80);
        glVertex2f(220, 80);
        glVertex2f(220, 100);
        glVertex2f(20, 100);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(20, 80);
        glVertex2f(20 + (player->combo_meter * 200.0f / 100.0f), 80);
        glVertex2f(20 + (player->combo_meter * 200.0f / 100.0f), 100);
        glVertex2f(20, 100);
        glEnd();
    }
    
    // Lives indicator
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < player->lives; i++) {
        glBegin(GL_QUADS);
        glVertex2f(250 + i * 25, 20);
        glVertex2f(270 + i * 25, 20);
        glVertex2f(270 + i * 25, 40);
        glVertex2f(250 + i * 25, 40);
        glEnd();
    }
    
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
    
    if (game.in_menu) {
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
        
        // Title
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(400, 100);
        glVertex2f(800, 100);
        glVertex2f(800, 150);
        glVertex2f(400, 150);
        glEnd();
        
        // Style selection rectangles
        const char* style_names[] = {"BRAWLER", "STRIKER", "PHANTOM", "TITAN"};
        for (int i = 0; i < 4; i++) {
            if (i == game.selected_style) {
                glColor3f(1.0f, 1.0f, 0.0f); // Selected - Yellow
            } else {
                glColor3f(0.5f, 0.5f, 0.5f); // Normal - Gray
            }
            
            // Draw selection box
            float y = 250 + i * 80;
            glBegin(GL_QUADS);
            glVertex2f(400, y);
            glVertex2f(800, y);
            glVertex2f(800, y + 60);
            glVertex2f(400, y + 60);
            glEnd();
            
            // Draw inner box for contrast
            if (i == game.selected_style) {
                glColor3f(0.2f, 0.2f, 0.2f);
            } else {
                glColor3f(0.3f, 0.3f, 0.3f);
            }
            glBegin(GL_QUADS);
            glVertex2f(410, y + 10);
            glVertex2f(790, y + 10);
            glVertex2f(790, y + 50);
            glVertex2f(410, y + 50);
            glEnd();
        }
        
        // Instructions
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(300, 650);
        glVertex2f(900, 650);
        glVertex2f(900, 720);
        glVertex2f(300, 720);
        glEnd();
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    } else {
        // Game rendering
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Free camera setup
        float yaw_rad = game.cam_yaw * M_PI / 180.0f;
        float pitch_rad = game.cam_pitch * M_PI / 180.0f;
        
        // Calculate look direction based on yaw and pitch
        float look_x = game.cam_x + sinf(yaw_rad) * cosf(pitch_rad);
        float look_y = game.cam_y + sinf(pitch_rad);
        float look_z = game.cam_z + cosf(yaw_rad) * cosf(pitch_rad);
        
        gluLookAt(game.cam_x, game.cam_y, game.cam_z,  // Camera position
                  look_x, look_y, look_z,              // Look at point
                  0, 1, 0);                            // Up vector
        
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

// Simple AI for testing
void update_ai_player(UnifiedPlayer* player) {
    if (player->is_local_player) return;
    
    // Simple AI: move towards nearest player and attack when close
    float nearest_dist = 1000.0f;
    UnifiedPlayer* nearest_player = NULL;
    
    for (int i = 0; i < game.num_players; i++) {
        UnifiedPlayer* other = &game.players[i];
        if (other == player || other->lives <= 0) continue;
        
        float dx = other->x - player->x;
        float dz = other->z - player->z;
        float dist = sqrtf(dx*dx + dz*dz);
        
        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_player = other;
        }
    }
    
    if (nearest_player) {
        float dx = nearest_player->x - player->x;
        float dz = nearest_player->z - player->z;
        
        // Move towards target
        if (nearest_dist > 2.0f) {
            player->vel_x = (dx > 0) ? 0.1f : -0.1f;
            player->vel_z = (dz > 0) ? 0.1f : -0.1f;
        } else {
            // Attack when close
            if (player->attack_animation == 0 && player->stamina > 15) {
                player->attack_animation = 30;
                player->stamina -= 15;
            }
        }
        
        // Face the target
        player->facing_angle = atan2f(dx, dz);
    }
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
            
            if (game.in_menu) {
                switch (wParam) {
                    case VK_UP:
                        game.selected_style = (game.selected_style - 1 + 4) % 4;
                        break;
                    case VK_DOWN:
                        game.selected_style = (game.selected_style + 1) % 4;
                        break;
                    case VK_RETURN:
                        // Start game with selected style
                        game.players[game.local_player_id].fighting_style = game.selected_style;
                        game.in_menu = 0;
                        printf("[UNIFIED] Game started! Fighting style: %d\n", game.selected_style);
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
                    case VK_TAB:
                        // Toggle camera mode between free camera and following player
                        game.follow_player = !game.follow_player;
                        if (game.follow_player) {
                            printf("[CAMERA] Following player mode\n");
                        } else {
                            printf("[CAMERA] Free camera mode - Hold right mouse to look around\n");
                        }
                        break;
                    case VK_F1:
                        // Add AI opponent for testing
                        if (game.num_players < MAX_PLAYERS) {
                            int new_id = game.num_players++;
                            UnifiedPlayer* ai_player = &game.players[new_id];
                            memset(ai_player, 0, sizeof(UnifiedPlayer));
                            ai_player->player_id = new_id;
                            ai_player->is_local_player = 0;
                            ai_player->x = ((rand() % 200) - 100) / 10.0f;
                            ai_player->z = ((rand() % 200) - 100) / 10.0f;
                            ai_player->y = 0.0f;
                            ai_player->max_health = 100;
                            ai_player->health = ai_player->max_health;
                            ai_player->lives = 3;
                            ai_player->max_stamina = 100;
                            ai_player->stamina = ai_player->max_stamina;
                            ai_player->fighting_style = rand() % 4;
                            printf("[UNIFIED] Added AI player %d at (%.1f, %.1f)\n", new_id, ai_player->x, ai_player->z);
                        }
                        break;
                }
            }
            return 0;
            
        case WM_KEYUP:
            keys[wParam] = FALSE;
            // ESC key releases mouse capture in free camera mode
            if (wParam == VK_ESCAPE && !game.follow_player) {
                ReleaseCapture();
                ShowCursor(TRUE);
            }
            return 0;
            
        case WM_LBUTTONDOWN:
            if (!game.follow_player) {
                // In free camera mode, capture mouse for look control
                SetCapture(hWnd);
                ShowCursor(FALSE);
            }
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
            handle_mouse_look(mouse_x, mouse_y);
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
    
    srand((unsigned int)time(NULL));
    
    printf("=== CHAOSFORGE UNIFIED ENGINE ===\n");
    printf("Real-time player-controlled combat arena\n");
    printf("All engines integrated into one executable\n\n");
    
    // Register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = UnifiedWndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = "ChaosForgeUnified";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_OK);
        return 1;
    }
    
    // Create window
    g_hWnd = CreateWindow("ChaosForgeUnified", "ChaosForge Unified Arena",
                          WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                          WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
    
    if (!g_hWnd) {
        MessageBox(NULL, "Failed to create window", "Error", MB_OK);
        return 1;
    }
    
    // Initialize unified system
    if (!init_unified_system(g_hWnd)) {
        MessageBox(NULL, "Failed to initialize unified system", "Error", MB_OK);
        return 1;
    }
    
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    printf("\n=== CONTROLS ===\n");
    printf("MENU: UP/DOWN - Select style, ENTER - Start game\n");
    printf("GAME: WASD - Move (player following) / Camera (free mode)\n");
    printf("      SHIFT - Sprint, SPACE - Block\n");
    printf("      Left Click - Light attack, Right Click - Heavy attack\n");
    printf("      TAB - Toggle camera mode (Follow/Free)\n");
    printf("      ESC - Release mouse (free camera) / Back to menu\n");
    printf("      F1 - Add AI opponent\n\n");
    
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
        
        // Update AI players
        if (!game.in_menu) {
            for (int i = 0; i < game.num_players; i++) {
                update_ai_player(&game.players[i]);
            }
        }
        
        // Update and render
        update_unified_game(delta_time);
        render_unified_game();
        
        Sleep(16); // ~60 FPS
    }
    
    cleanup_unified_system();
    return 0;
}