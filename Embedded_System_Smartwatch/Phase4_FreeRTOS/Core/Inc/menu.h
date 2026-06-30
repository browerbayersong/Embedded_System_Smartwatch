#ifndef __MENU_H
#define __MENU_H

#include "smartwatch_ui.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MENU_SCREEN_WATCH_FACE = 0,
    MENU_SCREEN_SENSOR_DATA,
    MENU_SCREEN_ACTIVITY,
    MENU_SCREEN_SETTINGS,
    MENU_SCREEN_DEVICE_INFO,
    MENU_SCREEN_COUNT
} MenuScreen_t;

typedef enum {
    MENU_ACTION_NONE = 0,
    MENU_ACTION_ROTATE_CW,
    MENU_ACTION_ROTATE_CCW,
    MENU_ACTION_PRESS,
    MENU_ACTION_LONG_PRESS
} MenuAction_t;

typedef struct {
    MenuScreen_t current_screen;
    uint8_t selected_index;
    uint8_t brightness;        /* 0..4 scale, can be used by display task */
    bool needs_redraw;
    bool in_submenu;
} MenuContext_t;

void Menu_Init(MenuContext_t *context);
void Menu_ProcessRotation(MenuContext_t *context, int16_t delta);
void Menu_ProcessButton(MenuContext_t *context, bool long_press);
void Menu_Render(SmartWatchData_t *data, MenuContext_t *context);

#endif /* __MENU_H */
