#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
// #include "multiplayer_integration.h"  // Disabled for now

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define M_PI 3.14159265358979323846

// Global variables
HWND g_hWnd = NULL;
HDC g_hDC = NULL;
HGLRC g_hRC = NULL;
int running = 1;
int keys[256] = {0};

// Free camera system (UE5-style)
typedef struct {
    float pos_x, pos_y, pos_z;     // Camera position
    float yaw, pitch;              // Camera rotation (degrees)
    float speed;                   // Movement speed
    float sensitivity;             // Mouse sensitivity
    int mouse_captured;            // Is mouse captured for look
} FreeCamera;

FreeCamera camera = {
    .pos_x = 0.0f, .pos_y = 5.0f, .pos_z = 15.0f,
    .yaw = 0.0f, .pitch = -15.0f,
    .speed = 0.5f, .sensitivity = 0.1f,
    .mouse_captured = 0
};

// Game state
int in_menu = 1;
int selected_style = 0;
const char* fighting_styles[] = {"Brawler", "Striker", "Phantom", "Titan"};

// Multiplayer state (disabled for now)
// int multiplayer_active = 0;
// int current_match_id = -1;
// int local_player_id = -1;

// Function prototypes
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL InitOpenGL(HWND hWnd);
void CleanupOpenGL(void);
void UpdateCamera(void);
void RenderScene(void);
void RenderMenu(void);
void DrawGameText(const char* text, float x, float y, float size);
void DrawBasePlate(void);

// Initialize OpenGL
BOOL InitOpenGL(HWND hWnd) {
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
    if (!g_hDC) return FALSE;

    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    if (!pixelFormat) return FALSE;

    if (!SetPixelFormat(g_hDC, pixelFormat, &pfd)) return FALSE;

    g_hRC = wglCreateContext(g_hDC);
    if (!g_hRC) return FALSE;

    if (!wglMakeCurrent(g_hDC, g_hRC)) return FALSE;

    // OpenGL settings - NO CLEAR COLOR (transparent background)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set viewport
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    return TRUE;
}

