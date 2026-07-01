#include "phase4_freertos.h"
#include <string.h>

static Phase4Runtime_t *g_runtime = NULL;

static uint8_t Phase4_ReadButtonEvent(void)
{
    static uint32_t last_press_tick = 0;
    static uint8_t pressed = 0;

    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_RESET) {
        if (!pressed) {
            pressed = 1;
            last_press_tick = HAL_GetTick();
            return 1;
        }

        if ((HAL_GetTick() - last_press_tick) > 800U) {
            return 2;
        }
        return 0;
    }

    pressed = 0;
    return 0;
}

void Phase4_InitRuntime(Phase4Runtime_t *runtime)
{
    if (runtime == NULL) {
        return;
    }

    memset(runtime, 0, sizeof(*runtime));
    g_runtime = runtime;

    UI_InitData(&runtime->watch_data);
    Menu_Init(&runtime->menu_context);
    Pedometer_Init(&runtime->pedometer_state);

    runtime->sensor_queue = xQueueCreate(4, sizeof(SmartWatchData_t));
    runtime->time_queue = xQueueCreate(2, sizeof(Phase4TimeData_t));
    runtime->menu_queue = xQueueCreate(4, sizeof(Phase4MenuMsg_t));
    runtime->bt_queue = xQueueCreate(4, sizeof(SmartWatchData_t));
    runtime->display_sem = xSemaphoreCreateBinary();
    runtime->i2c_mutex = xSemaphoreCreateMutex();
}

void Phase4_StartTasks(Phase4Runtime_t *runtime)
{
    if (runtime == NULL) {
        return;
    }

    xTaskCreate(vTask_TimeKeep, "TimeKeep", 256, runtime, 4, &runtime->task_time_keep);
    xTaskCreate(vTask_Sensor, "Sensor", 512, runtime, 3, &runtime->task_sensor);
    xTaskCreate(vTask_Display, "Display", 1024, runtime, 2, &runtime->task_display);
    xTaskCreate(vTask_Menu, "Menu", 512, runtime, 1, &runtime->task_menu);
    xTaskCreate(vTask_Bluetooth, "Bluetooth", 512, runtime, 0, &runtime->task_bluetooth);
}

void vTask_TimeKeep(void *pvParameters)
{
    Phase4Runtime_t *runtime = (Phase4Runtime_t *)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        if (runtime != NULL) {
            runtime->watch_data.second++;
            if (runtime->watch_data.second >= 60U) {
                runtime->watch_data.second = 0U;
                runtime->watch_data.minute++;
            }
            if (runtime->watch_data.minute >= 60U) {
                runtime->watch_data.minute = 0U;
                runtime->watch_data.hour++;
            }
            if (runtime->watch_data.hour >= 24U) {
                runtime->watch_data.hour = 0U;
            }

            Phase4TimeData_t time_data;
            time_data.hour = runtime->watch_data.hour;
            time_data.minute = runtime->watch_data.minute;
            time_data.second = runtime->watch_data.second;
            time_data.year = runtime->watch_data.year;
            time_data.month = runtime->watch_data.month;
            time_data.day = runtime->watch_data.day;
            time_data.weekday = runtime->watch_data.weekday;

            if (runtime->time_queue != NULL) {
                xQueueOverwrite(runtime->time_queue, &time_data);
            }
        }

        xSemaphoreGive(runtime->display_sem);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}

void vTask_Sensor(void *pvParameters)
{
    Phase4Runtime_t *runtime = (Phase4Runtime_t *)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        if (runtime != NULL) {
            if (runtime->i2c_mutex != NULL && xSemaphoreTake(runtime->i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                if (runtime->watch_data.imu_status) {
                    (void)MPU6050_ReadAccel(&runtime->watch_data.accel);
                    (void)MPU6050_ReadGyro(&runtime->watch_data.gyro);
                    runtime->watch_data.temp_celsius = (int8_t)MPU6050_ReadTemp();
                    MPU6050_CalcAngle(&runtime->watch_data.accel, &runtime->watch_data.angle);
                }
                xSemaphoreGive(runtime->i2c_mutex);
            }

            Pedometer_Update(&runtime->pedometer_state, &runtime->watch_data.accel,
                             xTaskGetTickCount() * portTICK_PERIOD_MS);
            runtime->watch_data.step_count = Pedometer_GetStepCount(&runtime->pedometer_state);
            runtime->watch_data.distance_m = Pedometer_GetDistanceMeters(&runtime->pedometer_state);
            runtime->watch_data.calories = Pedometer_GetCalories(&runtime->pedometer_state);

            if (runtime->sensor_queue != NULL) {
                xQueueOverwrite(runtime->sensor_queue, &runtime->watch_data);
            }
            if (runtime->display_sem != NULL) {
                xSemaphoreGive(runtime->display_sem);
            }
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200));
    }
}

void vTask_Display(void *pvParameters)
{
    Phase4Runtime_t *runtime = (Phase4Runtime_t *)pvParameters;

    for (;;) {
        if (runtime != NULL && runtime->display_sem != NULL) {
            xSemaphoreTake(runtime->display_sem, pdMS_TO_TICKS(500));
        }

        if (runtime != NULL) {
            if (runtime->i2c_mutex != NULL && xSemaphoreTake(runtime->i2c_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                Menu_Render(&runtime->watch_data, &runtime->menu_context);
                xSemaphoreGive(runtime->i2c_mutex);
            }
        }
    }
}

void vTask_Menu(void *pvParameters)
{
    Phase4Runtime_t *runtime = (Phase4Runtime_t *)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        if (runtime != NULL) {
            int16_t encoder_delta = (int16_t)TIM3->CNT;
            TIM3->CNT = 0;
            uint8_t button_event = Phase4_ReadButtonEvent();

            if (encoder_delta != 0 || button_event != 0) {
                if (encoder_delta != 0) {
                    Menu_ProcessRotation(&runtime->menu_context, encoder_delta);
                }
                if (button_event != 0) {
                    Menu_ProcessButton(&runtime->menu_context, (button_event == 2U));
                }

                if (runtime->menu_queue != NULL) {
                    Phase4MenuMsg_t msg;
                    msg.delta = encoder_delta;
                    msg.button_event = button_event;
                    xQueueOverwrite(runtime->menu_queue, &msg);
                }
                if (runtime->display_sem != NULL) {
                    xSemaphoreGive(runtime->display_sem);
                }
            }
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}

void vTask_Bluetooth(void *pvParameters)
{
    Phase4Runtime_t *runtime = (Phase4Runtime_t *)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t count = 0;

    for (;;) {
        if (runtime != NULL) {
            (void)BT_Process(&runtime->watch_data);
            runtime->watch_data.bt_connected = BT_IsConnected();
            if (++count % 10U == 0U) {
                BT_SendSensorData(&runtime->watch_data);
            }
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}
