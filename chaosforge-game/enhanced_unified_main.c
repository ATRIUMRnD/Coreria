// Enhanced Unified Main - Integrates all combat and animation systems
// File: chaosforge-game/enhanced_unified_main.c

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "enhanced_combat_system.h"
#include "gangbeast_avatar.h"
#include "multiplayer_integration.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define WINDOW_TITLE "ChaosForge Arena - Enhanced Combat System"

// Global variables
HWND g_hWnd;
HDC g_hDC;
HGLRC g_hRC;
EnhancedGameState g_game_state;
BOOL g_keys[256] = {FALSE};
int g_mouse_x = 0, g_mouse_y = 0;
BOOL g_mouse_buttons[3] = {FALSE};
clock_t g_last_frame_time;
float g_delta_time = 0.016f; // Target 60 FPS

// Function prototypes
LRESULT CALLBACK EnhancedWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL init_enhanced_opengl(HWND hWnd);
void cleanup_enhanced_opengl(void);
void render_enhanced_scene(void);
void update_enhanced_input(void);
void render_enhanced_hud(void);
void render_enhanced_player(EnhancedPlayer* player);
void setup_enhanced_lighting(void);

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    printf("=== CHAOSFORGE ENHANCED COMBAT SYSTEM ===\n");
    printf("Integrating all combat, animation, and physics systems\n");
    printf("Real-time performance monitoring and logging enabled\n\n");
    
    // Register window class
    WNDCLASSEX wcex = {
        sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_OWNDC, EnhancedWndProc, 0, 0,
        hInstance, NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, "EnhancedChaosForgeArena", NULL
    };
    
    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_OK);
        return -1;
    }

    // Create window
    g_hWnd = CreateWindow(
        "EnhancedChaosForgeArena", WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hWnd) {
        MessageBox(NULL, "Failed to create window", "Error", MB_OK);
        return -1;
    }

    // Initialize OpenGL
    if (!init_enhanced_opengl(g_hWnd)) {
        MessageBox(NULL, "Failed to initialize OpenGL", "Error", MB_OK);
        return -1;
    }

    // Initialize enhanced combat system
    enhanced_combat_init(&g_game_state);
    
    // Show window and start game loop
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    g_last_frame_time = clock();
    
    printf("=== ENHANCED CONTROLS ===\n");
    printf("COMBAT: Left Click - Light Punch, Right Click - Heavy Kick\n");
    printf("MOVEMENT: WASD - Move, SHIFT - Sprint, SPACE - Block\n");
    printf("SYSTEM: ESC - Exit, F1 - Add AI Player, F12 - Export Metrics\n");
    printf("MONITOR: Real-time performance logging to combat_log.txt\n\n");
    
    // Main game loop
    MSG msg;
    BOOL running = TRUE;
    
    while (running) {
        // Handle Windows messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = FALSE;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Calculate frame time
        clock_t current_time = clock();
        g_delta_time = ((float)(current_time - g_last_frame_time)) / CLOCKS_PER_SEC;
        g_last_frame_time = current_time;
        
        // Limit to 60 FPS
        if (g_delta_time < 0.016f) {
            Sleep(1);
            continue;
        }
        
        // Update input
        update_enhanced_input();
        
        // Update enhanced combat system
        enhanced_combat_update(&g_game_state, g_delta_time);
        
        // Render scene
        render_enhanced_scene();
        
        // Swap buffers
        SwapBuffers(g_hDC);
    }
    
    // Cleanup
    enhanced_combat_cleanup(&g_game_state);
    cleanup_enhanced_opengl();
    return (int)msg.wParam;
}

// Window procedure
LRESULT CALLBACK EnhancedWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SIZE:
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_KEYDOWN:
            g_keys[wParam] = TRUE;
            
            // Handle special keys
            switch (wParam) {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
                case VK_F1:
                    // Add AI player
                    if (g_game_state.num_players < MAX_PLAYERS) {
                        g_game_state.num_players++;
                        enhanced_combat_log_event(&g_game_state, "AI Player added - Total players: %d", g_game_state.num_players);
                    }
                    break;
                case VK_F12:
                    // Export current metrics
                    enhanced_combat_export_metrics(&g_game_state);
                    enhanced_combat_log_event(&g_game_state, "Metrics exported manually");
                    break;
            }
            return 0;
            
        case WM_KEYUP:
            g_keys[wParam] = FALSE;
            return 0;
            
        case WM_LBUTTONDOWN:
            g_mouse_buttons[0] = TRUE;
            // Light punch
            enhanced_combat_handle_input(&g_game_state, 0, ENHANCED_ACTION_PUNCH, 
                                       (float)LOWORD(lParam), (float)HIWORD(lParam));
            return 0;
            
        case WM_LBUTTONUP:
            g_mouse_buttons[0] = FALSE;
            return 0;
            
        case WM_RBUTTONDOWN:
            g_mouse_buttons[1] = TRUE;
            // Heavy kick
            enhanced_combat_handle_input(&g_game_state, 0, ENHANCED_ACTION_KICK, 
                                       (float)LOWORD(lParam), (float)HIWORD(lParam));
            return 0;
            
        case WM_RBUTTONUP:
            g_mouse_buttons[1] = FALSE;
            return 0;
            
        case WM_MOUSEMOVE:
            g_mouse_x = LOWORD(lParam);
            g_mouse_y = HIWORD(lParam);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Initialize OpenGL
