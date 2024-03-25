#include "smtc_hal_rtc.h"
#include <stdio.h>
#include <time.h> 

void main() {
    struct timespec remaining, request = { 5, 100 };

    uint32_t a = hal_rtc_get_time_ms();
    printf("Working.");
    nanosleep(&request, &remaining);
    uint32_t b = hal_rtc_get_time_ms();

    printf("start: %d, end: %d\n",a,b);
}