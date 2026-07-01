#ifndef PEDOMETER_H
#define PEDOMETER_H

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
} PedometerState;

void Pedometer_Init(PedometerState *state);
void Pedometer_Update(PedometerState *state, const int16_t accel_raw[3], uint32_t tick_ms);
uint32_t Pedometer_GetStepCount(const PedometerState *state);
float Pedometer_GetDistanceMeters(const PedometerState *state);
float Pedometer_GetCalories(const PedometerState *state);
void Pedometer_Reset(PedometerState *state);

#endif