// Bibliotecas
#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/timer.h"

// Macros
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define NUM_NUMBERS 11
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define IS_RGBW 0
#define BUTTON_A 5
#define BUTTON_B 6
#define BLUE_LED 12
#define GREEN_LED 11

// Variáveis globais
static volatile uint32_t LAST_TIME_A = 0;
static volatile uint32_t LAST_TIME_B = 0;
static volatile bool button_a_pressed = false;
static volatile bool button_b_pressed = false;
int current_number = 0;
int count_a = 0;
int count_b = 0;

// Função para inicializar os leds verde e azul
void init_leds() {
    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);

    gpio_init(BLUE_LED);
    gpio_set_dir(BLUE_LED, GPIO_OUT);
} 

// Função para os leds da matriz
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// Função para os leds da matriz
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Matriz de leds
const bool matriz_led[NUM_NUMBERS][NUM_PIXELS] = {
    {0,1,1,1,0, 0,1,0,1,0, 0,1,0,1,0, 0,1,0,1,0, 0,1,1,1,0}, // 0
    {0,1,1,1,0, 0,0,1,0,0, 0,0,1,0,0, 0,1,1,0,0, 0,0,1,0,0}, // 1
    {0,1,1,1,0, 0,1,0,0,0, 0,1,1,1,0, 0,0,0,1,0, 0,1,1,1,0}, // 2
    {0,1,1,1,0, 0,0,0,1,0, 0,1,1,1,0, 0,0,0,1,0, 0,1,1,1,0}, // 3
    {0,1,0,0,0, 0,0,0,1,0, 0,1,1,1,0, 0,1,0,1,0, 0,1,0,1,0}, // 4
    {0,1,1,1,0, 0,0,0,1,0, 0,1,1,1,0, 0,1,0,0,0, 0,1,1,1,0}, // 5
    {0,1,1,1,0, 0,1,0,1,0, 0,1,1,1,0, 0,1,0,0,0, 0,1,1,1,0}, // 6
    {0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 0,0,0,1,0, 0,1,1,1,0}, // 7
    {0,1,1,1,0, 0,1,0,1,0, 0,1,1,1,0, 0,1,0,1,0, 0,1,1,1,0}, // 8
    {0,1,1,1,0, 0,0,0,1,0, 0,1,1,1,0, 0,1,0,1,0, 0,1,1,1,0}, // 9
    {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0}  // Matriz vazia
};

// Função para desenhar um numero na matriz
void show_number_leds(int number) {
    int i;

    for (i = 0; i < NUM_PIXELS; i++) {
        put_pixel(matriz_led[number][i] ? urgb_u32(0, 0, 200) : 0);
    }
    sleep_ms(10);
} 

// Função para trabalhar a interrupção dos botões A e B
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == BUTTON_A && current_time - LAST_TIME_A > 200000) {
        LAST_TIME_A = current_time;
        button_a_pressed = true;
        count_a++;
    } else if(gpio == BUTTON_B && current_time - LAST_TIME_B > 200000) {
        LAST_TIME_B = current_time;
        button_b_pressed = true;
        count_b++;
    }
} 

// Função principal
int main() {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW); // Inicializa a maquina de estados
    stdio_init_all(); // Inicializa todas as interfaces de entrada e saída padrão
    i2c_init(I2C_PORT, 400 * 1000); // I2C Initialisation. Using it at 400Khz.
    init_leds(); // Inicializa os leds verde e azul

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Configura o botão A
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Interrupção para o botão A

    // Configura o botão B
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Interrupção para o botão B

    bool color = true;

    show_number_leds(NUM_NUMBERS - 1); // Apaga a matriz de leds

    while (true) {
        if(stdio_usb_connected()) {
            char letter;
            int c = getchar_timeout_us(1000);

            if(c != PICO_ERROR_TIMEOUT) { // Pega a entrada do monitor serial
                letter = (char)c;

                if(!(letter >= '0' && letter <= '9') && !(letter >= 'A' && letter <= 'Z') && !(letter >= 'a' && letter <= 'z')) {
                    continue;
                }

                printf("Caractere recebido: %c\n", letter);

                color = !color;

                // Verifica se a entrada foi um número entre 0 e 9 e mostra o número na matriz de leds
                if(letter >= '0' && letter <= '9') {
                    printf("%d\n", letter - '0');
                    show_number_leds(letter - '0');
                } else {
                    show_number_leds(NUM_NUMBERS - 1); // Apaga a matriz
                }

                // Imprime a entrada do monitor serial no display
                ssd1306_fill(&ssd, !color);
                ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color);
                ssd1306_draw_char(&ssd, letter, 50, 30);
                ssd1306_send_data(&ssd);

                sleep_ms(1000);
            } else if(button_a_pressed) { // Verifica se o botão A foi pressionado
                button_a_pressed = false;
                color = !color;

                show_number_leds(NUM_NUMBERS - 1); // Apaga a matriz

                if(count_a % 2 != 0) { // Acende o led verde caso o botão A tiver sido pressionado
                    ssd1306_fill(&ssd, !color);
                    ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color);
                    ssd1306_draw_string(&ssd, "Led verde", 30, 20);
                    ssd1306_draw_string(&ssd, "aceso", 30, 30);
                    ssd1306_send_data(&ssd);
                    gpio_put(GREEN_LED, true);
                    printf("Led verde aceso.\n");

                } else { // Desliga o led
                    ssd1306_fill(&ssd, !color);
                    ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color);
                    ssd1306_draw_string(&ssd, "Led verde", 30, 20);
                    ssd1306_draw_string(&ssd, "apagado", 30, 30);
                    ssd1306_send_data(&ssd);
                    gpio_put(GREEN_LED, false);
                    printf("Led verde apagado.\n");
                }

                sleep_ms(1000);
            } else if(button_b_pressed) { // Verifica se o botão B foi pressionado
                button_b_pressed = false;
                color = !color;

                show_number_leds(NUM_NUMBERS - 1); // Apaga a matriz de leds

                if(count_b % 2 != 0) { // Acende o led azul caso o botão B tiver sido pressionado
                    ssd1306_fill(&ssd, !color);
                    ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color);
                    ssd1306_draw_string(&ssd, "Led azul", 30, 20);
                    ssd1306_draw_string(&ssd, "aceso", 30, 30);
                    ssd1306_send_data(&ssd);
                    gpio_put(BLUE_LED, true);
                    printf("Led azul aceso.\n");

                } else { // Desliga o led
                    ssd1306_fill(&ssd, !color);
                    ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color);
                    ssd1306_draw_string(&ssd, "Led azul", 30, 20);
                    ssd1306_draw_string(&ssd, "apagado", 30, 30);
                    ssd1306_send_data(&ssd);
                    gpio_put(BLUE_LED, false);
                    printf("Led azul apagado.\n");
                }

                sleep_ms(1000);
            }
        }

        sleep_ms(10);
    }

    return 0;
}
