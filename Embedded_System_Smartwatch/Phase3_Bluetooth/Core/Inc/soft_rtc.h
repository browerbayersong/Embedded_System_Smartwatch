#ifndef SOFT_RTC_H
#define SOFT_RTC_H
#include <stdint.h>

typedef struct {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} RTC_Time;

extern RTC_Time rtc_time;
#endif