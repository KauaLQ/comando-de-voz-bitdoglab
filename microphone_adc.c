/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "pico/binary_info.h"
#include "functions/ssd1306_i2c.h"

#define ADC_NUM 2
#define ADC_PIN (26 + ADC_NUM)
#define ADC_VREF 3.3
#define ADC_RANGE (1 << 12)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

#define LED_PIN 13     // Pino onde o LED está conectado

volatile char action[200];

// Callback para verificar o valor da variável
void repeating_timer_callback(struct repeating_timer *t) {
    // Verifica se as palavras-chave estão presentes
    if (strstr(action, "ascender") != NULL && strstr(action, "led") != NULL) {
        gpio_put(LED_PIN, 1); // Liga o LED
        strcpy(action, ""); // Limpa a string action corretamente
    }
    else if (strstr(action, "apagar") != NULL && strstr(action, "led") != NULL) {
        gpio_put(LED_PIN, 0); // Liga o LED
        strcpy(action, ""); // Limpa a string action corretamente
    }
    else if (strstr(action, "tocar") != NULL && strstr(action, "alarme") != NULL) {
        //tocarAlarme(); // Referente ao buzzer
        strcpy(action, ""); // Limpa a string action corretamente
    }
    
}

int main() {
    stdio_init_all();

    sleep_ms(5000); // Espera 5 segundos para o host reconhecer a USB

    printf("Bip boop, ouvindo...\n");

    bi_decl(bi_program_description("Microfone Analógico com Raspberry Pi Pico"));
    bi_decl(bi_1pin_with_name(ADC_PIN, "ADC pino de Entrada"));

    adc_init();
    adc_gpio_init( ADC_PIN);
    adc_select_input( ADC_NUM);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0); // Desliga o LED inicialmente

    uint adc_raw;

    // Configura o temporizador repetitivo para verificar o estado do botão a cada 100 ms.
    struct repeating_timer timer;
    add_repeating_timer_ms(100, repeating_timer_callback, NULL, &timer);

    //configuração do display ssd1306
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    SSD1306_init();

    // Área de renderização do display
    struct render_area frame_area = {
        start_col: 0,
        end_col : SSD1306_WIDTH - 1,
        start_page : 0,
        end_page : SSD1306_NUM_PAGES - 1
        };

    calc_render_area_buflen(&frame_area);

    // Buffer para o display
    uint8_t buf[SSD1306_BUF_LEN];
    memset(buf, 0, SSD1306_BUF_LEN);

    WriteString(buf, 35, 32, "OUVINDO");

    render(buf, &frame_area);

    while (1) {
        adc_raw = adc_read(); // tensão bruta do ADC
        printf("%.2f\n", adc_raw * ADC_CONVERT);
        sleep_ms(10);
    }
}