#include "pedometer.h"
#include <math.h>

static const float Pedometer_StrideLengthMeters = 0.75f;
static const float Pedometer_CaloriePerStep = 0.04f;
static const float Pedometer_MinThreshold = 10.5f; /* ~1.07g */
static const uint32_t Pedometer_MinStepIntervalMs = 200;

void Pedometer_Init(PedometerState_t *state) {
    if (state == NULL) {
        return;
    }

    state->step_count = 0;
    state->last_step_ms = 0;
    state->filtered_acc = 9.81f;
    state->threshold = Pedometer_MinThreshold;
    state->max_acc = 9.81f;
    state->min_acc = 9.81f;
    state->sample_count = 0;
    state->peak_detected = false;
    state->distance_m = 0.0f;
    state->calories = 0.0f;
}

void Pedometer_Update(PedometerState_t *state, const MPU6050_Accel_t *accel, uint32_t tick_ms) {
    if (state == NULL || accel == NULL) {
        return;
    }

    float accel_mag = sqrtf(accel->ax * accel->ax + accel->ay * accel->ay + accel->az * accel->az);
    state->filtered_acc = state->filtered_acc * 0.9f + accel_mag * 0.1f;

    if (state->filtered_acc > state->max_acc) {
        state->max_acc = state->filtered_acc;
    }
    if (state->filtered_acc < state->min_acc) {
        state->min_acc = state->filtered_acc;
    }

    state->sample_count++;
    if (state->sample_count >= 50) {
        float dynamic = (state->max_acc + state->min_acc) * 0.5f;
        state->threshold = dynamic;
        if (state->threshold < Pedometer_MinThreshold) {
            state->threshold = Pedometer_MinThreshold;
        }
        state->max_acc = state->threshold + 0.5f;
        state->min_acc = state->threshold - 0.5f;
        state->sample_count = 0;
    }

    if (!state->peak_detected && state->filtered_acc > state->threshold) {
        if (tick_ms - state->last_step_ms >= Pedometer_MinStepIntervalMs) {
            state->step_count++;
            state->distance_m += Pedometer_StrideLengthMeters;
            state->calories += Pedometer_CaloriePerStep;
            state->last_step_ms = tick_ms;
            state->peak_detected = true;
        }
    }

    if (state->filtered_acc < state->threshold) {
        state->peak_detected = false;
    }
}

uint32_t Pedometer_GetStepCount(const PedometerState_t *state) {
    return (state == NULL) ? 0 : state->step_count;
}

float Pedometer_GetDistanceMeters(const PedometerState_t *state) {
    return (state == NULL) ? 0.0f : state->distance_m;
}

float Pedometer_GetCalories(const PedometerState_t *state) {
    return (state == NULL) ? 0.0f : state->calories;
}

void Pedometer_Reset(PedometerState_t *state) {
    if (state == NULL) {
        return;
    }
    state->step_count = 0;
    state->distance_m = 0.0f;
    state->calories = 0.0f;
    state->last_step_ms = 0;
    state->peak_detected = false;
    state->filtered_acc = 9.81f;
    state->threshold = Pedometer_MinThreshold;
    state->max_acc = 9.81f;
    state->min_acc = 9.81f;
    state->sample_count = 0;
}
