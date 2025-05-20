#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "functions/ssd1306_i2c.h"
#include "functions/buzzer.h"

#define ADC_PIN 28
#define SAMPLE_RATE_HZ 8000
#define MAX_LINE_LEN 128

#define LED_PIN 13     // Pino onde o LED está conectado
#define BUZZER_PIN 21

char input_line[MAX_LINE_LEN];
int input_pos = 0;

// Área de renderização do display
struct render_area frame_area = {
    start_col: 0,
    end_col : SSD1306_WIDTH - 1,
    start_page : 0,
    end_page : SSD1306_NUM_PAGES - 1
    };

// Buffer para o display
uint8_t buf[SSD1306_BUF_LEN];

bool audio_sample_callback(repeating_timer_t *t) {
    uint16_t raw = adc_read();
    uint8_t sample = raw >> 4; // 12 bits → 8 bits
    putchar_raw(sample);
    return true;
}

void process_received_line(char* line) {
    // Mostra no OLED
    memset(buf, 0, SSD1306_BUF_LEN);
    WriteString(buf, 0, 32, line);
    render(buf, &frame_area);

    // Aqui você pode comparar "line" e executar comandos:
    if (strstr(line, "acender") != NULL || strstr(line, "Acender" ) != NULL || strstr(line, "Acende") != NULL || strstr(line, "acende") != NULL) {
        gpio_put(LED_PIN, 1);
    } else if (strstr(line, "apagar") != NULL || strstr(line, "Apagar") != NULL || strstr(line, "Apaga") != NULL || strstr(line, "apaga") != NULL) {
        gpio_put(LED_PIN, 0);
    }
    else if (strstr(line, "piscar") != NULL || strstr(line, "Piscar") != NULL || strstr(line, "pisca") != NULL || strstr(line, "Pisca") != NULL) {
        for(int i = 0; i<5; i++){
            gpio_put(LED_PIN, 1);
            sleep_ms(300);
            gpio_put(LED_PIN, 0);
            sleep_ms(300);
        }
    }
    else if (strstr(line, "tocar") != NULL || strstr(line, "Tocar") != NULL || strstr(line, "Toca") != NULL || strstr(line, "Tocar") != NULL) {
        // Teste com beep simples
        beep(BUZZER_PIN, 2000);
    }
}

int main() {
    stdio_usb_init();
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(2); // GPIO28 = ADC2

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0); // Desliga o LED inicialmente

    pwm_init_buzzer(BUZZER_PIN); // inicializa o buzzer

    //configuração do display ssd1306
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    SSD1306_init();

    memset(buf, 0, SSD1306_BUF_LEN);
    calc_render_area_buflen(&frame_area);

    render(buf, &frame_area);

    // Inicia amostragem periódica
    repeating_timer_t timer;
    add_repeating_timer_us(-1000000 / SAMPLE_RATE_HZ, audio_sample_callback, NULL, &timer);

    while (true) {
        // Verifica se há dados para ler da USB
        int c = getchar_timeout_us(0);  // 0 = sem esperar
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\n' || c == '\r') {
                input_line[input_pos] = '\0';
                process_received_line(input_line);
                input_pos = 0;
            } else if (input_pos < MAX_LINE_LEN - 1) {
                input_line[input_pos++] = (char)c;
            }
        }
        tight_loop_contents(); // evita sleep
    }
}