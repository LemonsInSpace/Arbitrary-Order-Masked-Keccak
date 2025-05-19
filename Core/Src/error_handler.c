#include "error_handler.h"
#include "stm32f4xx_hal.h"
#include "debug_log.h"
#include "main.h"  // Required for LD5_Pin definition

void handle_rng_failure(void) {
    debug_log("ERROR: RNG failure detected — halting.");

    __disable_irq();  // Disable interrupts to prevent further execution

    while (1) {
        HAL_GPIO_TogglePin(GPIOD, LD5_Pin);  // LD5 = error indicator (typically red)
        HAL_Delay(200);  // 200ms blink
    }
}
