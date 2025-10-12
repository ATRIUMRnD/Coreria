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

// Constants
#define MAX_PARTICLES 64
#define MAX_DYNAMIC_OBJECTS 16
#define MAX_LOG_LINES 32
#define LOG_LINE_LENGTH 128

// Window dimensions
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Particle system
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    int life;
    int type; // 0: attack, 1: respawn
} Particle;

// Dynamic objects
typedef struct {
    float x, z;
    int style;
} DynamicObject;

// Global variables
HWND g_hWnd;
HDC g_hDC;
HGLRC g_hRC;

Particle particles[MAX_PARTICLES];
int particle_count = 0;
DynamicObject dynamic_objects[MAX_DYNAMIC_OBJECTS];
int dynamic_object_count = 0;
char game_logs[MAX_LOG_LINES][LOG_LINE_LENGTH];
int log_count = 0;

// Camera and input state
float cam_angle = 0.0f;
float cam_radius = 18.0f;
float cam_height = 8.0f;
float cam_pan_x = 0.0f, cam_pan_z = 0.0f;
float target_x = 0.0f, target_z = 0.0f;
int cam_follow_player = 0;
int mouse_left_down = 0;
int mouse_right_down = 0;
int mouse_last_x = -1, mouse_last_y = -1;
int show_console = 0;

// Player movement state
float player_vel_x = 0.0f, player_vel_z = 0.0f;
float player_speed = 0.15f;
float player_friction = 0.9f;
int player_can_move = 0;

// Game state
const char* fighting_styles[] = {"Brawler", "Striker", "Phantom", "Titan"};
int selected_style = 0;
int in_menu = 1;
float player_x = 0.0f, player_z = -5.0f;
int player_style_global = 0;
int running = 1;

// Game state variables
GameState game_state;
int game_tick = 0;
int training_phase = 1;
int master_style = 0;
int passed_exam = 0;
int training_ticks = 0;

// Key state tracking
BOOL keys[256] = {FALSE};

// Function prototypes
void spawn_particles(float x, float z, int type);
void update_particles(void);
void draw_particles(void);
int add_log(const char* msg);
void render_logs(void);
void draw_background(void);
void draw_menu(void);
void draw_base_plate(void);
void draw_player(float x, float z, int style, int is_master, int health, int attack_anim, int respawn_anim, int lives);
void draw_dynamic_objects(void);
void render_scene(GameState* state, int training_phase, int master_style, int player_style);
void copy_to_clipboard(const char* text);
void glut_sphere_approx(float radius, int subdivisions);
void update_game_logic(void);
void update_player_movement(void);
void handle_mouse_input(int x, int y);
void render_frame(void);
BOOL init_opengl(HWND hWnd);
void cleanup_opengl(void);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void spawn_particles(float x, float z, int type) {
    for (int i = 0; i < 8 && particle_count < MAX_PARTICLES; ++i) {
        float angle = (float)i * (2 * M_PI / 8);
        particles[particle_count].x = x;
        particles[particle_count].y = 1.0f;
        particles[particle_count].z = z;
        particles[particle_count].vx = cos(angle) * 0.5f;
        particles[particle_count].vy = 1.0f;
        particles[particle_count].vz = sin(angle) * 0.5f;
        particles[particle_count].life = 20;
        particles[particle_count].type = type;
        particle_count++;
    }
}

void update_particles(void) {
    for (int i = 0; i < particle_count; ++i) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].z += particles[i].vz;
        particles[i].vy -= 0.05f; // gravity
        particles[i].life--;
        if (particles[i].life <= 0) {
            // Remove dead particle by swapping with last
            particles[i] = particles[particle_count-1];
            particle_count--;
            i--;
        }
    }
}

void draw_particles(void) {
    for (int i = 0; i < particle_count; ++i) {
        if (particles[i].type == 0) glColor3f(1.0f, 0.2f, 0.2f); // attack
        else glColor3f(1.0f, 1.0f, 0.5f); // respawn
        glPushMatrix();
        glTranslatef(particles[i].x, particles[i].y, particles[i].z);
        glut_sphere_approx(0.2f, 8);
        glPopMatrix();
    }
}

