#ifndef AVATAR_MENU_H
#define AVATAR_MENU_H

#include "gangbeast_avatar.h"
#include <stdbool.h>

// Avatar menu state
typedef enum {
    AVATAR_MENU_BODY_TYPE = 0,
    AVATAR_MENU_COLOR_SCHEME = 1,
    AVATAR_MENU_ACCESSORIES = 2,
    AVATAR_MENU_PREVIEW = 3,
    AVATAR_MENU_CONFIRM = 4,
    AVATAR_MENU_COUNT
} AvatarMenuState;

// Avatar customization menu
typedef struct {
    AvatarMenuState current_state;
    int selected_option;
    bool menu_active;
    
    // Current avatar being customized
    GangbeastAvatar* preview_avatar;
    
    // Menu navigation
    float menu_time;
    float transition_time;
    
    // Preview settings
    float preview_rotation;
    RenderDetail preview_detail;
    
    // Player assignment
    int player_id;
    
    // Menu options
    int body_type_selection;
    int color_scheme_selection;
    int accessory_selection;
    
    // Animation
    bool animate_preview;
    float animation_speed;
} AvatarMenu;

// Function declarations
void avatar_menu_init(void);
void avatar_menu_cleanup(void);

// Menu management
AvatarMenu* avatar_menu_create(int player_id);
void avatar_menu_destroy(AvatarMenu* menu);
void avatar_menu_reset(AvatarMenu* menu);

// Menu state management
void avatar_menu_show(AvatarMenu* menu);
void avatar_menu_hide(AvatarMenu* menu);
bool avatar_menu_is_active(const AvatarMenu* menu);

// Input handling
void avatar_menu_handle_key_up(AvatarMenu* menu);
void avatar_menu_handle_key_down(AvatarMenu* menu);
void avatar_menu_handle_key_left(AvatarMenu* menu);
void avatar_menu_handle_key_right(AvatarMenu* menu);
void avatar_menu_handle_key_enter(AvatarMenu* menu);
void avatar_menu_handle_key_escape(AvatarMenu* menu);

// Update and rendering
void avatar_menu_update(AvatarMenu* menu, float delta_time);
void avatar_menu_render(const AvatarMenu* menu);
void avatar_menu_render_3d_preview(const AvatarMenu* menu);
void avatar_menu_render_ui_overlay(const AvatarMenu* menu);

// Avatar creation from menu
GangbeastAvatar* avatar_menu_create_final_avatar(const AvatarMenu* menu);

// Utility functions
const char* avatar_menu_get_current_option_text(const AvatarMenu* menu);
int avatar_menu_get_option_count(const AvatarMenu* menu);
void avatar_menu_apply_current_selection(AvatarMenu* menu);

// Quick preset functions
void avatar_menu_apply_preset_brawler(AvatarMenu* menu);
void avatar_menu_apply_preset_striker(AvatarMenu* menu);
void avatar_menu_apply_preset_phantom(AvatarMenu* menu);
void avatar_menu_apply_preset_titan(AvatarMenu* menu);

#endif // AVATAR_MENU_H
