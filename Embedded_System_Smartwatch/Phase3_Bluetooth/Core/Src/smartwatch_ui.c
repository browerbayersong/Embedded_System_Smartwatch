#include "smartwatch_ui.h"
#include "oled.h"
#include "soft_rtc.h"
#include <stdio.h>
#include <string.h>

/* Weekday strings */
static const char *weekdays_en[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

static void format_signed_fixed1(char *buf, size_t size, float value) {
    int scaled = (int)(value * 10.0f + (value >= 0.0f ? 0.5f : -0.5f));
    char sign = '+';

    if (scaled < 0) {
        sign = '-';
        scaled = -scaled;
    }

    snprintf(buf, size, "%c%d.%d", sign, scaled / 10, scaled % 10);
}

static void format_signed_int(char *buf, size_t size, float value) {
    int rounded = (int)(value + (value >= 0.0f ? 0.5f : -0.5f));
    char sign = '+';

    if (rounded < 0) {
        sign = '-';
        rounded = -rounded;
    }

    snprintf(buf, size, "%c%d", sign, rounded);
}

/* ==================== Data Init ==================== */

void UI_InitData(SmartWatchData_t *data) {
    data->hour = rtc_time.hour;
    data->minute = rtc_time.min;
    data->second = rtc_time.sec;
    data->year = 2026;
    data->month = 7;
    data->day = 2;
    data->weekday = 4;       /* Thursday */
    data->battery_pct = 85;
    data->temp_celsius = 25;
    data->imu_status = 0;
    data->bt_connected = 0;
    data->accel.ax = 0.0f;
    data->accel.ay = 0.0f;
    data->accel.az = 9.81f;
    data->gyro.gx = 0.0f;
    data->gyro.gy = 0.0f;
    data->gyro.gz = 0.0f;
    data->angle.pitch = 0.0f;
    data->angle.roll = 0.0f;
    data->step_count = 0;
    data->distance_m = 0.0f;
    data->calories = 0.0f;
}

/* ==================== Status Bar ==================== */

void UI_DrawStatusBar(SmartWatchData_t *data) {
    /* Clear page 0 */
    for (uint16_t i = 0; i < SSD1306_WIDTH; i++)
        OLED_Buffer[i] = 0x00;

    /* Battery icon (left side, x=0-14) */
    for (uint8_t x = 0; x < 14; x++) {
        if (x == 0 || x == 13)
            OLED_Buffer[x] = 0x7E;
        else if (x == 14)
            OLED_Buffer[x] = 0x18;
        else
            OLED_Buffer[x] = 0x42;
    }
    uint8_t fill_width = (data->battery_pct * 10) / 100;
    if (fill_width > 10) fill_width = 10;
    for (uint8_t x = 2; x < 2 + fill_width; x++)
        OLED_Buffer[x] = 0x5A;

    /* Battery percentage */
    char bat_str[5];
    snprintf(bat_str, sizeof(bat_str), "%d%%", data->battery_pct);
    OLED_DrawString6x8(18, 0, bat_str);

    /* Time in center */
    char time_str[9];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", data->hour, data->minute);
    OLED_DrawString6x8(60, 0, time_str);

    /* Bluetooth status indicator */
    if (data->bt_connected) {
        OLED_DrawString6x8(102, 0, "BT");
    }

    /* IMU status dot (right side) */
    if (data->imu_status) {
        OLED_DrawFilledCircle(123, 3, 2);
    } else {
        OLED_DrawCircle(123, 3, 2);
    }

    /* Separator line at y=7 */
    for (uint8_t x = 0; x < SSD1306_WIDTH; x++)
        OLED_SetPixel(x, 7, 1);
}

/* ==================== Page Indicator ==================== */

void UI_DrawPageIndicator(uint8_t current, uint8_t total) {
    for (uint16_t i = 7 * SSD1306_WIDTH; i < SSD1306_WIDTH * SSD1306_PAGES; i++)
        OLED_Buffer[i] = 0x00;

    for (uint8_t x = 0; x < SSD1306_WIDTH; x++)
        OLED_SetPixel(x, 56, 1);

    uint8_t dot_spacing = 14;
    uint8_t total_width = (total - 1) * dot_spacing;
    uint8_t start_x = (SSD1306_WIDTH - total_width) / 2;

    for (uint8_t i = 0; i < total; i++) {
        uint8_t cx = start_x + i * dot_spacing;
        if (i == current) {
            OLED_DrawFilledCircle(cx, 60, 2);
        } else {
            OLED_DrawCircle(cx, 60, 2);
        }
    }
}

/* ==================== Page 0: Watch Face ==================== */

void UI_DrawWatchFace(SmartWatchData_t *data) {
    OLED_ClearBuffer();

    UI_DrawStatusBar(data);

    /* Large time HH:MM at pages 1-3 (y=8..31), centered horizontally.
     * 16x24 font = 3 pages. 5 digits × 16 cols + 4 gaps × 2 = 88px wide.
     * y=8 is page-aligned.
     */
    uint8_t time_x = (SSD1306_WIDTH - 88) / 2;
    OLED_DrawTime16x24(time_x, 8, data->hour, data->minute);

    /* Date line at page 4 (y=32..39): "2026-07-06 Mon" */
    char date_str[24];
    snprintf(date_str, sizeof(date_str), "%04d-%02d-%02d %s",
             data->year, data->month, data->day,
             weekdays_en[data->weekday]);
    uint8_t date_len = strlen(date_str);
    uint8_t date_x = (SSD1306_WIDTH - date_len * 6) / 2;
    OLED_DrawString6x8(date_x, 4, date_str);

    /* IMU / temperature at page 5 (y=40..47) */
    char info_str[24];
    if (data->imu_status) {
        snprintf(info_str, sizeof(info_str), "T:%d C  steps:%d",
                 data->temp_celsius, data->step_count);
    } else {
        snprintf(info_str, sizeof(info_str), "T:-- C  steps:%d",
                 data->step_count);
    }
    uint8_t info_len = strlen(info_str);
    uint8_t info_x = (SSD1306_WIDTH - info_len * 6) / 2;
    OLED_DrawString6x8(info_x, 5, info_str);

    UI_DrawPageIndicator(PAGE_WATCH_FACE, PAGE_MAX);
}

/* ==================== Page 1: IMU Sensor Detail ==================== */

void UI_DrawIMU(SmartWatchData_t *data) {
    OLED_ClearBuffer();

    UI_DrawStatusBar(data);

    OLED_DrawString8x16(4, 1, "IMU  MPU6050");

    for (uint8_t x = 0; x < SSD1306_WIDTH; x++)
        OLED_SetPixel(x, 24, 1);

    if (!data->imu_status) {
        OLED_DrawString6x8(4, 4, "MPU6050: NOT FOUND");
        OLED_DrawString6x8(4, 5, "PB10/PB11  I2C2");
        OLED_DrawString6x8(4, 6, "Auto-retry every 5s");
    } else {
        char buf[48];
        char ax[16], ay[16], az[16];
        char gx[16], gy[16], gz[16];
        char pitch[16], roll[16];

        format_signed_fixed1(ax, sizeof(ax), data->accel.ax);
        format_signed_fixed1(ay, sizeof(ay), data->accel.ay);
        format_signed_fixed1(az, sizeof(az), data->accel.az);
        format_signed_int(gx, sizeof(gx), data->gyro.gx);
        format_signed_int(gy, sizeof(gy), data->gyro.gy);
        format_signed_int(gz, sizeof(gz), data->gyro.gz);
        format_signed_int(pitch, sizeof(pitch), data->angle.pitch);
        format_signed_int(roll, sizeof(roll), data->angle.roll);

        /* Accel: "X+0.1 Y+0.3 Z+9.8" = 17 chars = 102px */
        snprintf(buf, sizeof(buf), "X%s Y%s Z%s", ax, ay, az);
        OLED_DrawString6x8(0, 3, buf);

        /* Gyro: "x+0 y+0 z+0 d/s" = 14 chars = 84px */
        snprintf(buf, sizeof(buf), "x%s y%s z%s d/s", gx, gy, gz);
        OLED_DrawString6x8(0, 4, buf);

        /* Pitch & Roll: "P+0 R+0 deg" = 12 chars = 72px */
        snprintf(buf, sizeof(buf), "P%s R%s deg", pitch, roll);
        OLED_DrawString6x8(0, 5, buf);

        /* Temperature + I2C status */
        snprintf(buf, sizeof(buf), "T:%dC %s",
                 data->temp_celsius,
                 g_mpu6050_i2c_error ? "ERR" : "OK");
        OLED_DrawString6x8(0, 6, buf);

        /* Attitude indicator (right side, away from text) */
        uint8_t cx = 114, cy = 40, r = 7;

        OLED_DrawCircle(cx, cy, r);
        OLED_DrawHLine(cx - r, cy, 2 * r + 1);
        OLED_DrawVLine(cx, cy - r, 2 * r + 1);

        int8_t dot_x = cx + (int8_t)(data->angle.roll * 0.12f);
        int8_t dot_y = cy + (int8_t)(data->angle.pitch * 0.12f);

        if (dot_x < cx - r + 2) dot_x = cx - r + 2;
        if (dot_x > cx + r - 2) dot_x = cx + r - 2;
        if (dot_y < cy - r + 2) dot_y = cy - r + 2;
        if (dot_y > cy + r - 2) dot_y = cy + r - 2;

        OLED_DrawFilledCircle(dot_x, dot_y, 2);
    }

    UI_DrawPageIndicator(PAGE_IMU, PAGE_MAX);
}

/* ==================== Page 2: Bluetooth ==================== */

void UI_DrawBluetooth(SmartWatchData_t *data) {
    OLED_ClearBuffer();

    UI_DrawStatusBar(data);

    OLED_DrawString8x16(4, 1, "BLUETOOTH");

    for (uint8_t x = 0; x < SSD1306_WIDTH; x++)
        OLED_SetPixel(x, 24, 1);

    OLED_DrawString6x8(4, 3, "HC-05 USART2 38400");
    OLED_DrawString6x8(4, 4, "TX: PA2  RX: PA3");

    if (data->bt_connected) {
        OLED_DrawString6x8(4, 6, "Status: CONNECTED");
    } else {
        OLED_DrawString6x8(4, 6, "Status: WAITING");
    }

    UI_DrawPageIndicator(PAGE_BLUETOOTH, PAGE_MAX);
}

/* ==================== Page 3: Device Info ==================== */

void UI_DrawDeviceInfo(SmartWatchData_t *data) {
    OLED_ClearBuffer();

    UI_DrawStatusBar(data);

    OLED_DrawString8x16(4, 1, "DEV INFO");

    for (uint8_t x = 0; x < SSD1306_WIDTH; x++)
        OLED_SetPixel(x, 24, 1);

    OLED_DrawString6x8(4, 3, "STM32F103C8T6  72MHz");
    OLED_DrawString6x8(4, 4, "OLED: I2C1 PB6/PB7");
    OLED_DrawString6x8(4, 5, "IMU:  I2C2 PB10/PB11");
    OLED_DrawString6x8(4, 6, "BT:   USART2 PA2/PA3");

    /* MPU6050 status */
    if (data->imu_status) {
        OLED_DrawString6x8(4, 7, "MPU6050: OK");
    } else {
        OLED_DrawString6x8(4, 7, "MPU6050: NOT FOUND");
    }

    UI_DrawPageIndicator(PAGE_DEVICE_INFO, PAGE_MAX);
}

/* ==================== Page Dispatcher ==================== */

void UI_DrawPage(UIPage_t page, SmartWatchData_t *data) {
    switch (page) {
        case PAGE_WATCH_FACE:  UI_DrawWatchFace(data);   break;
        case PAGE_IMU:         UI_DrawIMU(data);          break;
        case PAGE_BLUETOOTH:   UI_DrawBluetooth(data);    break;
        case PAGE_DEVICE_INFO: UI_DrawDeviceInfo(data);   break;
        case PAGE_ACTIVITY:    {
            /* Activity page - 画在 PAGE_DEVICE_INFO 之后 */
            OLED_ClearBuffer();
            UI_DrawStatusBar(data);
            OLED_DrawString8x16(4, 1, "ACTIVITY");

            for (uint8_t x = 0; x < SSD1306_WIDTH; x++)
                OLED_SetPixel(x, 24, 1);

            char line[22];
            snprintf(line, sizeof(line), "Steps: %lu", (unsigned long)data->step_count);
            OLED_DrawString6x8(4, 3, line);
            snprintf(line, sizeof(line), "Dist: %.1f m", data->distance_m);
            OLED_DrawString6x8(4, 4, line);
            snprintf(line, sizeof(line), "Cal: %.1f kcal", data->calories);
            OLED_DrawString6x8(4, 5, line);

            UI_DrawPageIndicator(PAGE_ACTIVITY, PAGE_MAX);
            OLED_Update();
        } break;
        default: break;
    }
    if (page != PAGE_ACTIVITY) OLED_Update();
}