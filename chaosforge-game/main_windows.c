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
#include "nanosvg.h"
#include "chaosforge_ragdoll.h"

// Constants
#define MAX_PARTICLES 64
#define MAX_DYNAMIC_OBJECTS 16
#define MAX_LOG_LINES 32
#define LOG_LINE_LENGTH 128

// Window dimensions
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
int window_width = WINDOW_WIDTH;
int window_height = WINDOW_HEIGHT;

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

// UE5-style camera system
float cam_pos_x = 0.0f, cam_pos_y = 8.0f, cam_pos_z = 18.0f;  // Camera position
float cam_yaw = 0.0f, cam_pitch = -15.0f;  // Camera rotation (degrees)
float cam_speed = 0.5f;  // Movement speed
float cam_sensitivity = 0.1f;  // Mouse sensitivity
int mouse_left_down = 0;
int mouse_right_down = 0;
int last_mouse_x = 0, last_mouse_y = 0;
int free_camera_mode = 1;  // Always use free camera in menu

// Legacy camera variables (for compatibility)
float cam_angle = 0.0f;
float cam_radius = 18.0f;
float cam_height = 8.0f;
float cam_pan_x = 0.0f, cam_pan_z = 0.0f;
float target_x = 0.0f, target_z = 0.0f;
int cam_follow_player = 0;
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

// Ragdoll system integration
int ragdoll_engine_initialized = 0;
RagdollHandle player_ragdoll = RAGDOLL_INIT();
int ragdoll_physics_enabled = 0;

// SVG graphics system
NSVGimage* menu_graphics = NULL;
NSVGimage* hud_graphics = NULL;

// Function prototypes
void spawn_particles(float x, float z, int type);
void update_particles(void);
void draw_particles(void);
int add_log(const char* msg);
void render_logs(void);
// Removed draw_background prototype
void draw_menu(void);
void draw_base_plate(void);
void draw_player(float x, float z, int style, int is_master, int health, int attack_anim, int respawn_anim, int lives);
void draw_dynamic_objects(void);
void render_scene(GameState* state, int training_phase, int master_style, int player_style);
void copy_to_clipboard(const char* text);
void glut_sphere_approx(float radius, int subdivisions);
void draw_bitmap_char(char c, float x, float y, float size);
void draw_text(const char* text, float x, float y, float size);
void draw_game_hud(void);
void update_game_logic(void);
void update_player_movement(void);
void update_ue5_camera(void);
void handle_mouse_input(int x, int y);
void render_frame(void);
void render_menu_orthographic(void);
void render_game_perspective(void);
// Removed draw_menu_background_gradient prototype
BOOL init_opengl(HWND hWnd);
void cleanup_opengl(void);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// SVG graphics functions
void init_svg_graphics(void);
void cleanup_svg_graphics(void);
void draw_svg_graphics(NSVGimage* image, float x, float y, float scale);
NSVGimage* create_menu_graphics(void);
NSVGimage* create_hud_graphics(void);

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
        for (int i = 1; i < MAX_LOG_LINES; ++i) {
            strncpy(game_logs[i-1], game_logs[i], LOG_LINE_LENGTH);
        }
        strncpy(game_logs[MAX_LOG_LINES-1], msg, LOG_LINE_LENGTH-1);
        game_logs[MAX_LOG_LINES-1][LOG_LINE_LENGTH-1] = '\0';
        return 1;
    }
}

