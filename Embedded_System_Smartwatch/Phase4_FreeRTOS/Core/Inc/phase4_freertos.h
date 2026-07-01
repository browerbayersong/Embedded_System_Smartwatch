#ifndef __PHASE4_FREERTOS_H
#define __PHASE4_FREERTOS_H

#include "main.h"
#include "menu.h"
#include "pedometer.h"
#include "oled.h"
#include "mpu6050.h"
#include "bluetooth.h"
#include "smartwatch_ui.h"
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;
} Phase4TimeData_t;

typedef struct {
    int16_t delta;
    uint8_t button_event;
} Phase4MenuMsg_t;

typedef struct {
    SmartWatchData_t watch_data;
    MenuContext_t menu_context;
    PedometerState_t pedometer_state;
    QueueHandle_t sensor_queue;
    QueueHandle_t time_queue;
    QueueHandle_t menu_queue;
    QueueHandle_t bt_queue;
    SemaphoreHandle_t display_sem;
    SemaphoreHandle_t i2c_mutex;
    TaskHandle_t task_time_keep;
    TaskHandle_t task_sensor;
    TaskHandle_t task_display;
    TaskHandle_t task_menu;
    TaskHandle_t task_bluetooth;
} Phase4Runtime_t;

void Phase4_InitRuntime(Phase4Runtime_t *runtime);
void Phase4_StartTasks(Phase4Runtime_t *runtime);
void vTask_TimeKeep(void *pvParameters);
void vTask_Sensor(void *pvParameters);
void vTask_Display(void *pvParameters);
void vTask_Menu(void *pvParameters);
void vTask_Bluetooth(void *pvParameters);

#endif /* __PHASE4_FREERTOS_H */