BOOL init_enhanced_opengl(HWND hWnd) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, 8, 0,
        PFD_MAIN_PLANE,
        0, 0, 0, 0
    };

    g_hDC = GetDC(hWnd);
    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    if (!pixelFormat || !SetPixelFormat(g_hDC, pixelFormat, &pfd)) return FALSE;
    
    g_hRC = wglCreateContext(g_hDC);
    if (!g_hRC || !wglMakeCurrent(g_hDC, g_hRC)) return FALSE;
    
    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    
    setup_enhanced_lighting();
    
    printf("[ENHANCED] OpenGL initialized successfully\n");
    return TRUE;
}

void cleanup_enhanced_opengl(void) {
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

void setup_enhanced_lighting(void) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    GLfloat light_position[] = {10.0f, 10.0f, 10.0f, 1.0f};
    GLfloat light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat light_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void update_enhanced_input(void) {
    // Movement input
    int movement_input = 0;
    
    if (g_keys['W']) {
        enhanced_combat_handle_input(&g_game_state, 0, ENHANCED_ACTION_MOVE_FORWARD, 0, 0);
        movement_input = 1;
    }
    if (g_keys['S']) {
        enhanced_combat_handle_input(&g_game_state, 0, ENHANCED_ACTION_MOVE_BACKWARD, 0, 0);
        movement_input = 1;
    }
    if (g_keys['A']) {
        enhanced_combat_handle_input(&g_game_state, 0, ENHANCED_ACTION_MOVE_LEFT, 0, 0);
        movement_input = 1;
    }
    if (g_keys['D']) {
        enhanced_combat_handle_input(&g_game_state, 0, ENHANCED_ACTION_MOVE_RIGHT, 0, 0);
        movement_input = 1;
    }
    
    // Block input
    if (g_keys[VK_SPACE]) {
        enhanced_combat_handle_input(&g_game_state, 0, ENHANCED_ACTION_BLOCK, 0, 0);
    }
    
    // If no movement input, set player to idle animation
    if (!movement_input) {
        EnhancedPlayer* player = &g_game_state.players[0];
        if (player->combat.avatar && player->combat.current_anim == ANIM_WALKING) {
            gangbeast_avatar_set_animation(player->combat.avatar, ANIM_IDLE);
            player->combat.current_anim = ANIM_IDLE;
        }
    }
}

void render_enhanced_scene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up camera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1, 100.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Position camera to view arena
    EnhancedPlayer* local_player = &g_game_state.players[0];
    gluLookAt(
        local_player->x + 10.0f, local_player->y + 8.0f, local_player->z + 10.0f,  // Camera position
        local_player->x, local_player->y, local_player->z,                         // Look at player
        0.0f, 1.0f, 0.0f                                                          // Up vector
    );
    
    // Render arena floor
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.6f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-20.0f, 0.0f, -20.0f);
    glVertex3f(20.0f, 0.0f, -20.0f);
    glVertex3f(20.0f, 0.0f, 20.0f);
    glVertex3f(-20.0f, 0.0f, 20.0f);
    glEnd();
    
    // Arena boundaries
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-15.0f, 0.0f, -15.0f);
    glVertex3f(15.0f, 0.0f, -15.0f);
    glVertex3f(15.0f, 0.0f, 15.0f);
    glVertex3f(-15.0f, 0.0f, 15.0f);
    glEnd();
    
    glEnable(GL_LIGHTING);
    
    // Render all players
    for (int i = 0; i < g_game_state.num_players; i++) {
        render_enhanced_player(&g_game_state.players[i]);
    }
    
    // Render HUD
    render_enhanced_hud();
}