int add_log(const char* msg) {
    if (log_count < MAX_LOG_LINES) {
        strncpy(game_logs[log_count], msg, LOG_LINE_LENGTH-1);
        game_logs[log_count][LOG_LINE_LENGTH-1] = '\0';
        log_count++;
        return 1;
    } else {
        // If full, shift logs up and add new
        for (int i = 1; i < MAX_LOG_LINES; ++i) {
            strncpy(game_logs[i-1], game_logs[i], LOG_LINE_LENGTH);
        }
        strncpy(game_logs[MAX_LOG_LINES-1], msg, LOG_LINE_LENGTH-1);
        game_logs[MAX_LOG_LINES-1][LOG_LINE_LENGTH-1] = '\0';
        return 1;
    }
}

void render_logs(void) {
    // Console output for now
    if (show_console) {
        static int last_log_count = 0;
        if (log_count != last_log_count) {
            printf("[CONSOLE] Logs:\n");
            for (int i = 0; i < log_count; ++i) {
                printf("  %s\n", game_logs[i]);
            }
            last_log_count = log_count;
        }
    }
}

void glut_sphere_approx(float radius, int subdivisions) {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 8; ++i) {
        float theta1 = i * M_PI / 4;
        float theta2 = (i+1) * M_PI / 4;
        glVertex3f(0, 0, 0);
        glVertex3f(radius * cos(theta1), radius * sin(theta1), 0);
        glVertex3f(radius * cos(theta2), radius * sin(theta2), 0);
    }
    glEnd();
}

void draw_background(void) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.1f, 0.3f); glVertex2i(0, 0);
    glColor3f(0.2f, 0.2f, 0.5f); glVertex2i(WINDOW_WIDTH, 0);
    glColor3f(0.3f, 0.3f, 0.7f); glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT);
    glColor3f(0.2f, 0.2f, 0.5f); glVertex2i(0, WINDOW_HEIGHT);
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void draw_menu(void) {
    // Simple menu - draw colored rectangles to represent menu items
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Draw menu background
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(200, 150);
    glVertex2f(600, 150);
    glVertex2f(600, 450);
    glVertex2f(200, 450);
    glEnd();

    // Draw fighting style options as colored rectangles
    for (int i = 0; i < 4; i++) {
        if (i == selected_style) {
            glColor3f(1.0f, 1.0f, 0.0f); // Yellow for selected
        } else {
            switch (i) {
                case 0: glColor3f(1.0f, 0.2f, 0.2f); break; // Brawler - Red
                case 1: glColor3f(0.2f, 1.0f, 0.2f); break; // Striker - Green
                case 2: glColor3f(0.2f, 0.2f, 1.0f); break; // Phantom - Blue
                case 3: glColor3f(1.0f, 1.0f, 0.2f); break; // Titan - Yellow
            }
        }
        float y = 380 - i * 60;
        glBegin(GL_QUADS);
        glVertex2f(250, y - 20);
        glVertex2f(550, y - 20);
        glVertex2f(550, y + 20);
        glVertex2f(250, y + 20);
        glEnd();
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // Print menu to console
    static int last_selected = -1;
    if (selected_style != last_selected) {
        printf("\n=== CHAOSFORGE ARENA ===\n");
        for (int i = 0; i < 4; i++) {
            printf("%s %s\n", (i == selected_style) ? ">" : " ", fighting_styles[i]);
        }
        printf("Use UP/DOWN to select, ENTER to start\n");
        last_selected = selected_style;
    }
}

void draw_base_plate(void) {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-20, 0, -20);
    glVertex3f(20, 0, -20);
    glVertex3f(20, 0, 20);
    glVertex3f(-20, 0, 20);
    glEnd();
    
    // Grid lines
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_LINES);
    for (int i = -20; i <= 20; i += 5) {
        glVertex3f(i, 0.01f, -20);
        glVertex3f(i, 0.01f, 20);
        glVertex3f(-20, 0.01f, i);
        glVertex3f(20, 0.01f, i);
    }
    glEnd();
}

