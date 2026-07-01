#include "pedometer.h"
#include <math.h>

static const float PEDOMETER_STRIDE_METERS    = 0.75f;
static const float PEDOMETER_CALORIE_PER_STEP  = 0.04f;
static const float PEDOMETER_MIN_THRESHOLD_G   = 1.1f;
static const uint32_t PEDOMETER_MIN_STEP_INTERVAL_MS = 200;

#define MPU6050_LSB_PER_G   16384.0f
#define GRAVITY_M_S2        9.81f

static float raw_to_mps2(int16_t raw) {
    return ((float)raw / MPU6050_LSB_PER_G) * GRAVITY_M_S2;
}

void Pedometer_Init(PedometerState *state) {
    if (state == NULL) return;
    state->step_count = 0;
    state->last_step_ms = 0;
    state->filtered_acc = GRAVITY_M_S2;
    state->threshold = PEDOMETER_MIN_THRESHOLD_G * GRAVITY_M_S2;
    state->max_acc = GRAVITY_M_S2;
    state->min_acc = GRAVITY_M_S2;
    state->sample_count = 0;
    state->peak_detected = false;
    state->distance_m = 0.0f;
    state->calories = 0.0f;
}

void Pedometer_Update(PedometerState *state, const int16_t accel_raw[3], uint32_t tick_ms) {
    if (state == NULL || accel_raw == NULL) return;

    float ax = raw_to_mps2(accel_raw[0]);
    float ay = raw_to_mps2(accel_raw[1]);
    float az = raw_to_mps2(accel_raw[2]);
    float accel_mag = sqrtf(ax * ax + ay * ay + az * az);

    state->filtered_acc = state->filtered_acc * 0.9f + accel_mag * 0.1f;

    if (state->filtered_acc > state->max_acc) state->max_acc = state->filtered_acc;
    if (state->filtered_acc < state->min_acc) state->min_acc = state->filtered_acc;

    state->sample_count++;
    if (state->sample_count >= 50) {
        float dynamic_threshold = (state->max_acc + state->min_acc) * 0.5f;
        state->threshold = dynamic_threshold;
        if (state->threshold < PEDOMETER_MIN_THRESHOLD_G * GRAVITY_M_S2) {
            state->threshold = PEDOMETER_MIN_THRESHOLD_G * GRAVITY_M_S2;
        }
        state->max_acc = state->threshold + 0.5f;
        state->min_acc = state->threshold - 0.5f;
        state->sample_count = 0;
    }

    if (!state->peak_detected && state->filtered_acc > state->threshold) {
        if (tick_ms - state->last_step_ms >= PEDOMETER_MIN_STEP_INTERVAL_MS) {
            state->step_count++;
            state->distance_m += PEDOMETER_STRIDE_METERS;
            state->calories += PEDOMETER_CALORIE_PER_STEP;
            state->last_step_ms = tick_ms;
            state->peak_detected = true;
        }
    }

    if (state->filtered_acc < state->threshold) {
        state->peak_detected = false;
    }
}

uint32_t Pedometer_GetStepCount(const PedometerState *state) {
    return (state == NULL) ? 0 : state->step_count;
}

float Pedometer_GetDistanceMeters(const PedometerState *state) {
    return (state == NULL) ? 0.0f : state->distance_m;
}

float Pedometer_GetCalories(const PedometerState *state) {
    return (state == NULL) ? 0.0f : state->calories;
}

void Pedometer_Reset(PedometerState *state) {
    if (state == NULL) return;
    state->step_count = 0;
    state->distance_m = 0.0f;
    state->calories = 0.0f;
    state->last_step_ms = 0;
    state->peak_detected = false;
    state->filtered_acc = GRAVITY_M_S2;
    state->threshold = PEDOMETER_MIN_THRESHOLD_G * GRAVITY_M_S2;
    state->max_acc = GRAVITY_M_S2;
    state->min_acc = GRAVITY_M_S2;
    state->sample_count = 0;
}