void render_enhanced_player(EnhancedPlayer* player) {
    if (!player) return;
    
    glPushMatrix();
    
    // Position player
    glTranslatef(player->x, player->y, player->z);
    glRotatef(player->facing_angle * 180.0f / 3.14159f, 0, 1, 0);
    
    // Render avatar if available
    if (player->combat.avatar) {
        gangbeast_avatar_render(player->combat.avatar, 0, 0, 0, 0, DETAIL_HIGH);
    } else {
        // Fallback simple rendering
        glColor3f(1.0f, 0.5f, 0.2f);
        glTranslatef(0, 1.0f, 0);
        
        // Body
        glPushMatrix();
        glScalef(0.5f, 1.0f, 0.3f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Head
        glPushMatrix();
        glTranslatef(0, 0.8f, 0);
        glutSolidSphere(0.3f, 12, 8);
        glPopMatrix();
    }
    
    // Render combat effects
    if (player->combat.attack_animation > 0) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        
        float intensity = (float)player->combat.attack_animation / 45.0f;
        glColor4f(1.0f, 1.0f, 0.0f, intensity * 0.5f);
        
        glPushMatrix();
        glTranslatef(0, 1.0f, -1.5f);
        glScalef(2.0f, 0.1f, 3.0f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }
    
    if (player->combat.block_animation > 0) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        
        glColor4f(0.0f, 0.0f, 1.0f, 0.3f);
        
        glPushMatrix();
        glTranslatef(0, 1.0f, 0);
        glScalef(1.5f, 2.0f, 1.5f);
        glutWireCube(1.0f);
        glPopMatrix();
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }
    
    glPopMatrix();
}

void render_enhanced_hud(void) {
    // Switch to 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
    EnhancedPlayer* player = &g_game_state.players[0];
    
    // Health bar
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(20, 20);
    glVertex2f(20 + (200 * player->health / player->max_health), 20);
    glVertex2f(20 + (200 * player->health / player->max_health), 40);
    glVertex2f(20, 40);
    glEnd();
    
    // Health bar border
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(20, 20);
    glVertex2f(220, 20);
    glVertex2f(220, 40);
    glVertex2f(20, 40);
    glEnd();
    
    // Stamina bar
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(20, 50);
    glVertex2f(20 + (200 * player->stamina / player->max_stamina), 50);
    glVertex2f(20 + (200 * player->stamina / player->max_stamina), 70);
    glVertex2f(20, 70);
    glEnd();
    
    // Stamina bar border
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(20, 50);
    glVertex2f(220, 50);
    glVertex2f(220, 70);
    glVertex2f(20, 70);
    glEnd();
    
    // Performance info
    glColor3f(0.8f, 0.8f, 0.8f);
    
    char fps_text[64];
    sprintf(fps_text, "FPS: %.1f", g_game_state.performance.avg_fps);
    // Note: Text rendering would require bitmap fonts or texture-based text
    // For now, just render the frame data
    
    // Combat stats
    char combat_text[128];
    sprintf(combat_text, "Hits: %d | Blocks: %d | Combo: %d", 
            player->combat.successful_hits, 
            player->combat.blocked_attacks,
            player->combat.combo_meter);
    
    // System status indicators
    glColor3f(g_game_state.avatar_system_enabled ? 0.0f : 1.0f, 
             g_game_state.avatar_system_enabled ? 1.0f : 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(WINDOW_WIDTH - 120, 20);
    glVertex2f(WINDOW_WIDTH - 100, 20);
    glVertex2f(WINDOW_WIDTH - 100, 30);
    glVertex2f(WINDOW_WIDTH - 120, 30);
    glEnd();
    
    glColor3f(g_game_state.physics_system_enabled ? 0.0f : 1.0f, 
             g_game_state.physics_system_enabled ? 1.0f : 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(WINDOW_WIDTH - 90, 20);
    glVertex2f(WINDOW_WIDTH - 70, 20);
    glVertex2f(WINDOW_WIDTH - 70, 30);
    glVertex2f(WINDOW_WIDTH - 90, 30);
    glEnd();
    
    glColor3f(g_game_state.multiplayer_system_enabled ? 0.0f : 1.0f, 
             g_game_state.multiplayer_system_enabled ? 1.0f : 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(WINDOW_WIDTH - 60, 20);
    glVertex2f(WINDOW_WIDTH - 40, 20);
    glVertex2f(WINDOW_WIDTH - 40, 30);
    glVertex2f(WINDOW_WIDTH - 60, 30);
    glEnd();
    
    // Restore 3D rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}