#include "stm32f4xx_hal.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "debug_log.h"
extern UART_HandleTypeDef huart2;

void debug_log(const char *fmt, ...) {
    char buf[128];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 3, fmt, args);  // Leave space for "\r\n\0"
    va_end(args);

    // Append "\r\n" if there's room
    size_t len = strlen(buf);
    if (len < sizeof(buf) - 2) {
        buf[len++] = '\r';
        buf[len++] = '\n';
        buf[len] = '\0';
    }

    HAL_UART_Transmit(&huart2, (uint8_t*)buf, len, HAL_MAX_DELAY);
}