void render_logs(void) {
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

static unsigned char font_data[256][8] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['A'] = {0x18, 0x24, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x00},
    ['B'] = {0x7C, 0x42, 0x42, 0x7C, 0x42, 0x42, 0x7C, 0x00},
    ['C'] = {0x3C, 0x42, 0x40, 0x40, 0x40, 0x42, 0x3C, 0x00},
    ['D'] = {0x78, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78, 0x00},
    ['E'] = {0x7E, 0x40, 0x40, 0x78, 0x40, 0x40, 0x7E, 0x00},
    ['F'] = {0x7E, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x00},
    ['G'] = {0x3C, 0x42, 0x40, 0x4E, 0x42, 0x42, 0x3C, 0x00},
    ['H'] = {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x00},
    ['I'] = {0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00},
    ['L'] = {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E, 0x00},
    ['M'] = {0x42, 0x66, 0x5A, 0x42, 0x42, 0x42, 0x42, 0x00},
    ['N'] = {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x00},
    ['O'] = {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
    ['P'] = {0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0x00},
    ['R'] = {0x7C, 0x42, 0x42, 0x7C, 0x44, 0x42, 0x41, 0x00},
    ['S'] = {0x3C, 0x42, 0x40, 0x3C, 0x02, 0x42, 0x3C, 0x00},
    ['T'] = {0x7F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00},
    ['U'] = {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00},
    ['W'] = {0x41, 0x41, 0x41, 0x49, 0x49, 0x55, 0x63, 0x00},
    ['w'] = {0x00, 0x00, 0x41, 0x49, 0x49, 0x55, 0x63, 0x00},
    ['l'] = {0x18, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 0x00},
    ['e'] = {0x00, 0x00, 0x3C, 0x42, 0x7E, 0x40, 0x3C, 0x00},
    ['r'] = {0x00, 0x00, 0x5C, 0x62, 0x40, 0x40, 0x40, 0x00},
    ['a'] = {0x00, 0x00, 0x3C, 0x02, 0x3E, 0x42, 0x3E, 0x00},
    ['k'] = {0x40, 0x40, 0x44, 0x48, 0x70, 0x48, 0x44, 0x00},
    ['t'] = {0x10, 0x10, 0x7C, 0x10, 0x10, 0x12, 0x0C, 0x00},
    ['i'] = {0x00, 0x08, 0x00, 0x18, 0x08, 0x08, 0x1C, 0x00},
    ['n'] = {0x00, 0x00, 0x58, 0x64, 0x44, 0x44, 0x44, 0x00},
    ['o'] = {0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00},
    ['m'] = {0x00, 0x00, 0x68, 0x54, 0x54, 0x54, 0x54, 0x00},
    ['h'] = {0x40, 0x40, 0x58, 0x64, 0x44, 0x44, 0x44, 0x00},
    ['s'] = {0x00, 0x00, 0x3C, 0x40, 0x38, 0x04, 0x78, 0x00},
    ['>'] = {0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x00},
    [':'] = {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00},
    ['/'] = {0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00},
    ['='] = {0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00},
    ['-'] = {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
    ['u'] = {0x00, 0x00, 0x44, 0x44, 0x44, 0x4C, 0x34, 0x00},
    ['p'] = {0x00, 0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40},
    ['d'] = {0x04, 0x04, 0x3C, 0x44, 0x44, 0x4C, 0x34, 0x00},
};

void draw_bitmap_char(char c, float x, float y, float size) {
    if (c < 0 || c > 255) c = ' ';
    
    glBegin(GL_POINTS);
    for (int row = 0; row < 8; row++) {
        unsigned char byte = font_data[(unsigned char)c][row];
        for (int col = 0; col < 8; col++) {
            if (byte & (0x80 >> col)) {
                glVertex2f(x + col * size, y - row * size);
            }
        }
    }
    glEnd();
}

void draw_text(const char* text, float x, float y, float size) {
    float current_x = x;
    glPointSize(size);
    
    for (int i = 0; text[i] != '\0'; i++) {
        draw_bitmap_char(text[i], current_x, y, size);
        current_x += 8 * size + size;
    }
}

// Removed draw_background() function - was another source of gray background

void draw_menu(void) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.05f, 0.05f, 0.2f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(100, 80);
    glVertex2f(700, 80);
    glVertex2f(700, 520);
    glVertex2f(100, 520);
    glEnd();

    glColor3f(0.3f, 0.3f, 0.6f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(100, 80);
    glVertex2f(700, 80);
    glVertex2f(700, 520);
    glVertex2f(100, 520);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    draw_text("=== CHAOSFORGE ARENA ===", 150, 470, 2.5f);

    glColor3f(0.8f, 0.8f, 0.8f);
    draw_text("Choose Your Fighting Style", 200, 430, 1.8f);

    for (int i = 0; i < 4; i++) {
        float y = 380 - i * 70;
        
        if (i == selected_style) {
            glColor3f(0.4f, 0.4f, 0.9f);
        } else {
            glColor3f(0.15f, 0.15f, 0.25f);
        }
        
        glBegin(GL_QUADS);
        glVertex2f(130, y - 15);
        glVertex2f(670, y - 15);
        glVertex2f(670, y + 35);
        glVertex2f(130, y + 35);
        glEnd();
        
        if (i == selected_style) {
            glColor3f(1.0f, 1.0f, 0.5f);
        } else {
            glColor3f(0.3f, 0.3f, 0.4f);
        }
        glBegin(GL_LINE_LOOP);
        glVertex2f(130, y - 15);
        glVertex2f(670, y - 15);
        glVertex2f(670, y + 35);
        glVertex2f(130, y + 35);
        glEnd();
        
        if (i == selected_style) {
            glColor3f(1.0f, 1.0f, 0.0f);
            draw_text(">>", 140, y, 2.2f);
        } else {
            glColor3f(0.5f, 0.5f, 0.5f);
            draw_text("  ", 140, y, 2.2f);
        }
        
        switch (i) {
            case 0: glColor3f(1.0f, 0.2f, 0.2f); break;
            case 1: glColor3f(0.2f, 1.0f, 0.2f); break;
            case 2: glColor3f(0.2f, 0.4f, 1.0f); break;
            case 3: glColor3f(1.0f, 0.9f, 0.1f); break;
        }
        
        draw_text(fighting_styles[i], 200, y, 2.2f);
        
        glColor3f(0.7f, 0.7f, 0.7f);
        const char* descriptions[] = {
            "Heavy & Powerful",
            "Fast & Precise", 
            "Agile & Ethereal",
            "Mighty & Unstoppable"
        };
        draw_text(descriptions[i], 400, y, 1.4f);
    }

    glColor3f(0.9f, 0.9f, 0.5f);
    draw_text("Use UP/DOWN to select, ENTER to start", 150, 120, 1.6f);
    
    glColor3f(0.6f, 0.6f, 0.6f);
    draw_text("ESC to exit", 320, 100, 1.2f);
    
    glDisable(GL_BLEND);

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
    if (in_menu) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.1f, 0.1f, 0.15f, 0.4f);
    glBegin(GL_QUADS);
    glVertex3f(-20, 0.0f, -20);
    glVertex3f(20, 0.0f, -20);
    glVertex3f(20, 0.0f, 20);
    glVertex3f(-20, 0.0f, 20);
    glEnd();

    glColor4f(0.25f, 0.25f, 0.3f, 0.6f);
    glBegin(GL_LINES);
    for (int i = -20; i <= 20; i += 5) {
        glVertex3f(i, 0.01f, -20);
        glVertex3f(i, 0.01f, 20);
        glVertex3f(-20, 0.01f, i);
        glVertex3f(20, 0.01f, i);
    }
    glEnd();

    glColor4f(0.4f, 0.4f, 0.5f, 0.8f);
    glBegin(GL_LINES);
    glVertex3f(-20, 0.02f, 0);
    glVertex3f(20, 0.02f, 0);
    glVertex3f(0, 0.02f, -20);
    glVertex3f(0, 0.02f, 20);
    glEnd();

    glDisable(GL_BLEND);
}

void draw_player(float x, float z, int style, int is_master, int health, int attack_anim, int respawn_anim, int lives) {
    (void)is_master;
    (void)lives;
    glPushMatrix();
    glTranslatef(x, 1.0f, z);

    switch (style) {
        case 0: glColor3f(1.0f, 0.2f, 0.2f); break;
        case 1: glColor3f(0.2f, 1.0f, 0.2f); break;
        case 2: glColor3f(0.2f, 0.2f, 1.0f); break;
        case 3: glColor3f(1.0f, 1.0f, 0.2f); break;
        default: glColor3f(0.8f, 0.8f, 0.8f); break;
    }

    if (attack_anim > 0) {
        glColor3f(1.0f, 0.5f, 0.5f);
    }
    if (respawn_anim > 0) {
        glColor3f(1.0f, 1.0f, 0.5f);
    }

    float size = 0.75f;
    glBegin(GL_QUADS);
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(-size, size, size);
    glVertex3f(size, size, size);
    glVertex3f(size, size, -size);
    glVertex3f(-size, -size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glVertex3f(-size, size, size);
    glVertex3f(-size, size, -size);
    glEnd();

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
    
    if (x == player_x && z == player_z && (player_vel_x != 0.0f || player_vel_z != 0.0f)) {
        glPushMatrix();
        glTranslatef(x, 0.1f, z);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(player_vel_x * 10, 0, player_vel_z * 10);
        glEnd();
        glPopMatrix();
    }
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
    if (in_menu) return;
    for (int i = 0; i < dynamic_object_count; ++i) {
        draw_player(dynamic_objects[i].x, dynamic_objects[i].z, dynamic_objects[i].style, 0, 100, 0, 0, 2);
    }
}

void render_scene(GameState* state, int training_phase, int master_style, int player_style) {
    draw_particles();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)window_width/(double)window_height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    float cam_target_x = 0.0f, cam_target_z = 0.0f;
    if (training_phase && !in_menu) {
        cam_target_x = player_x;
        cam_target_z = player_z;
    } else if (cam_follow_player && state) {
        float angle = 0 * (2 * M_PI / MAX_PLAYERS);
        cam_target_x = 7.0f * cos(angle);
        cam_target_z = 7.0f * sin(angle);
    }
    
    cam_target_x += cam_pan_x;
    cam_target_z += cam_pan_z;

    // Use UE5-style camera when in menu mode or free camera mode
    if (in_menu || free_camera_mode) {
        // Calculate look direction from yaw and pitch
        float look_x = cam_pos_x - sin(cam_yaw * M_PI / 180.0f) * cos(cam_pitch * M_PI / 180.0f);
        float look_y = cam_pos_y + sin(cam_pitch * M_PI / 180.0f);
        float look_z = cam_pos_z - cos(cam_yaw * M_PI / 180.0f) * cos(cam_pitch * M_PI / 180.0f);

        gluLookAt(cam_pos_x, cam_pos_y, cam_pos_z, look_x, look_y, look_z, 0.0f, 1.0f, 0.0f);
    } else {
        // Legacy camera system for gameplay
        float cam_x = cam_target_x + cam_radius * cos(cam_angle);
        float cam_y = cam_height;
        float cam_z = cam_target_z + cam_radius * sin(cam_angle);
        gluLookAt(cam_x, cam_y, cam_z, cam_target_x, 0.0f, cam_target_z, 0.0f, 1.0f, 0.0f);
    }
    draw_base_plate();
    if (training_phase) {
        draw_player(player_x, player_z, player_style, 0, 100, 0, 0, 2);
        draw_player(0.0f, 5.0f, master_style, 1, 100, 0, 0, 2);
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

// Removed draw_menu_background_gradient() function - was causing gray blob overlay

void render_menu_orthographic() {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, window_width, 0, window_height, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Removed draw_menu_background_gradient() to eliminate gray blob
    draw_menu();
    
    if (menu_graphics) {
        draw_svg_graphics(menu_graphics, 0, 0, 1.0f);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

void render_game_perspective() {
    glEnable(GL_DEPTH_TEST);
    if (training_phase && !passed_exam) {
        render_scene(NULL, 1, master_style, player_style_global);
    } else {
        render_scene(&game_state, 0, 0, 0);
    }
}

void render_frame(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Always render the 3D scene first (even in menu mode)
    render_game_perspective();

    if (in_menu) {
        // Overlay the menu on top of the 3D scene
        render_menu_orthographic();
    } else {
        draw_game_hud();
    }

    if (show_console) {
        render_logs();
    }
}

void update_game_logic(void) {
    if (!running) return;

    if (in_menu) {
        // No game logic in menu
    } else if (training_phase && !passed_exam) {
        training_ticks++;
        if (training_ticks == 120) {
            add_log("[Training] You land a decisive blow! Master defeated. Ticket granted.");
            passed_exam = 1;
            training_phase = 0;

            init_game_state(&game_state);
            for (int i = 0; i < MAX_PLAYERS; ++i) {
                game_state.players[i].style = player_style_global;
            }
            spawn_players(&game_state);
            add_log("[Time: 0.016s] Game loop started (60 FPS)");
        }
    } else {
        update_game(&game_state, game_tick);

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
    
    // Update ragdoll physics simulation
    if (ragdoll_engine_initialized && ragdoll_physics_enabled && !in_menu) {
        coreria_step_simulation(0.016f); // 60 FPS timestep
        
        // Update player position from ragdoll if active
        if (RAGDOLL_IS_VALID(player_ragdoll)) {
            float ragdoll_x, ragdoll_y, ragdoll_z;
            if (coreria_get_ragdoll_position(player_ragdoll, &ragdoll_x, &ragdoll_y, &ragdoll_z) == 0) {
                // Only update XZ position, keep the traditional player movement for Y
                player_x = ragdoll_x;
                player_z = ragdoll_z;
            }
        }
    }
}

void draw_game_hud(void) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(10, 480);
    glVertex2f(300, 480);
    glVertex2f(300, 590);
    glVertex2f(10, 590);
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    draw_text("Player Status:", 20, 575, 1.2f);
    
    char pos_text[64];
    snprintf(pos_text, sizeof(pos_text), "Position: %.1f, %.1f", player_x, player_z);
    glColor3f(0.8f, 0.8f, 1.0f);
    draw_text(pos_text, 20, 555, 1.0f);
    
    char vel_text[64];
    snprintf(vel_text, sizeof(vel_text), "Velocity: %.2f, %.2f", player_vel_x, player_vel_z);
    draw_text(vel_text, 20, 540, 1.0f);
    
    char style_text[64];
    snprintf(style_text, sizeof(style_text), "Style: %s", fighting_styles[player_style_global]);
    switch (player_style_global) {
        case 0: glColor3f(1.0f, 0.3f, 0.3f); break;
        case 1: glColor3f(0.3f, 1.0f, 0.3f); break;
        case 2: glColor3f(0.3f, 0.3f, 1.0f); break;
        case 3: glColor3f(1.0f, 1.0f, 0.3f); break;
    }
    draw_text(style_text, 20, 525, 1.0f);
    
    glColor3f(0.7f, 0.7f, 0.7f);
    draw_text("WASD: Fly Camera, QE: Up/Down", 20, 515, 0.8f);
    draw_text("Right Mouse: Look Around", 20, 505, 0.8f);
    draw_text("Shift: Fast, Ctrl: Slow, F12: Refresh", 20, 495, 0.8f);
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    if (hud_graphics) {
        draw_svg_graphics(hud_graphics, 0, 0, 1.0f);
    }
}

void update_player_movement(void) {
    if (!player_can_move || in_menu) return;
    
    float move_x = 0.0f, move_z = 0.0f;
    
    if (keys['W']) move_z -= player_speed;
    if (keys['S']) move_z += player_speed;
    if (keys['A']) move_x -= player_speed;
    if (keys['D']) move_x += player_speed;
    
    player_vel_x = player_vel_x * player_friction + move_x;
    player_vel_z = player_vel_z * player_friction + move_z;
    
    player_x += player_vel_x;
    player_z += player_vel_z;
    
    if (player_x < -15.0f) { player_x = -15.0f; player_vel_x = 0.0f; }
    if (player_x > 15.0f) { player_x = 15.0f; player_vel_x = 0.0f; }
    if (player_z < -15.0f) { player_z = -15.0f; player_vel_z = 0.0f; }
    if (player_z > 15.0f) { player_z = 15.0f; player_vel_z = 0.0f; }
    
    // Legacy camera controls (still available)
    if (keys[VK_LEFT]) cam_angle -= 0.03f;
    if (keys[VK_RIGHT]) cam_angle += 0.03f;
    if (keys[VK_UP]) cam_height += 0.1f;
    if (keys[VK_DOWN]) cam_height -= 0.1f;

    if (cam_height < 2.0f) cam_height = 2.0f;
    if (cam_height > 25.0f) cam_height = 25.0f;
}

// UE5-style camera system
void update_ue5_camera(void) {
    // WASD movement (always active in menu mode)
    float forward_x = -sin(cam_yaw * M_PI / 180.0f);
    float forward_z = -cos(cam_yaw * M_PI / 180.0f);
    float right_x = cos(cam_yaw * M_PI / 180.0f);
    float right_z = -sin(cam_yaw * M_PI / 180.0f);

    float move_speed = cam_speed;
    if (keys[VK_SHIFT]) move_speed *= 3.0f;  // Speed boost with Shift
    if (keys[VK_CONTROL]) move_speed *= 0.3f;  // Slow mode with Ctrl

    if (keys['W']) {
        cam_pos_x += forward_x * move_speed;
        cam_pos_z += forward_z * move_speed;
    }
    if (keys['S']) {
        cam_pos_x -= forward_x * move_speed;
        cam_pos_z -= forward_z * move_speed;
    }
    if (keys['A']) {
        cam_pos_x -= right_x * move_speed;
        cam_pos_z -= right_z * move_speed;
    }
    if (keys['D']) {
        cam_pos_x += right_x * move_speed;
        cam_pos_z += right_z * move_speed;
    }
    if (keys['Q']) cam_pos_y -= move_speed;  // Down
    if (keys['E']) cam_pos_y += move_speed;  // Up

    // Constrain camera position
    if (cam_pos_y < 1.0f) cam_pos_y = 1.0f;
    if (cam_pos_y > 50.0f) cam_pos_y = 50.0f;

    // Mouse wheel for speed adjustment
    if (keys[VK_ADD] || keys[VK_OEM_PLUS]) cam_speed += 0.1f;
    if (keys[VK_SUBTRACT] || keys[VK_OEM_MINUS]) cam_speed -= 0.1f;
    if (cam_speed < 0.1f) cam_speed = 0.1f;
    if (cam_speed > 5.0f) cam_speed = 5.0f;
}

void handle_mouse_input(int x, int y) {
    if (mouse_last_x == -1) {
        mouse_last_x = x;
        mouse_last_y = y;
        last_mouse_x = x;
        last_mouse_y = y;
        return;
    }

    int dx = x - mouse_last_x;
    int dy = y - mouse_last_y;

    // UE5-style mouse look (right mouse button)
    if (mouse_right_down && (in_menu || free_camera_mode)) {
        cam_yaw += dx * cam_sensitivity;
        cam_pitch -= dy * cam_sensitivity;

        // Constrain pitch to prevent flipping
        if (cam_pitch > 89.0f) cam_pitch = 89.0f;
        if (cam_pitch < -89.0f) cam_pitch = -89.0f;

        // Wrap yaw around 360 degrees
        while (cam_yaw > 360.0f) cam_yaw -= 360.0f;
        while (cam_yaw < 0.0f) cam_yaw += 360.0f;
    }

    // Legacy camera controls (for compatibility)
    if (mouse_right_down && !in_menu) {
        cam_angle += dx * 0.01f;
        cam_height -= dy * 0.1f;
        if (cam_height < 2.0f) cam_height = 2.0f;
        if (cam_height > 25.0f) cam_height = 25.0f;
    }

    if (mouse_left_down && !in_menu) {
        cam_pan_x += dx * 0.05f;
        cam_pan_z += dy * 0.05f;
    }

    mouse_last_x = x;
    mouse_last_y = y;
    last_mouse_x = x;
    last_mouse_y = y;
}

void init_svg_graphics(void) {
    menu_graphics = create_menu_graphics();
    hud_graphics = create_hud_graphics();
    printf("[DEBUG] SVG graphics initialized\n");
}

void cleanup_svg_graphics(void) {
    if (menu_graphics) {
        nsvgDelete(menu_graphics);
        menu_graphics = NULL;
    }
    if (hud_graphics) {
        nsvgDelete(hud_graphics);
        hud_graphics = NULL;
    }
    printf("[DEBUG] SVG graphics cleaned up\n");
}

NSVGimage* create_menu_graphics(void) {
    NSVGimage* image = nsvgCreateImage(800, 600);
    if (!image) return NULL;

    NSVGshape* border = nsvgCreateRect(90, 70, 620, 460, 0x40404080);
    nsvgAddShape(image, border);

    NSVGshape* corner1 = nsvgCreateCircle(100, 80, 10, 0x6060A0FF);
    NSVGshape* corner2 = nsvgCreateCircle(700, 80, 10, 0x6060A0FF);
    NSVGshape* corner3 = nsvgCreateCircle(100, 520, 10, 0x6060A0FF);
    NSVGshape* corner4 = nsvgCreateCircle(700, 520, 10, 0x6060A0FF);

    nsvgAddShape(image, corner1);
    nsvgAddShape(image, corner2);
    nsvgAddShape(image, corner3);
    nsvgAddShape(image, corner4);

    return image;
}

NSVGimage* create_hud_graphics(void) {
    NSVGimage* image = nsvgCreateImage(800, 600);
    if (!image) return NULL;

    NSVGshape* hud_frame = nsvgCreateRect(5, 475, 310, 120, 0x20202040);
    nsvgAddShape(image, hud_frame);

    NSVGshape* status_dot = nsvgCreateCircle(25, 580, 5, 0x00FF00FF);
    nsvgAddShape(image, status_dot);

    return image;
}

void draw_svg_graphics(NSVGimage* image, float x, float y, float scale) {
    if (!image) return;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    NSVGshape* shape = image->shapes;
    while (shape) {
        if (shape->fill != 0) {
            float r = ((shape->fill >> 24) & 0xFF) / 255.0f;
            float g = ((shape->fill >> 16) & 0xFF) / 255.0f;
            float b = ((shape->fill >> 8) & 0xFF) / 255.0f;
            float a = (shape->fill & 0xFF) / 255.0f;

            glColor4f(r, g, b, a);

            if (shape->npts >= 4) {
                glBegin(GL_QUADS);
                for (int i = 0; i < 4 && i < shape->npts; i++) {
                    glVertex2f(x + shape->pts[i*2] * scale, y + shape->pts[i*2+1] * scale);
                }
                glEnd();
            }
        }
        shape = shape->next;
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

NSVGimage* nsvgCreateImage(float width, float height) {
    NSVGimage* image = (NSVGimage*)malloc(sizeof(NSVGimage));
    if (!image) return NULL;
    image->width = width;
    image->height = height;
    image->shapes = NULL;
    return image;
}

NSVGshape* nsvgCreateRect(float x, float y, float w, float h, unsigned int fill) {
    NSVGshape* shape = (NSVGshape*)malloc(sizeof(NSVGshape));
    if (!shape) return NULL;

    shape->pts = (float*)malloc(8 * sizeof(float));
    if (!shape->pts) {
        free(shape);
        return NULL;
    }

    shape->pts[0] = x; shape->pts[1] = y;
    shape->pts[2] = x + w; shape->pts[3] = y;
    shape->pts[4] = x + w; shape->pts[5] = y + h;
    shape->pts[6] = x; shape->pts[7] = y + h;

    shape->npts = 4;
    shape->fill = fill;
    shape->stroke = 0;
    shape->strokeWidth = 0;
    shape->next = NULL;

    return shape;
}

NSVGshape* nsvgCreateCircle(float cx, float cy, float r, unsigned int fill) {
    NSVGshape* shape = (NSVGshape*)malloc(sizeof(NSVGshape));
    if (!shape) return NULL;

    int segments = 8;
    shape->pts = (float*)malloc(segments * 2 * sizeof(float));
    if (!shape->pts) {
        free(shape);
        return NULL;
    }

    for (int i = 0; i < segments; i++) {
        float angle = i * 2.0f * M_PI / segments;
        shape->pts[i*2] = cx + r * cos(angle);
        shape->pts[i*2+1] = cy + r * sin(angle);
    }

    shape->npts = segments;
    shape->fill = fill;
    shape->stroke = 0;
    shape->strokeWidth = 0;
    shape->next = NULL;

    return shape;
}

NSVGshape* nsvgCreateLine(float x1, float y1, float x2, float y2, unsigned int stroke, float strokeWidth) {
    NSVGshape* shape = (NSVGshape*)malloc(sizeof(NSVGshape));
    if (!shape) return NULL;

    shape->pts = (float*)malloc(4 * sizeof(float));
    if (!shape->pts) {
        free(shape);
        return NULL;
    }

    shape->pts[0] = x1; shape->pts[1] = y1;
    shape->pts[2] = x2; shape->pts[3] = y2;

    shape->npts = 2;
    shape->fill = 0;
    shape->stroke = stroke;
    shape->strokeWidth = strokeWidth;
    shape->next = NULL;

    return shape;
}

void nsvgAddShape(NSVGimage* image, NSVGshape* shape) {
    if (!image || !shape) return;

    shape->next = image->shapes;
    image->shapes = shape;
}

void nsvgDelete(NSVGimage* image) {
    if (!image) return;

    NSVGshape* shape = image->shapes;
    while (shape) {
        NSVGshape* next = shape->next;
        if (shape->pts) free(shape->pts);
        free(shape);
        shape = next;
    }

    free(image);
}

NSVGimage* nsvgParseFromFile(const char* filename, const char* units, float dpi) {
    (void)filename; (void)units; (void)dpi;
    return NULL;
}

NSVGimage* nsvgParse(char* input, const char* units, float dpi) {
    (void)input; (void)units; (void)dpi;
    return NULL;
}

BOOL init_opengl(HWND hWnd) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0,
        PFD_MAIN_PLANE, 0, 0, 0, 0
    };

    g_hDC = GetDC(hWnd);
    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    if (!pixelFormat) return FALSE;

    if (!SetPixelFormat(g_hDC, pixelFormat, &pfd)) return FALSE;

    g_hRC = wglCreateContext(g_hDC);
    if (!g_hRC) return FALSE;

    if (!wglMakeCurrent(g_hDC, g_hRC)) return FALSE;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // Transparent black - no gray blob

    init_svg_graphics();

    return TRUE;
}

void cleanup_opengl(void) {
    cleanup_svg_graphics();

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
            
            if (!last_keys[wParam]) {
                switch (wParam) {
                    case VK_RETURN:
                        if (in_menu) {
                            player_style_global = selected_style;
                            in_menu = 0;
                            player_can_move = 1;
                            
                            // Spawn ragdoll for the player
                            if (ragdoll_engine_initialized && !RAGDOLL_IS_VALID(player_ragdoll)) {
                                player_ragdoll = coreria_spawn_ragdoll(0, selected_style, player_x, 2.0f, player_z);
                                if (RAGDOLL_IS_VALID(player_ragdoll)) {
                                    printf("[MAIN] Player ragdoll spawned with style %s (Handle: %u)\n", 
                                           fighting_styles[selected_style], player_ragdoll.id);
                                } else {
                                    printf("[MAIN] Failed to spawn player ragdoll\n");
                                }
                            }
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
                        break;
                    case '1': // Punch/Basic Attack
                        if (!in_menu && RAGDOLL_IS_VALID(player_ragdoll)) {
                            // Apply force to arms for punching motion
                            coreria_apply_limb_force(player_ragdoll, 2, 100.0f, 0.0f, 0.0f, 0.0f, 50.0f, 0.0f); // Left arm
                            coreria_apply_limb_force(player_ragdoll, 3, 100.0f, 0.0f, 0.0f, 0.0f, -50.0f, 0.0f); // Right arm
                            add_log("[Combat] Executed punch attack!");
                        }
                        break;
                    case '2': // Kick Attack  
                        if (!in_menu && RAGDOLL_IS_VALID(player_ragdoll)) {
                            // Apply force to legs for kicking motion
                            coreria_apply_limb_force(player_ragdoll, 4, 150.0f, 0.0f, 50.0f, 0.0f, 0.0f, 75.0f); // Left leg
                            coreria_apply_limb_force(player_ragdoll, 5, 150.0f, 0.0f, -50.0f, 0.0f, 0.0f, -75.0f); // Right leg
                            add_log("[Combat] Executed kick attack!");
                        }
                        break;
                    case '3': // Special Move (Style-specific)
                        if (!in_menu && RAGDOLL_IS_VALID(player_ragdoll)) {
                            switch (player_style_global) {
                                case 0: // Brawler - Berserker Rage
                                    for (int limb = 0; limb < 6; limb++) {
                                        coreria_apply_limb_force(player_ragdoll, limb, 200.0f, 0.0f, 0.0f, 100.0f, 0.0f, 0.0f);
                                    }
                                    add_log("[Combat] BERSERKER RAGE activated!");
                                    break;
                                case 1: // Striker - Tornado Kick
                                    coreria_apply_limb_force(player_ragdoll, 2, 0.0f, 0.0f, 0.0f, 0.0f, 500.0f, 0.0f); // Spinning arms
                                    coreria_apply_limb_force(player_ragdoll, 3, 0.0f, 0.0f, 0.0f, 0.0f, -500.0f, 0.0f);
                                    add_log("[Combat] TORNADO KICK activated!");
                                    break;
                                case 2: // Phantom - Phase Step
                                    coreria_apply_limb_force(player_ragdoll, 4, 50.0f, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f); // Light movement
                                    coreria_apply_limb_force(player_ragdoll, 5, 50.0f, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                                    add_log("[Combat] PHASE STEP activated!");
                                    break;
                                case 3: // Titan - Ground Slam
                                    coreria_apply_limb_force(player_ragdoll, 2, 0.0f, -800.0f, 0.0f, 0.0f, 0.0f, 200.0f); // Arms down
                                    coreria_apply_limb_force(player_ragdoll, 3, 0.0f, -800.0f, 0.0f, 0.0f, 0.0f, -200.0f);
                                    add_log("[Combat] GROUND SLAM activated!");
                                    break;
                            }
                        }
                        break;
                    case 'R': // Reset ragdoll position
                        if (!in_menu && RAGDOLL_IS_VALID(player_ragdoll)) {
                            // Destroy and respawn ragdoll at current player position
                            coreria_destroy_ragdoll(player_ragdoll);
                            player_ragdoll = coreria_spawn_ragdoll(0, player_style_global, player_x, 2.0f, player_z);
                            add_log("[Combat] Ragdoll reset to player position!");
                        }
                        break;
                    case 'T': // Toggle ragdoll physics
                        if (!in_menu) {
                            ragdoll_physics_enabled = !ragdoll_physics_enabled;
                            if (ragdoll_physics_enabled) {
                                add_log("[System] Ragdoll physics enabled!");
                            } else {
                                add_log("[System] Ragdoll physics disabled!");
                            }
                        }
                        break;
                    case VK_UP:
                        if (in_menu) {
                            selected_style = (selected_style + 3) % 4;
                        }
                        break;
                    case VK_DOWN:
                        if (in_menu) {
                            selected_style = (selected_style + 1) % 4;
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
                cam_radius -= delta / 120.0f;
                if (cam_radius < 5.0f) cam_radius = 5.0f;
                if (cam_radius > 50.0f) cam_radius = 50.0f;
            }
            break;
            
        case WM_SIZE:
            window_width = LOWORD(lParam);
            window_height = HIWORD(lParam);
            glViewport(0, 0, window_width, window_height);
            break;
            
default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    WNDCLASSEX wcex = {
        sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_OWNDC, WndProc, 0, 0,
        hInstance, NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, "ChaosForgeArena", NULL
    };
    
    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_OK);
        return -1;
    }

    g_hWnd = CreateWindow(
        "ChaosForgeArena", "ChaosForge Arena", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hWnd) {
        MessageBox(NULL, "Failed to create window", "Error", MB_OK);
        return -1;
    }

    if (!init_opengl(g_hWnd)) {
        MessageBox(NULL, "Failed to initialize OpenGL", "Error", MB_OK);
        cleanup_opengl();
        return -1;
    }

    // Initialize ragdoll physics engine
    if (coreria_init_engine() == 0) {
        ragdoll_engine_initialized = 1;
        ragdoll_physics_enabled = 1;
        printf("[MAIN] ChaosForge multiplayer ragdoll engine initialized successfully\n");
    } else {
        printf("[MAIN] Warning: Failed to initialize ragdoll engine, continuing without physics\n");
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    DWORD lastTime = GetTickCount();
    
    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = 0;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        update_player_movement();
        update_ue5_camera();  // Update UE5-style camera

        // F12 for real-time reload/refresh
        if (keys[VK_F12]) {
            // Force a full refresh - could reload shaders, textures, etc.
            InvalidateRect(g_hWnd, NULL, TRUE);
            keys[VK_F12] = 0;  // Prevent repeated triggers
        }

        DWORD currentTime = GetTickCount();
        if (currentTime - lastTime >= 16) {
            update_game_logic();
            render_frame();
            SwapBuffers(g_hDC);
            lastTime = currentTime;
        }
        
        Sleep(1);
    }

    // Cleanup ragdoll system
    if (ragdoll_engine_initialized) {
        if (RAGDOLL_IS_VALID(player_ragdoll)) {
            coreria_destroy_ragdoll(player_ragdoll);
            player_ragdoll = RAGDOLL_INIT();
        }
        coreria_shutdown_engine();
        printf("[MAIN] Ragdoll engine shut down successfully\n");
    }

    cleanup_opengl();
    return 0;
}
