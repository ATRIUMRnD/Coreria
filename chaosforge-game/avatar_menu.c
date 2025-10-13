#include "avatar_menu.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

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

// Simple text rendering replacement (placeholder)
static void simple_bitmap_character(int character) {
    // This is a placeholder - in a real implementation you'd render bitmap fonts
    // For now, just render a small square for each character
    (void)character;
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(8, 0);
    glVertex2f(8, 12);
    glVertex2f(0, 12);
    glEnd();
    glTranslatef(10, 0, 0); // Move to next character position
}

#define glutBitmapCharacter(font, character) simple_bitmap_character(character)
#define GLUT_BITMAP_HELVETICA_18 0

// Global state
static bool g_avatar_menu_initialized = false;

void avatar_menu_init(void) {
    if (g_avatar_menu_initialized) return;
    
    // Initialize avatar system if not already done
    gangbeast_avatar_init();
    
    g_avatar_menu_initialized = true;
    printf("[Avatar Menu] Initialized avatar customization menu\n");
}

void avatar_menu_cleanup(void) {
    if (!g_avatar_menu_initialized) return;
    
    g_avatar_menu_initialized = false;
    printf("[Avatar Menu] Cleaned up avatar menu system\n");
}

AvatarMenu* avatar_menu_create(int player_id) {
    if (!g_avatar_menu_initialized) {
        avatar_menu_init();
    }
    
    AvatarMenu* menu = (AvatarMenu*)malloc(sizeof(AvatarMenu));
    if (!menu) return NULL;
    
    // Initialize menu state
    memset(menu, 0, sizeof(AvatarMenu));
    
    menu->current_state = AVATAR_MENU_BODY_TYPE;
    menu->selected_option = 0;
    menu->menu_active = false;
    menu->player_id = player_id;
    
    menu->menu_time = 0.0f;
    menu->transition_time = 0.0f;
    menu->preview_rotation = 0.0f;
    menu->preview_detail = DETAIL_HIGH;
    
    menu->body_type_selection = BODY_TYPE_NORMAL;
    menu->color_scheme_selection = COLOR_SCHEME_RED;
    menu->accessory_selection = ACCESSORY_NONE;
    
    menu->animate_preview = true;
    menu->animation_speed = 1.0f;
    
    // Create preview avatar
    menu->preview_avatar = gangbeast_avatar_create(BODY_TYPE_NORMAL, COLOR_SCHEME_RED);
    
    return menu;
}

void avatar_menu_destroy(AvatarMenu* menu) {
    if (!menu) return;
    
    if (menu->preview_avatar) {
        gangbeast_avatar_destroy(menu->preview_avatar);
    }
    
    free(menu);
}

void avatar_menu_reset(AvatarMenu* menu) {
    if (!menu) return;
    
    menu->current_state = AVATAR_MENU_BODY_TYPE;
    menu->selected_option = 0;
    menu->menu_time = 0.0f;
    menu->transition_time = 0.0f;
    
    menu->body_type_selection = BODY_TYPE_NORMAL;
    menu->color_scheme_selection = COLOR_SCHEME_RED;
    menu->accessory_selection = ACCESSORY_NONE;
    
    if (menu->preview_avatar) {
        gangbeast_avatar_destroy(menu->preview_avatar);
        menu->preview_avatar = gangbeast_avatar_create(BODY_TYPE_NORMAL, COLOR_SCHEME_RED);
    }
}

void avatar_menu_show(AvatarMenu* menu) {
    if (!menu) return;
    menu->menu_active = true;
    menu->menu_time = 0.0f;
}

void avatar_menu_hide(AvatarMenu* menu) {
    if (!menu) return;
    menu->menu_active = false;
}

bool avatar_menu_is_active(const AvatarMenu* menu) {
    return menu && menu->menu_active;
}

void avatar_menu_handle_key_up(AvatarMenu* menu) {
    if (!menu || !menu->menu_active) return;
    
    if (menu->selected_option > 0) {
        menu->selected_option--;
    } else {
        menu->selected_option = avatar_menu_get_option_count(menu) - 1;
    }
}

