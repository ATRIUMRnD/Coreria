// ChaosForge Arena - Windows OpenGL Version
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
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
// GLFW window pointer (for cross-platform input)
void* window; // Use GLFWwindow* in real code
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
const char* fighting_styles[] = {"Brawler", "Striker", "Phantom", "Titan"};
int selected_style = 0;
int in_menu = 1;
float player_x = 0.0f, player_z = -5.0f;
int player_style_global = 0;
int running = 1;

// Game state variables for GLUT
GameState game_state;
int game_tick = 0;
int training_phase = 1;
int master_style = 0;
int passed_exam = 0;
int training_ticks = 0;

// Function prototypes
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
            case 256: running = 0; break; // GLFW_KEY_ESCAPE
            case 257: // GLFW_KEY_ENTER
                if (in_menu) { player_style_global = selected_style; in_menu = 0; }
                break;
            case 67: // 'C'
                if (in_menu) { copy_to_clipboard(fighting_styles[selected_style]); add_log("[Menu] Style copied to clipboard."); }
                break;
            case 32: // Space
                if (!in_menu && dynamic_object_count < MAX_DYNAMIC_OBJECTS) {
                    dynamic_objects[dynamic_object_count].x = player_x;
                    dynamic_objects[dynamic_object_count].z = player_z;
                    dynamic_objects[dynamic_object_count].style = player_style_global;
                    dynamic_object_count++;
                    add_log("[Game] New object spawned at player position.");
                }
                break;
            case 70: show_console = !show_console; break; // 'F'
            case 265: if (in_menu) selected_style = (selected_style + 3) % 4; else cam_height += 0.5f; break; // GLFW_KEY_UP
            case 264: if (in_menu) selected_style = (selected_style + 1) % 4; else cam_height -= 0.5f; break; // GLFW_KEY_DOWN
            case 263: if (!in_menu) cam_angle -= 0.1f; break; // GLFW_KEY_LEFT
            case 262: if (!in_menu) cam_angle += 0.1f; break; // GLFW_KEY_RIGHT
        }
    }
}

void mouse_scroll_callback(float yoffset) {
    cam_radius -= yoffset;
    if (cam_radius < 5.0f) cam_radius = 5.0f;
    if (cam_radius > 50.0f) cam_radius = 50.0f;
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    float cam_x = target_x + cam_pan_x + cam_radius * cos(cam_angle);
    float cam_y = cam_height;
    float cam_z = target_z + cam_pan_z + cam_radius * sin(cam_angle);
    gluLookAt(cam_x, cam_y, cam_z, target_x + cam_pan_x, 0.0f, target_z + cam_pan_z, 0.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
}

void draw_menu(void) {
    // Simple menu - draw colored rectangles to represent menu items
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);
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
#ifdef _WIN32
    if (OpenClipboard(NULL)) {
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
#endif
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

// Windows message handling
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            running = 0;
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_ESCAPE:
                    running = 0;
                    PostQuitMessage(0);
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
                case 'F':
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

// Rendering function
void render_frame(void) {
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_background();

    if (in_menu) {
        draw_menu();
    } else if (training_phase && !passed_exam) {
        render_scene(NULL, 1, master_style, player_style_global);
    } else {
        render_scene(&game_state, 0, 0, 0);
    }

    if (show_console) {
        render_logs();
    }

    SwapBuffers(hdc);
}

// Game update function
void update_game_logic(void) {
    if (!running) return;

    if (in_menu) {
        // Just redraw menu
    } else if (training_phase && !passed_exam) {
        training_ticks++;
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

// OpenGL initialization
int init_opengl(void) {
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

    int format = ChoosePixelFormat(hdc, &pfd);
    if (!format) return 0;

    if (!SetPixelFormat(hdc, format, &pfd)) return 0;

    hglrc = wglCreateContext(hdc);
    if (!hglrc) return 0;

    if (!wglMakeCurrent(hdc, hglrc)) return 0;

    // Set up OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return 1;
}

// Main function
int main(int argc, char** argv) {
    printf("[DEBUG] Starting ChaosForge Arena...\n");

    // Register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "ChaosForgeWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc)) {
        printf("[ERROR] Failed to register window class\n");
        return -1;
    }

    // Create window
    hwnd = CreateWindow("ChaosForgeWindow", "ChaosForge Arena",
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                        100, 100, 800, 600,
                        NULL, NULL, GetModuleHandle(NULL), NULL);

    if (!hwnd) {
        printf("[ERROR] Failed to create window\n");
        return -1;
    }

    // Get device context
    hdc = GetDC(hwnd);
    if (!hdc) {
        printf("[ERROR] Failed to get device context\n");
        return -1;
    }

    // Initialize OpenGL
    if (!init_opengl()) {
        printf("[ERROR] Failed to initialize OpenGL\n");
        return -1;
    }

    printf("[DEBUG] OpenGL initialized successfully.\n");

    add_log("[ChaosForge] Window opened: 800x600");
    add_log("[ChaosForge] Initializing Coreria engine...");
    add_log("[ChaosForge] Loading arena: chaosforge_coliseum.obj");
    add_log("[ChaosForge] Initializing ODE physics...");

    // Set up training phase
    player_x = 0.0f;
    player_z = 0.0f;
    master_style = 0;

    // Main message loop
    MSG msg;
    DWORD last_time = GetTickCount();

    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        DWORD current_time = GetTickCount();
        if (current_time - last_time >= 16) { // ~60 FPS
            update_game_logic();
            render_frame();
            last_time = current_time;
        }

        Sleep(1); // Prevent 100% CPU usage
    }

    // Cleanup
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    return 0;
}
