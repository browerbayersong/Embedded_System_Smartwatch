#ifndef __PEDOMETER_H
#define __PEDOMETER_H

#include "mpu6050.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t step_count;
    uint32_t last_step_ms;
    float filtered_acc;
    float threshold;
    float max_acc;
    float min_acc;
    uint8_t sample_count;
    bool peak_detected;
    float distance_m;
    float calories;
} PedometerState_t;

void Pedometer_Init(PedometerState_t *state);
void Pedometer_Update(PedometerState_t *state, const MPU6050_Accel_t *accel, uint32_t tick_ms);
uint32_t Pedometer_GetStepCount(const PedometerState_t *state);
float Pedometer_GetDistanceMeters(const PedometerState_t *state);
float Pedometer_GetCalories(const PedometerState_t *state);
void Pedometer_Reset(PedometerState_t *state);

#endif /* __PEDOMETER_H */