void avatar_menu_handle_key_down(AvatarMenu* menu) {
    if (!menu || !menu->menu_active) return;
    
    menu->selected_option = (menu->selected_option + 1) % avatar_menu_get_option_count(menu);
}

void avatar_menu_handle_key_left(AvatarMenu* menu) {
    if (!menu || !menu->menu_active) return;
    
    switch (menu->current_state) {
        case AVATAR_MENU_BODY_TYPE:
            if (menu->body_type_selection > 0) {
                menu->body_type_selection--;
                avatar_menu_apply_current_selection(menu);
            }
            break;
        case AVATAR_MENU_COLOR_SCHEME:
            if (menu->color_scheme_selection > 0) {
                menu->color_scheme_selection--;
                avatar_menu_apply_current_selection(menu);
            }
            break;
        case AVATAR_MENU_ACCESSORIES:
            // Toggle accessories off
            if (menu->accessory_selection & (1 << menu->selected_option)) {
                menu->accessory_selection &= ~(1 << menu->selected_option);
                avatar_menu_apply_current_selection(menu);
            }
            break;
        default:
            break;
    }
}

void avatar_menu_handle_key_right(AvatarMenu* menu) {
    if (!menu || !menu->menu_active) return;
    
    switch (menu->current_state) {
        case AVATAR_MENU_BODY_TYPE:
            if (menu->body_type_selection < BODY_TYPE_COUNT - 1) {
                menu->body_type_selection++;
                avatar_menu_apply_current_selection(menu);
            }
            break;
        case AVATAR_MENU_COLOR_SCHEME:
            if (menu->color_scheme_selection < COLOR_SCHEME_COUNT - 1) {
                menu->color_scheme_selection++;
                avatar_menu_apply_current_selection(menu);
            }
            break;
        case AVATAR_MENU_ACCESSORIES:
            // Toggle accessories on
            menu->accessory_selection |= (1 << menu->selected_option);
            avatar_menu_apply_current_selection(menu);
            break;
        default:
            break;
    }
}

void avatar_menu_handle_key_enter(AvatarMenu* menu) {
    if (!menu || !menu->menu_active) return;
    
    switch (menu->current_state) {
        case AVATAR_MENU_BODY_TYPE:
        case AVATAR_MENU_COLOR_SCHEME:
        case AVATAR_MENU_ACCESSORIES:
            // Move to next menu state
            menu->current_state = (AvatarMenuState)((menu->current_state + 1) % AVATAR_MENU_COUNT);
            menu->selected_option = 0;
            menu->transition_time = 0.0f;
            break;
        case AVATAR_MENU_PREVIEW:
            menu->current_state = AVATAR_MENU_CONFIRM;
            menu->selected_option = 0;
            break;
        case AVATAR_MENU_CONFIRM:
            if (menu->selected_option == 0) {
                // Confirm selection
                avatar_menu_hide(menu);
            } else {
                // Go back to body type selection
                menu->current_state = AVATAR_MENU_BODY_TYPE;
                menu->selected_option = 0;
            }
            break;
        case AVATAR_MENU_COUNT:
            // Should not happen
            break;
    }
}

void avatar_menu_handle_key_escape(AvatarMenu* menu) {
    if (!menu || !menu->menu_active) return;
    
    if (menu->current_state == AVATAR_MENU_BODY_TYPE) {
        avatar_menu_hide(menu);
    } else {
        // Go back to previous menu state
        menu->current_state = (AvatarMenuState)((menu->current_state - 1 + AVATAR_MENU_COUNT) % AVATAR_MENU_COUNT);
        menu->selected_option = 0;
        menu->transition_time = 0.0f;
    }
}

void avatar_menu_update(AvatarMenu* menu, float delta_time) {
    if (!menu) return;
    
    menu->menu_time += delta_time;
    menu->transition_time += delta_time;
    
    if (menu->animate_preview) {
        menu->preview_rotation += delta_time * 30.0f * menu->animation_speed; // 30 degrees per second
        if (menu->preview_rotation > 360.0f) {
            menu->preview_rotation -= 360.0f;
        }
        
        // Update avatar animation
        if (menu->preview_avatar) {
            gangbeast_avatar_update_animation(menu->preview_avatar, delta_time);
        }
    }
}

