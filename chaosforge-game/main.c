// ChaosForge Arena - Clean Windows Version
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>
#include "player_controller.h"
#include "combat_system.h"
#include "physics_manager.h"
#include "game_state.h"

// Constants
#define MAX_PARTICLES 64
#define MAX_DYNAMIC_OBJECTS 16
#define MAX_LOG_LINES 32
#define LOG_LINE_LENGTH 128

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
GLFWwindow* window; // Declare window globally for GLFW
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
int cam_follow_player = 0;
int mouse_left_down = 0;
int mouse_last_x = -1, mouse_last_y = -1;
int show_console = 0;

// Game state
// (Removed Windows-specific handles)
const char* fighting_styles[] = {"Brawler", "Striker", "Phantom", "Titan"};
int selected_style = 0;
int in_menu = 1;
float player_x = 0.0f, player_z = -5.0f;
int player_style_global = 0;
int running = 1;

// Function prototypes
void spawn_particles(float x, float z, int type);
void update_particles(void);
void draw_particles(void);
void add_log(const char* msg);
void render_logs(void);
void draw_background(void);
void draw_menu(void);
void draw_base_plate(void);
void draw_player(float x, float z, int style, int is_master, int health, int attack_anim, int respawn_anim, int lives);
void draw_dynamic_objects(void);
void render_scene(GameState* state, int training_phase, int master_style, int player_style);
void copy_to_clipboard(const char* text);
int init_opengl(void);
// (Removed Windows-specific WindowProc declaration)

// Particle system implementation
void spawn_particles(float x, float z, int type) {
    for (int i = 0; i < 8 && particle_count < MAX_PARTICLES; ++i) {
        float angle = (float)i * (2 * M_PI / 8);
void add_log(const char* msg);
void spawn_particles(float x, float z, int type);
        particles[particle_count].vx = 0.1f * cos(angle);
        particles[particle_count].vy = 0.08f;
        particles[particle_count].vz = 0.1f * sin(angle);
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
        particles[i].vy -= 0.005f; // gravity
        particles[i].life--;
        if (particles[i].life <= 0) {
            // Remove particle
            particles[i] = particles[particle_count-1];
            particle_count--;
            i--;
        }
    }
}

void draw_particles(void) {
    for (int i = 0; i < particle_count; ++i) {
        glPushMatrix();
        glTranslatef(particles[i].x, particles[i].y, particles[i].z);
        if (particles[i].type == 0)
            glColor3f(1,0,0); // attack: red
        else
            glColor3f(1,1,0); // respawn: yellow
        GLUquadric* quad = gluNewQuadric();
        gluSphere(quad, 0.12, 8, 8);
        gluDeleteQuadric(quad);
        glPopMatrix();
    }
}

// Dynamic objects rendering
void draw_dynamic_objects(void) {
    for (int i = 0; i < dynamic_object_count; ++i) {
        draw_player(dynamic_objects[i].x, dynamic_objects[i].z, dynamic_objects[i].style, 0, 100, 0, 0, 2);
    }
}

// Scene rendering
void render_scene(GameState* state, int training_phase, int master_style, int player_style) {
    // Draw particles
    draw_particles();
    // Camera setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 800.0/600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float target_x = 0.0f, target_z = 0.0f;
    if (cam_follow_player && state) {
        // Follow player 0
        float angle = 0 * (2 * M_PI / MAX_PLAYERS);
        target_x = 7.0f * cos(angle);
        target_z = 7.0f * sin(angle);
    }
    float cam_x = target_x + cam_radius * cos(cam_angle);
    float cam_y = cam_height;
    float cam_z = target_z + cam_radius * sin(cam_angle);
    gluLookAt(cam_x, cam_y, cam_z, target_x, 0.0f, target_z, 0.0f, 1.0f, 0.0f);
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

#ifdef _WIN32
// Windows message handling
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            running = 0;
            return 0;
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_F12:
                    show_console = !show_console;
                    break;
                case VK_UP:
                    if (in_menu) selected_style = (selected_style + 3) % 4;
                    else cam_height += 0.5f;
                    break;
                case VK_DOWN:
                    if (in_menu) selected_style = (selected_style + 1) % 4;
                    else cam_height -= 0.5f;
                    break;
                case VK_LEFT:
                    if (!in_menu) cam_angle -= 0.1f;
                    break;
                case VK_RIGHT:
                    if (!in_menu) cam_angle += 0.1f;
                    break;
                case VK_RETURN:
                    if (in_menu) {
                        player_style_global = selected_style;
                        in_menu = 0;
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
            }
            return 0;
        case WM_LBUTTONDOWN:
            mouse_left_down = 1;
            mouse_last_x = LOWORD(lParam);
            mouse_last_y = HIWORD(lParam);
            return 0;
        case WM_LBUTTONUP:
            mouse_left_down = 0;
            return 0;
        case WM_MOUSEMOVE:
            if (mouse_left_down) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                int dx = x - mouse_last_x;
                int dy = y - mouse_last_y;
                cam_angle += dx * 0.01f;
                cam_height += dy * 0.05f;
                if (cam_height < 2.0f) cam_height = 2.0f;
                if (cam_height > 30.0f) cam_height = 30.0f;
                mouse_last_x = x;
                mouse_last_y = y;
            }
            return 0;
        case WM_MOUSEWHEEL:
            {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                cam_radius -= delta / 120.0f;
                if (cam_radius < 5.0f) cam_radius = 5.0f;
                if (cam_radius > 50.0f) cam_radius = 50.0f;
            }
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// OpenGL initialization
int init_opengl(void) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "ChaosForgeWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        return 0;
    }