void draw_player(float x, float z, int style, int is_master, int health, int attack_anim, int respawn_anim, int lives) {
    (void)is_master;
    (void)lives;
    glPushMatrix();
    glTranslatef(x, 1.0f, z);

    // Color based on style
    switch (style) {
        case 0: glColor3f(1.0f, 0.2f, 0.2f); break; // Brawler - Red
        case 1: glColor3f(0.2f, 1.0f, 0.2f); break; // Striker - Green
        case 2: glColor3f(0.2f, 0.2f, 1.0f); break; // Phantom - Blue
        case 3: glColor3f(1.0f, 1.0f, 0.2f); break; // Titan - Yellow
        default: glColor3f(0.8f, 0.8f, 0.8f); break;
    }

    // Flash effects
    if (attack_anim > 0) {
        glColor3f(1.0f, 0.5f, 0.5f); // Red flash for attack
    }
    if (respawn_anim > 0) {
        glColor3f(1.0f, 1.0f, 0.5f); // Yellow flash for respawn
    }

    // Draw simple cube for player
    float size = 0.75f;
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);
    // Back face
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, -size, -size);
    // Top face
    glVertex3f(-size, size, -size);
    glVertex3f(-size, size, size);
    glVertex3f(size, size, size);
    glVertex3f(size, size, -size);
    // Bottom face
    glVertex3f(-size, -size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, size);
    // Right face
    glVertex3f(size, -size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    glVertex3f(size, -size, size);
    // Left face
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glVertex3f(-size, size, size);
    glVertex3f(-size, size, -size);
    glEnd();

    // Health bar above player
    glTranslatef(0, 2.0f, 0);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.8f, 0.1f, 0);
    glVertex3f(0.8f, 0.1f, 0);
    glVertex3f(0.8f, 0.3f, 0);
    glVertex3f(-0.8f, 0.3f, 0);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    float health_width = 1.6f * (health / 100.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.8f, 0.1f, 0);
    glVertex3f(-0.8f + health_width, 0.1f, 0);
    glVertex3f(-0.8f + health_width, 0.3f, 0);
    glVertex3f(-0.8f, 0.3f, 0);
    glEnd();

    glPopMatrix();
}

void copy_to_clipboard(const char* text) {
    if (OpenClipboard(g_hWnd)) {
        EmptyClipboard();
        HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, strlen(text) + 1);
        if (hClipboardData) {
            char* pchData = (char*)GlobalLock(hClipboardData);
            strcpy(pchData, text);
            GlobalUnlock(hClipboardData);
            SetClipboardData(CF_TEXT, hClipboardData);
        }
        CloseClipboard();
    }
}

void draw_dynamic_objects(void) {
    for (int i = 0; i < dynamic_object_count; ++i) {
        draw_player(dynamic_objects[i].x, dynamic_objects[i].z, dynamic_objects[i].style, 0, 100, 0, 0, 2);
    }
}

void render_scene(GameState* state, int training_phase, int master_style, int player_style) {
    // Draw particles
    draw_particles();
    // Camera setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)WINDOW_WIDTH/(double)WINDOW_HEIGHT, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Set camera target based on game state
    float cam_target_x = 0.0f, cam_target_z = 0.0f;
    if (training_phase && !in_menu) {
        // In training mode, center on player position
        cam_target_x = player_x;
        cam_target_z = player_z;
    } else if (cam_follow_player && state) {
        // Follow player 0 in main game
        float angle = 0 * (2 * M_PI / MAX_PLAYERS);
        cam_target_x = 7.0f * cos(angle);
        cam_target_z = 7.0f * sin(angle);
    }
    
    // Apply camera panning
    cam_target_x += cam_pan_x;
    cam_target_z += cam_pan_z;
    
    float cam_x = cam_target_x + cam_radius * cos(cam_angle);
    float cam_y = cam_height;
    float cam_z = cam_target_z + cam_radius * sin(cam_angle);
    gluLookAt(cam_x, cam_y, cam_z, cam_target_x, 0.0f, cam_target_z, 0.0f, 1.0f, 0.0f);
    // Draw base plate
    draw_base_plate();
    if (training_phase) {
        draw_player(player_x, player_z, player_style, 0, 100, 0, 0, 2); // player
        draw_player(0.0f, 5.0f, master_style, 1, 100, 0, 0, 2);  // master
        draw_dynamic_objects();
    } else {
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            float angle = i * (2 * M_PI / MAX_PLAYERS);
            float x = 7.0f * cos(angle);
            float z = 7.0f * sin(angle);
            int is_bot = 2;
            int health = state->players[i].health;
            int attack_anim = state->attack_anim[i];
            int respawn_anim = state->respawn_anim[i];
            int lives = state->players[i].lives;
            draw_player(x, z, state->players[i].style, is_bot, health, attack_anim, respawn_anim, lives);
        }
        draw_dynamic_objects();
    }
}

void render_frame(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (in_menu) {
        // Set up 3D perspective for menu
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0, (double)WINDOW_WIDTH/(double)WINDOW_HEIGHT, 0.1, 100.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0, 5, 10, 0, 0, 0, 0, 1, 0);
        
        // Draw base plate in menu
        draw_base_plate();
        
        // Draw menu overlay
        draw_menu();
    } else if (training_phase && !passed_exam) {
        render_scene(NULL, 1, master_style, player_style_global);
    } else {
        render_scene(&game_state, 0, 0, 0);
    }

    if (show_console) {
        render_logs();
    }
}

