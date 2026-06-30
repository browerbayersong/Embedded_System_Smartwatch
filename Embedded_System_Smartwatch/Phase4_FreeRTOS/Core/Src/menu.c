#include "menu.h"
#include "oled.h"
#include "smartwatch_ui.h"
#include <string.h>
#include <stdio.h>

static const char *MenuNames[MENU_SCREEN_COUNT] = {
    "Watch Face",
    "Sensor",
    "Activity",
    "Settings",
    "Device"
};

void Menu_Init(MenuContext_t *context) {
    if (context == NULL) {
        return;
    }
    context->current_screen = MENU_SCREEN_WATCH_FACE;
    context->selected_index = 0;
    context->brightness = 3;
    context->needs_redraw = true;
    context->in_submenu = false;
}

void Menu_ProcessRotation(MenuContext_t *context, int16_t delta) {
    if (context == NULL || delta == 0) {
        return;
    }

    if (context->current_screen == MENU_SCREEN_SETTINGS) {
        /* In settings, rotation adjusts brightness */
        if (delta > 0 && context->brightness < 4) {
            context->brightness++;
            context->needs_redraw = true;
        } else if (delta < 0 && context->brightness > 0) {
            context->brightness--;
            context->needs_redraw = true;
        }
        return;
    }

    if (delta > 0) {
        context->current_screen = (MenuScreen_t)((context->current_screen + 1) % MENU_SCREEN_COUNT);
    } else {
        context->current_screen = (MenuScreen_t)((context->current_screen + MENU_SCREEN_COUNT - 1) % MENU_SCREEN_COUNT);
    }
    context->needs_redraw = true;
}

void Menu_ProcessButton(MenuContext_t *context, bool long_press) {
    if (context == NULL) {
        return;
    }

    if (long_press) {
        context->current_screen = MENU_SCREEN_WATCH_FACE;
        context->needs_redraw = true;
        context->in_submenu = false;
        return;
    }

    switch (context->current_screen) {
        case MENU_SCREEN_WATCH_FACE:
            context->current_screen = MENU_SCREEN_SENSOR_DATA;
            break;
        case MENU_SCREEN_SENSOR_DATA:
            context->current_screen = MENU_SCREEN_ACTIVITY;
            break;
        case MENU_SCREEN_ACTIVITY:
            context->in_submenu = !context->in_submenu;
            break;
        case MENU_SCREEN_SETTINGS:
            context->in_submenu = !context->in_submenu;
            break;
        case MENU_SCREEN_DEVICE_INFO:
            context->current_screen = MENU_SCREEN_WATCH_FACE;
            break;
        default:
            context->current_screen = MENU_SCREEN_WATCH_FACE;
            break;
    }
    context->needs_redraw = true;
}

static void Menu_DrawFooter(MenuContext_t *context) {
    char footer[24];
    snprintf(footer, sizeof(footer), "[%s] B:%u",
             MenuNames[context->current_screen], context->brightness);
    OLED_DrawString6x8(0, 6, footer);
}

static void Menu_DrawActivityPage(SmartWatchData_t *data, MenuContext_t *context) {
    OLED_ClearBuffer();
    UI_DrawStatusBar(data);
    OLED_DrawString8x16(0, 1, "Activity");
    for (uint8_t x = 0; x < SSD1306_WIDTH; x++) {
        OLED_SetPixel(x, 24, 1);
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "Steps: %lu", (unsigned long)data->step_count);
    OLED_DrawString6x8(0, 4, buf);

    snprintf(buf, sizeof(buf), "Dist: %.2fm", data->distance_m);
    OLED_DrawString6x8(0, 5, buf);

    snprintf(buf, sizeof(buf), "Cal: %.1fkcal", data->calories);
    OLED_DrawString6x8(0, 6, buf);

    if (context->in_submenu) {
        OLED_DrawString6x8(0, 2, "Press->Reset");
    }
}

static void Menu_DrawSettingsPage(SmartWatchData_t *data, MenuContext_t *context) {
    OLED_ClearBuffer();
    UI_DrawStatusBar(data);
    OLED_DrawString8x16(0, 1, "Settings");
    for (uint8_t x = 0; x < SSD1306_WIDTH; x++) {
        OLED_SetPixel(x, 24, 1);
    }

    char buf[24];
    snprintf(buf, sizeof(buf), "Brightness: %u", context->brightness + 1);
    OLED_DrawString6x8(0, 4, buf);
    OLED_DrawString6x8(0, 5, "Rotate->change");
    OLED_DrawString6x8(0, 6, "Long press->home");
}

static void Menu_DrawSwitchToPage(SmartWatchData_t *data, MenuContext_t *context) {
    if (context == NULL || data == NULL) {
        return;
    }

    switch (context->current_screen) {
        case MENU_SCREEN_WATCH_FACE:
            UI_DrawPage(PAGE_WATCH_FACE, data);
            break;
        case MENU_SCREEN_SENSOR_DATA:
            UI_DrawPage(PAGE_IMU, data);
            break;
        case MENU_SCREEN_DEVICE_INFO:
            UI_DrawPage(PAGE_DEVICE_INFO, data);
            break;
        default:
            UI_DrawPage(PAGE_WATCH_FACE, data);
            break;
    }
}

void Menu_Render(SmartWatchData_t *data, MenuContext_t *context) {
    if (context == NULL || data == NULL) {
        return;
    }

    if (!context->needs_redraw) {
        return;
    }

    switch (context->current_screen) {
        case MENU_SCREEN_WATCH_FACE:
        case MENU_SCREEN_SENSOR_DATA:
        case MENU_SCREEN_DEVICE_INFO:
            Menu_DrawSwitchToPage(data, context);
            break;
        case MENU_SCREEN_ACTIVITY:
            Menu_DrawActivityPage(data, context);
            break;
        case MENU_SCREEN_SETTINGS:
            Menu_DrawSettingsPage(data, context);
            break;
        default:
            Menu_DrawSwitchToPage(data, context);
            break;
    }

    Menu_DrawFooter(context);
    OLED_Update();
    context->needs_redraw = false;
}
