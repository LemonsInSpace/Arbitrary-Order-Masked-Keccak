#include "error_handler.h"
#include "stm32f4xx_hal.h"
#include "debug_log.h"
#include "main.h"  // for LD5_Pin

#define LED_PORT GPIOD
#define LED_PIN  LD5_Pin

static void blink_dot(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    HAL_Delay(200);
}

static void blink_dash(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    HAL_Delay(600);
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    HAL_Delay(200);
}

void handle_rng_failure(void) {
    debug_log("ERROR: RNG failure detected — halting.");

    __disable_irq();  // Stop all interrupts

    while (1) {
        // S: ...
        blink_dot(); blink_dot(); blink_dot();

        HAL_Delay(600);  // Gap between letters

        // O: ---
        blink_dash(); blink_dash(); blink_dash();

        HAL_Delay(600);  // Gap between letters

        // S: ...
        blink_dot(); blink_dot(); blink_dot();

        HAL_Delay(1400);  // Gap between words (SOS loop)
    }
}