void update_game_logic(void) {
    if (!running) return;

    if (in_menu) {
        // Just redraw menu
    } else if (training_phase && !passed_exam) {
        training_ticks++;
        printf("[DEBUG] Training phase: tick %d, player at (%.1f, %.1f)\n", training_ticks, player_x, player_z);
        if (training_ticks == 120) {
            add_log("[Training] You land a decisive blow! Master defeated. Ticket granted.");
            passed_exam = 1;
            training_phase = 0;

            // Initialize main game
            init_game_state(&game_state);
            for (int i = 0; i < MAX_PLAYERS; ++i) {
                game_state.players[i].style = player_style_global;
            }
            spawn_players(&game_state);
            add_log("[Time: 0.016s] Game loop started (60 FPS)");
            printf("[DEBUG] Main game started\n");
        }
    } else {
        // Main game loop
        update_game(&game_state, game_tick);

        // Update animations and particles
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            if (game_state.attack_anim[i] > 0) {
                game_state.attack_anim[i]--;
                if (game_state.attack_anim[i] == 9) {
                    float angle = i * (2 * M_PI / MAX_PLAYERS);
                    float x = 7.0f * cos(angle);
                    float z = 7.0f * sin(angle);
                    spawn_particles(x, z, 0);
                }
            }
            if (game_state.respawn_anim[i] > 0) {
                game_state.respawn_anim[i]--;
                if (game_state.respawn_anim[i] == 19) {
                    float angle = i * (2 * M_PI / MAX_PLAYERS);
                    float x = 7.0f * cos(angle);
                    float z = 7.0f * sin(angle);
                    spawn_particles(x, z, 1);
                }
            }
        }
        update_particles();
        game_tick++;
    }
}

void update_player_movement(void) {
    if (!player_can_move || in_menu) return;
    
    // Keyboard movement (WASD for player, Arrow keys for camera)
    float move_x = 0.0f, move_z = 0.0f;
    
    if (keys['W']) move_z -= player_speed;
    if (keys['S']) move_z += player_speed;
    if (keys['A']) move_x -= player_speed;
    if (keys['D']) move_x += player_speed;
    
    // Apply movement with momentum
    player_vel_x = player_vel_x * player_friction + move_x;
    player_vel_z = player_vel_z * player_friction + move_z;
    
    // Update player position
    player_x += player_vel_x;
    player_z += player_vel_z;
    
    // Keep player within bounds (-15 to 15)
    if (player_x < -15.0f) { player_x = -15.0f; player_vel_x = 0.0f; }
    if (player_x > 15.0f) { player_x = 15.0f; player_vel_x = 0.0f; }
    if (player_z < -15.0f) { player_z = -15.0f; player_vel_z = 0.0f; }
    if (player_z > 15.0f) { player_z = 15.0f; player_vel_z = 0.0f; }
    
    // Camera movement with arrow keys
    if (keys[VK_LEFT]) cam_angle -= 0.03f;
    if (keys[VK_RIGHT]) cam_angle += 0.03f;
    if (keys[VK_UP]) cam_height += 0.1f;
    if (keys[VK_DOWN]) cam_height -= 0.1f;
    
    // Limit camera height
    if (cam_height < 2.0f) cam_height = 2.0f;
    if (cam_height > 25.0f) cam_height = 25.0f;
}

void handle_mouse_input(int x, int y) {
    if (mouse_last_x == -1) {
        mouse_last_x = x;
        mouse_last_y = y;
        return;
    }
    
    int dx = x - mouse_last_x;
    int dy = y - mouse_last_y;
    
    // Right mouse button: camera rotation
    if (mouse_right_down) {
        cam_angle += dx * 0.01f;
        cam_height -= dy * 0.1f;
        if (cam_height < 2.0f) cam_height = 2.0f;
        if (cam_height > 25.0f) cam_height = 25.0f;
    }
    
    // Left mouse button: camera pan
    if (mouse_left_down) {
        cam_pan_x += dx * 0.05f;
        cam_pan_z += dy * 0.05f;
    }
    
    mouse_last_x = x;
    mouse_last_y = y;
}

BOOL init_opengl(HWND hWnd) {
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

    // Initialize OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    printf("[DEBUG] OpenGL initialized successfully.\n");
    
    return TRUE;
}