int avatar_menu_get_option_count(const AvatarMenu* menu) {
    if (!menu) return 0;
    
    switch (menu->current_state) {
        case AVATAR_MENU_BODY_TYPE:
            return BODY_TYPE_COUNT;
        case AVATAR_MENU_COLOR_SCHEME:
            return COLOR_SCHEME_COUNT;
        case AVATAR_MENU_ACCESSORIES:
            return 6; // Number of accessory types
        case AVATAR_MENU_PREVIEW:
            return 1;
        case AVATAR_MENU_CONFIRM:
            return 2; // Confirm or Back
        default:
            return 1;
    }
}

void avatar_menu_apply_current_selection(AvatarMenu* menu) {
    if (!menu || !menu->preview_avatar) return;
    
    // Update preview avatar based on current selections
    gangbeast_avatar_set_body_type(menu->preview_avatar, (AvatarBodyType)menu->body_type_selection);
    gangbeast_avatar_set_color_scheme(menu->preview_avatar, (AvatarColorScheme)menu->color_scheme_selection);
    
    // Apply accessories
    menu->preview_avatar->accessories = menu->accessory_selection;
}

GangbeastAvatar* avatar_menu_create_final_avatar(const AvatarMenu* menu) {
    if (!menu || !menu->preview_avatar) return NULL;

    GangbeastAvatar* final_avatar = gangbeast_avatar_create(
        (AvatarBodyType)menu->body_type_selection,
        (AvatarColorScheme)menu->color_scheme_selection
    );

    if (final_avatar) {
        final_avatar->accessories = menu->accessory_selection;
    }

    return final_avatar;
}

// Helper function to render text (simple bitmap text)
static void render_text(float x, float y, const char* text, bool selected) {
    glRasterPos2f(x, y);

    if (selected) {
        glColor3f(1.0f, 1.0f, 0.0f); // Yellow for selected
    } else {
        glColor3f(1.0f, 1.0f, 1.0f); // White for normal
    }

    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

void avatar_menu_render_3d_preview(const AvatarMenu* menu) {
    if (!menu || !menu->preview_avatar) return;

    // Set up 3D viewport for preview
    glViewport(400, 100, 400, 400); // Right side of screen

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Camera position for preview
    float cam_distance = 4.0f;
    float cam_x = cam_distance * sinf(menu->preview_rotation * M_PI / 180.0f);
    float cam_z = cam_distance * cosf(menu->preview_rotation * M_PI / 180.0f);
    gluLookAt(cam_x, 2.0f, cam_z, 0, 1, 0, 0, 1, 0);

    // Enable depth testing for 3D preview
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Render the avatar
    gangbeast_avatar_render(menu->preview_avatar, 0, 0, 0, 0, menu->preview_detail);

    // Simple ground plane
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-2, 0, -2);
    glVertex3f(2, 0, -2);
    glVertex3f(2, 0, 2);
    glVertex3f(-2, 0, 2);
    glEnd();

    glDisable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // Restore full viewport
    glViewport(0, 0, 800, 600);
}