void CleanupOpenGL(void) {
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

// Update free camera (UE5-style)
void UpdateCamera(void) {
    // Calculate movement vectors
    float forward_x = -sin(camera.yaw * M_PI / 180.0f);
    float forward_z = -cos(camera.yaw * M_PI / 180.0f);
    float right_x = cos(camera.yaw * M_PI / 180.0f);
    float right_z = -sin(camera.yaw * M_PI / 180.0f);
    
    float move_speed = camera.speed;
    if (keys[VK_SHIFT]) move_speed *= 3.0f;  // Speed boost
    if (keys[VK_CONTROL]) move_speed *= 0.3f;  // Slow mode
    
    // WASD movement
    if (keys['W']) {
        camera.pos_x += forward_x * move_speed;
        camera.pos_z += forward_z * move_speed;
    }
    if (keys['S']) {
        camera.pos_x -= forward_x * move_speed;
        camera.pos_z -= forward_z * move_speed;
    }
    if (keys['A']) {
        camera.pos_x -= right_x * move_speed;
        camera.pos_z -= right_z * move_speed;
    }
    if (keys['D']) {
        camera.pos_x += right_x * move_speed;
        camera.pos_z += right_z * move_speed;
    }
    if (keys['Q']) camera.pos_y -= move_speed;  // Down
    if (keys['E']) camera.pos_y += move_speed;  // Up
    
    // Constrain camera position
    if (camera.pos_y < 0.5f) camera.pos_y = 0.5f;
    if (camera.pos_y > 100.0f) camera.pos_y = 100.0f;
    
    // Speed adjustment
    if (keys[VK_ADD] || keys[VK_OEM_PLUS]) {
        camera.speed += 0.1f;
        if (camera.speed > 5.0f) camera.speed = 5.0f;
    }
    if (keys[VK_SUBTRACT] || keys[VK_OEM_MINUS]) {
        camera.speed -= 0.1f;
        if (camera.speed < 0.1f) camera.speed = 0.1f;
    }
}

// Draw simple text (bitmap)
void DrawGameText(const char* text, float x, float y, float size) {
    glRasterPos2f(x, y);
    // Simple text rendering - you can enhance this
    for (int i = 0; text[i]; i++) {
        // Basic character rendering
    }
}

// Draw the base plate (3D ground)
void DrawBasePlate(void) {
    glColor3f(0.2f, 0.3f, 0.4f);
    glBegin(GL_QUADS);
    
    // Large ground plane
    float size = 50.0f;
    glVertex3f(-size, 0.0f, -size);
    glVertex3f(size, 0.0f, -size);
    glVertex3f(size, 0.0f, size);
    glVertex3f(-size, 0.0f, size);
    
    glEnd();
    
    // Grid lines
    glColor3f(0.4f, 0.5f, 0.6f);
    glBegin(GL_LINES);
    for (int i = -10; i <= 10; i++) {
        float pos = i * 5.0f;
        // Vertical lines
        glVertex3f(pos, 0.01f, -size);
        glVertex3f(pos, 0.01f, size);
        // Horizontal lines
        glVertex3f(-size, 0.01f, pos);
        glVertex3f(size, 0.01f, pos);
    }
    glEnd();
    
    // Add some 3D objects for reference
    glColor3f(0.8f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    // Simple cube
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    // Back face
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    // Top face
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    // Bottom face
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    // Right face
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    // Left face
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glEnd();
    glPopMatrix();
}

// Render the 3D scene
void RenderScene(void) {
    // Set up 3D perspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75.0, (double)WINDOW_WIDTH / (double)WINDOW_HEIGHT, 0.1, 1000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Apply camera transformation
    float look_x = camera.pos_x - sin(camera.yaw * M_PI / 180.0f) * cos(camera.pitch * M_PI / 180.0f);
    float look_y = camera.pos_y + sin(camera.pitch * M_PI / 180.0f);
    float look_z = camera.pos_z - cos(camera.yaw * M_PI / 180.0f) * cos(camera.pitch * M_PI / 180.0f);
    
    gluLookAt(camera.pos_x, camera.pos_y, camera.pos_z, 
              look_x, look_y, look_z, 
              0.0f, 1.0f, 0.0f);
    
    // Draw the 3D scene
    DrawBasePlate();
}

// Render menu overlay (transparent)
void RenderMenu(void) {
    if (!in_menu) return;

    // Switch to 2D orthographic projection
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

    // Semi-transparent menu background
    glColor4f(0.0f, 0.0f, 0.2f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(100, 100);
    glVertex2f(WINDOW_WIDTH - 100, 100);
    glVertex2f(WINDOW_WIDTH - 100, WINDOW_HEIGHT - 100);
    glVertex2f(100, WINDOW_HEIGHT - 100);
    glEnd();

    // Menu border
    glColor4f(0.3f, 0.3f, 0.8f, 0.9f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(100, 100);
    glVertex2f(WINDOW_WIDTH - 100, 100);
    glVertex2f(WINDOW_WIDTH - 100, WINDOW_HEIGHT - 100);
    glVertex2f(100, WINDOW_HEIGHT - 100);
    glEnd();

    // Title
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glRasterPos2f(200, WINDOW_HEIGHT - 200);
    // Title: "=== CHAOSFORGE ARENA ==="

    // Fighting style options
    glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
    glRasterPos2f(200, WINDOW_HEIGHT - 300);
    // Subtitle: "Choose Your Fighting Style:"

    for (int i = 0; i < 4; i++) {
        float y = WINDOW_HEIGHT - 350 - i * 50;

        if (i == selected_style) {
            glColor4f(1.0f, 1.0f, 0.3f, 1.0f);  // Highlight selected
            // Draw selection indicator
            glBegin(GL_QUADS);
            glVertex2f(180, y - 10);
            glVertex2f(190, y - 10);
            glVertex2f(190, y + 20);
            glVertex2f(180, y + 20);
            glEnd();
        } else {
            glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
        }

        glRasterPos2f(220, y);
        // Draw fighting style name (simplified)
    }

    // Controls help
    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
    glRasterPos2f(200, 150);
    // "WASD: Fly Camera, QE: Up/Down"
    glRasterPos2f(200, 130);
    // "Right Mouse: Look Around"
    glRasterPos2f(200, 110);
    // "UP/DOWN: Select, ENTER: Start"

    // Restore 3D settings
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Main render function
void Render(void) {
    // Clear with transparent black (no gray blob!)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Always render 3D scene first
    RenderScene();

    // Overlay menu if in menu mode
    RenderMenu();

    // Swap buffers
    SwapBuffers(g_hDC);
}

// Handle mouse input for camera look
void HandleMouseLook(int x, int y) {
    static int last_x = -1, last_y = -1;
    static int mouse_right_down = 0;

    if (last_x == -1) {
        last_x = x;
        last_y = y;
        return;
    }

    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
        if (!mouse_right_down) {
            // Just pressed - capture mouse
            SetCapture(g_hWnd);
            ShowCursor(FALSE);
            mouse_right_down = 1;
            camera.mouse_captured = 1;
        }

        int dx = x - last_x;
        int dy = y - last_y;

        camera.yaw += dx * camera.sensitivity;
        camera.pitch -= dy * camera.sensitivity;

        // Constrain pitch
        if (camera.pitch > 89.0f) camera.pitch = 89.0f;
        if (camera.pitch < -89.0f) camera.pitch = -89.0f;

        // Wrap yaw
        while (camera.yaw > 360.0f) camera.yaw -= 360.0f;
        while (camera.yaw < 0.0f) camera.yaw += 360.0f;

        // Reset cursor to center for continuous look
        POINT center = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
        ClientToScreen(g_hWnd, &center);
        SetCursorPos(center.x, center.y);
        last_x = WINDOW_WIDTH / 2;
        last_y = WINDOW_HEIGHT / 2;
    } else {
        if (mouse_right_down) {
            // Just released - release mouse
            ReleaseCapture();
            ShowCursor(TRUE);
            mouse_right_down = 0;
            camera.mouse_captured = 0;
        }
        last_x = x;
        last_y = y;
    }
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            return 0;

        case WM_SIZE:
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_KEYDOWN:
            keys[wParam] = 1;

            // Menu navigation
            if (in_menu) {
                if (wParam == VK_UP) {
                    selected_style = (selected_style - 1 + 4) % 4;
                }
                if (wParam == VK_DOWN) {
                    selected_style = (selected_style + 1) % 4;
                }
                if (wParam == VK_RETURN) {
                    // Start game
                    printf("[Game] Starting game with style: %s\n", fighting_styles[selected_style]);
                    in_menu = 0;  // Start game
                }
            }

            // F12 for real-time refresh
            if (wParam == VK_F12) {
                printf("[Game] F12 pressed - refreshing systems\n");
                InvalidateRect(hWnd, NULL, TRUE);
                // on_f12_pressed();  // Refresh multiplayer systems (disabled)
            }

            // ESC to toggle menu
            if (wParam == VK_ESCAPE) {
                in_menu = !in_menu;
            }

            return 0;

        case WM_KEYUP:
            keys[wParam] = 0;
            return 0;

        case WM_MOUSEMOVE:
            HandleMouseLook(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_MOUSEWHEEL:
            {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                camera.speed += delta / 1200.0f;
                if (camera.speed < 0.1f) camera.speed = 0.1f;
                if (camera.speed > 10.0f) camera.speed = 10.0f;
            }
            return 0;

        case WM_CLOSE:
            running = 0;
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = "ChaosForgeArenaNew";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_OK);
        return -1;
    }

    // Create window
    g_hWnd = CreateWindow(
        "ChaosForgeArenaNew",
        "ChaosForge Arena - New Engine",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!g_hWnd) {
        MessageBox(NULL, "Failed to create window", "Error", MB_OK);
        return -1;
    }

    // Initialize OpenGL
    if (!InitOpenGL(g_hWnd)) {
        MessageBox(NULL, "Failed to initialize OpenGL", "Error", MB_OK);
        return -1;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // Main loop
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

        // Update camera
        UpdateCamera();

        // Render at 60 FPS
        DWORD currentTime = GetTickCount();
        if (currentTime - lastTime >= 16) {
            Render();
            lastTime = currentTime;
        }

        Sleep(1);
    }

    // Cleanup multiplayer (disabled)
    // if (multiplayer_active) {
    //     cleanup_multiplayer_engine();
    // }

    CleanupOpenGL();
    return 0;
}
