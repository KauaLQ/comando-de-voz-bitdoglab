#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

// Inicializa o PWM no pino especificado para o buzzer
void pwm_init_buzzer(uint pin);

// Emite um beep simples com duração especificada
void beep(uint pin, uint duration_ms);

#endif