#endif

// Main function
int main() {
    printf("[DEBUG] Starting ChaosForge Arena...\n");

#ifdef _WIN32
    if (!init_opengl()) {
        printf("[ERROR] Failed to initialize OpenGL\n");
        return -1;
    }
#endif

    printf("[DEBUG] OpenGL initialized successfully.\n");

    add_log("[ChaosForge] Window opened: 800x600");
    add_log("[ChaosForge] Initializing Coreria engine...");
    add_log("[ChaosForge] Loading arena: chaosforge_coliseum.obj");
    add_log("[ChaosForge] Initializing ODE physics...");

    // Menu loop
    while (running && in_menu) {
#ifdef _WIN32
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw_background();
        draw_menu();
        if (show_console) render_logs();
#ifdef _WIN32
    glfwSwapBuffers(window);
        Sleep(16); // ~60 FPS
#endif
    }

    // Training phase
    player_x = 0.0f;
    player_z = 0.0f;
    int player_style = player_style_global;
    int master_style = player_style;
    char logbuf[LOG_LINE_LENGTH];
    snprintf(logbuf, LOG_LINE_LENGTH, "[Training] Welcome! You must defeat the master (%s) to earn your entrance exam ticket.", get_style_name(master_style));
    add_log(logbuf);
    int passed_exam = 0;
    int training_ticks = 0;
    while (running && !passed_exam) {
#ifdef _WIN32
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw_background();
        render_scene(NULL, 1, master_style, player_style);
        if (show_console) render_logs();
#ifdef _WIN32
    glfwSwapBuffers(window);
#endif
        training_ticks++;
        if (training_ticks == 120) {
            add_log("[Training] You land a decisive blow! Master defeated. Ticket granted.");
            passed_exam = 1;
        }
#ifdef _WIN32
        Sleep(16); // ~60 FPS
#endif
    }

    // Main match setup
    GameState state;
    init_game_state(&state);
    // Fill lobby with bots matching player style if no other players
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        state.players[i].style = player_style;
    }
    spawn_players(&state);

    add_log("[Time: 0.016s] Game loop started (60 FPS)");
    int tick = 1;
    while (running) {
#ifdef _WIN32
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
#endif

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw_background();
        render_scene(&state, 0, 0, 0);
        if (show_console) render_logs();
        update_game(&state, tick);

        // Decrement animation timers and spawn particles
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            if (state.attack_anim[i] > 0) {
                state.attack_anim[i]--;
                if (state.attack_anim[i] == 9) {
                    float angle = i * (2 * M_PI / MAX_PLAYERS);
                    float x = 7.0f * cos(angle);
                    float z = 7.0f * sin(angle);
                    spawn_particles(x, z, 0);
                }
            }
            if (state.respawn_anim[i] > 0) {
                state.respawn_anim[i]--;
                if (state.respawn_anim[i] == 19) {
                    float angle = i * (2 * M_PI / MAX_PLAYERS);
                    float x = 7.0f * cos(angle);
                    float z = 7.0f * sin(angle);
                    spawn_particles(x, z, 1);
                }
            }
        }
        update_particles();
#ifdef _WIN32
    glfwSwapBuffers(window);
#endif
        tick++;
#ifdef _WIN32
        Sleep(8); // ~120 FPS
#endif
    }

    // Graceful shutdown
    add_log("[ChaosForge] Shutting down gracefully...");
#ifdef _WIN32
#endif
    return 0;
}
