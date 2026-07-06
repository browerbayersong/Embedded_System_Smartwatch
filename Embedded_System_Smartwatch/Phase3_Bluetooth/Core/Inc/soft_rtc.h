#ifndef SOFT_RTC_H
#define SOFT_RTC_H
#include <stdint.h>

typedef struct {
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  weekday;  /* 0=Sun ... 6=Sat */
} RTC_Time;

extern RTC_Time rtc_time;
#endif