void cleanup_opengl(void) {
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static int last_keys[256] = {0};
    
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        case WM_KEYDOWN:
            keys[wParam] = TRUE;
            
            // Handle one-time key presses
            if (!last_keys[wParam]) {
                switch (wParam) {
                    case VK_RETURN:
                        if (in_menu) {
                            player_style_global = selected_style;
                            in_menu = 0;
                            player_can_move = 1;  // Enable player movement
                            printf("[DEBUG] Selected %s, starting game...\n", fighting_styles[selected_style]);
                            printf("[DEBUG] Movement controls: WASD to move, Arrow keys for camera, Mouse for camera control\n");
                        }
                        break;
                    case 'C':
                        if (in_menu) {
                            copy_to_clipboard(fighting_styles[selected_style]);
                            add_log("[Menu] Style copied to clipboard.");
                        }
                        break;
                    case VK_SPACE:
                        if (!in_menu && dynamic_object_count < MAX_DYNAMIC_OBJECTS) {
                            dynamic_objects[dynamic_object_count].x = player_x;
                            dynamic_objects[dynamic_object_count].z = player_z;
                            dynamic_objects[dynamic_object_count].style = player_style_global;
                            dynamic_object_count++;
                            add_log("[Game] New object spawned at player position.");
                        }
                        break;
                    case VK_F12:
                        show_console = !show_console;
                        printf("[DEBUG] Console toggled: %s\n", show_console ? "ON" : "OFF");
                        break;
                    case VK_UP:
                        if (in_menu) {
                            selected_style = (selected_style + 3) % 4;
                            printf("[DEBUG] Menu selection: %s\n", fighting_styles[selected_style]);
                        }
                        break;
                    case VK_DOWN:
                        if (in_menu) {
                            selected_style = (selected_style + 1) % 4;
                            printf("[DEBUG] Menu selection: %s\n", fighting_styles[selected_style]);
                        }
                        break;
                    case VK_ESCAPE:
                        running = 0;
                        PostQuitMessage(0);
                        break;
                }
            }
            last_keys[wParam] = 1;
            break;
            
        case WM_KEYUP:
            keys[wParam] = FALSE;
            last_keys[wParam] = 0;
            break;
            
        case WM_LBUTTONDOWN:
            mouse_left_down = 1;
            SetCapture(hWnd);
            break;
            
        case WM_LBUTTONUP:
            mouse_left_down = 0;
            ReleaseCapture();
            break;
            
        case WM_RBUTTONDOWN:
            mouse_right_down = 1;
            SetCapture(hWnd);
            break;
            
        case WM_RBUTTONUP:
            mouse_right_down = 0;
            ReleaseCapture();
            break;
            
        case WM_MOUSEMOVE:
            handle_mouse_input(LOWORD(lParam), HIWORD(lParam));
            break;
            
        case WM_MOUSEWHEEL:
            {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                cam_radius -= delta / 120.0f;  // 120 is standard wheel delta
                if (cam_radius < 5.0f) cam_radius = 5.0f;
                if (cam_radius > 50.0f) cam_radius = 50.0f;
                printf("[DEBUG] Camera zoom: %.1f\n", cam_radius);
            }
            break;
            
        case WM_SIZE:
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    printf("[DEBUG] Starting ChaosForge Arena...\n");

    // Register window class
    WNDCLASSEX wcex = {
        sizeof(WNDCLASSEX),
        CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        WndProc,
        0, 0,
        hInstance,
        NULL,
        LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL,
        "ChaosForgeArena",
        NULL
    };
    
    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_OK);
        return -1;
    }

    // Create window
    g_hWnd = CreateWindow(
        "ChaosForgeArena",
        "ChaosForge Arena",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL,
        hInstance,
        NULL
    );

    if (!g_hWnd) {
        MessageBox(NULL, "Failed to create window", "Error", MB_OK);
        return -1;
    }

    // Initialize OpenGL
    if (!init_opengl(g_hWnd)) {
        MessageBox(NULL, "Failed to initialize OpenGL", "Error", MB_OK);
        cleanup_opengl();
        return -1;
    }

    // Show window
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // Main loop
    MSG msg;
    DWORD lastTime = GetTickCount();
    
    while (running) {
        // Handle messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = 0;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Update player movement and camera
        update_player_movement();

        // Update and render at ~60 FPS
        DWORD currentTime = GetTickCount();
        if (currentTime - lastTime >= 16) { // ~60 FPS
            update_game_logic();
            render_frame();
            SwapBuffers(g_hDC);
            lastTime = currentTime;
        }
        
        Sleep(1); // Yield CPU
    }

    // Cleanup
    cleanup_opengl();
    return 0;
}