void avatar_menu_render_ui_overlay(const AvatarMenu* menu) {
    if (!menu) return;

    // Set up 2D orthographic projection for UI
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    // Menu title
    glColor3f(1.0f, 1.0f, 1.0f);
    char title[64];
    snprintf(title, sizeof(title), "Player %d Avatar Customization", menu->player_id + 1);
    render_text(50, 50, title, false);

    // Current menu state title
    const char* state_titles[] = {
        "Body Type", "Color Scheme", "Accessories", "Preview", "Confirm"
    };

    if (menu->current_state < AVATAR_MENU_COUNT) {
        render_text(50, 100, state_titles[menu->current_state], false);
    }

    // Menu options
    float option_y = 150;

    switch (menu->current_state) {
        case AVATAR_MENU_BODY_TYPE:
            for (int i = 0; i < BODY_TYPE_COUNT; i++) {
                bool selected = (i == menu->body_type_selection);
                render_text(70, option_y + i * 30,
                           gangbeast_avatar_get_body_type_name((AvatarBodyType)i),
                           selected);
            }
            break;

        case AVATAR_MENU_COLOR_SCHEME:
            for (int i = 0; i < COLOR_SCHEME_COUNT; i++) {
                bool selected = (i == menu->color_scheme_selection);
                render_text(70, option_y + i * 30,
                           gangbeast_avatar_get_color_scheme_name((AvatarColorScheme)i),
                           selected);
            }
            break;

        case AVATAR_MENU_ACCESSORIES:
            {
                const char* accessory_names[] = {
                    "Hat", "Glasses", "Mask", "Cape", "Gloves", "Boots"
                };

                for (int i = 0; i < 6; i++) {
                    bool selected = (i == menu->selected_option);
                    bool enabled = (menu->accessory_selection & (1 << (i + 1))) != 0;

                    char option_text[64];
                    snprintf(option_text, sizeof(option_text), "%s %s",
                            accessory_names[i], enabled ? "[ON]" : "[OFF]");

                    render_text(70, option_y + i * 30, option_text, selected);
                }
            }
            break;

        case AVATAR_MENU_PREVIEW:
            render_text(70, option_y, "Use arrow keys to rotate", false);
            render_text(70, option_y + 30, "Press ENTER to continue", true);
            break;

        case AVATAR_MENU_CONFIRM:
            render_text(70, option_y, "Confirm Avatar", menu->selected_option == 0);
            render_text(70, option_y + 30, "Back to Customization", menu->selected_option == 1);
            break;
        case AVATAR_MENU_COUNT:
            // Should not happen
            break;
    }

    // Instructions
    glColor3f(0.7f, 0.7f, 0.7f);
    render_text(50, 550, "Arrow Keys: Navigate | ENTER: Select | ESC: Back", false);

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void avatar_menu_render(const AvatarMenu* menu) {
    if (!menu || !menu->menu_active) return;

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render 3D preview
    avatar_menu_render_3d_preview(menu);

    // Render UI overlay
    avatar_menu_render_ui_overlay(menu);
}

const char* avatar_menu_get_current_option_text(const AvatarMenu* menu) {
    if (!menu) return "Unknown";

    switch (menu->current_state) {
        case AVATAR_MENU_BODY_TYPE:
            return gangbeast_avatar_get_body_type_name((AvatarBodyType)menu->body_type_selection);
        case AVATAR_MENU_COLOR_SCHEME:
            return gangbeast_avatar_get_color_scheme_name((AvatarColorScheme)menu->color_scheme_selection);
        case AVATAR_MENU_ACCESSORIES:
            return "Accessories";
        case AVATAR_MENU_PREVIEW:
            return "Preview";
        case AVATAR_MENU_CONFIRM:
            return "Confirm";
        default:
            return "Unknown";
    }
}

// Preset functions
void avatar_menu_apply_preset_brawler(AvatarMenu* menu) {
    if (!menu) return;

    menu->body_type_selection = BODY_TYPE_CHUNKY;
    menu->color_scheme_selection = COLOR_SCHEME_RED;
    menu->accessory_selection = ACCESSORY_GLOVES;
    avatar_menu_apply_current_selection(menu);
}

void avatar_menu_apply_preset_striker(AvatarMenu* menu) {
    if (!menu) return;

    menu->body_type_selection = BODY_TYPE_SLIM;
    menu->color_scheme_selection = COLOR_SCHEME_GREEN;
    menu->accessory_selection = ACCESSORY_BOOTS;
    avatar_menu_apply_current_selection(menu);
}

void avatar_menu_apply_preset_phantom(AvatarMenu* menu) {
    if (!menu) return;

    menu->body_type_selection = BODY_TYPE_TALL;
    menu->color_scheme_selection = COLOR_SCHEME_BLUE;
    menu->accessory_selection = ACCESSORY_CAPE | ACCESSORY_MASK;
    avatar_menu_apply_current_selection(menu);
}

void avatar_menu_apply_preset_titan(AvatarMenu* menu) {
    if (!menu) return;

    menu->body_type_selection = BODY_TYPE_CHUNKY;
    menu->color_scheme_selection = COLOR_SCHEME_YELLOW;
    menu->accessory_selection = ACCESSORY_HAT | ACCESSORY_GLOVES | ACCESSORY_BOOTS;
    avatar_menu_apply_current_selection(menu);
}
