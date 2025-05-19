#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

// Handle fatal errors like RNG failure.
// May log, halt, blink LED, or enter safe state.
void handle_rng_failure(void);

#endif // ERROR_HANDLER_H
