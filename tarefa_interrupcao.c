#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "ws2812.pio.h"

// Definição dos pinos
#define LED_R 11
#define LED_G 12
#define LED_B 13
#define BUTTON_A 5
#define BUTTON_B 6
#define MATRIX_WS2812 7

// Variáveis globais
volatile int numero_atual = 0;
PIO pio = pio0;  // PIO utilizado
uint sm = 0;     // State machine do PIO


// Variáveis para debounce
volatile uint32_t ultimo_tempo_a = 0;
volatile uint32_t ultimo_tempo_b = 0;
const uint32_t TEMPO_DEBOUNCE = 200; // 200ms de debounce

// Variável para controlar a intensidade do brilho (0 a 255)
volatile uint8_t brilho = 55;  // Brilho máximo (255 é totalmente brilhante, 0 é apagado)

volatile uint32_t cor_acesa = 0x00FF00;   // 🟢 Verde
volatile uint32_t cor_apagada = 0xFF0000; // ⚫ Preto (apagado)

// Mapeamento de números 5x5 na matriz WS2812
const uint32_t numeros[10][25] = {
    {1, 1, 1, 1, 1,   1, 0, 0, 0, 1,   1, 0, 0, 0, 1,   1, 0, 0, 0, 1,   1, 1, 1, 1, 1}, // 0
    {0, 1, 1, 1, 0,   0, 0, 1, 0, 0,   0, 0, 1, 0, 0,   0, 1, 1, 0, 0,   0, 0, 1, 0, 0}, // 1
    {1, 1, 1, 1, 1,   1, 0, 0, 0, 0,   1, 1, 1, 1, 1,   0, 0, 0, 0, 1,   1, 1, 1, 1, 1}, // 2
    {1, 1, 1, 1, 1,   0, 0, 0, 0, 1,   1, 1, 1, 1, 1,   0, 0, 0, 0, 1,   1, 1, 1, 1, 1}, // 3
    {1, 0, 0, 0, 0,   0, 0, 0, 0, 1,   1, 1, 1, 1, 1,   1, 0, 0, 0, 1,   1, 0, 0, 0, 1}, // 4
    {1, 1, 1, 1, 1,   0, 0, 0, 0, 1,   1, 1, 1, 1, 1,   1, 0, 0, 0, 0,   1, 1, 1, 1, 1}, // 5
    {1, 1, 1, 1, 1,   1, 0, 0, 0, 1,   1, 1, 1, 1, 1,   1, 0, 0, 0, 0,   1, 1, 1, 1, 1}, // 6
    {1, 0, 0, 0, 0,   0, 0, 0, 0, 1,   1, 0, 0, 0, 0,   0, 0, 0, 0, 1,   1, 1, 1, 1, 1}, // 7
    {1, 1, 1, 1, 1,   1, 0, 0, 0, 1,   1, 1, 1, 1, 1,   1, 0, 0, 0, 1,   1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 1,   0, 0, 0, 0, 1,   1, 1, 1, 1, 1,   1, 0, 0, 0, 1,   1, 1, 1, 1, 1}  // 9
};

// Função para aplicar o brilho na cor
uint32_t aplicar_brilho(uint32_t cor) {
    // Extrair os componentes RGB
    uint8_t r = (cor >> 16) & 0xFF;
    uint8_t g = (cor >> 8) & 0xFF;
    uint8_t b = cor & 0xFF;

    // Ajustar o brilho (multiplicar cada componente pelo brilho/255)
    r = (r * brilho) / 255;
    g = (g * brilho) / 255;
    b = (b * brilho) / 255;

    // Combinar os componentes de volta em um único valor RGB
    return (r << 16) | (g << 8) | b;
}

// Função para enviar dados RGB para o WS2812
void ws2812_put(uint32_t color) {
    pio_sm_put_blocking(pio, sm, color);  // Envia a cor para o PIO
}

// Função para exibir o número na matriz WS2812
void exibir_numero(int numero) {
    uint32_t leds[25];

    for (int i = 0; i < 25; i++) {
        uint32_t cor = (numeros[numero][i] == 1) ? cor_acesa : cor_apagada;
        leds[i] = aplicar_brilho(cor);
    }

    for (int i = 0; i < 25; i++) {
        ws2812_put(leds[i]);
    }
}


void botoes_irq_handler(uint gpio, uint32_t events) {
    printf("⚡ Interrupção detectada no GPIO %u\n", gpio);
    
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A) {
        if (tempo_atual - ultimo_tempo_a < TEMPO_DEBOUNCE) return;  // Debounce individual
        ultimo_tempo_a = tempo_atual;
        numero_atual = (numero_atual + 1) % 10;
        printf("Botão A pressionado: Incrementando número para %d\n", numero_atual);
    } 
    else if (gpio == BUTTON_B) {
        if (tempo_atual - ultimo_tempo_b < TEMPO_DEBOUNCE) return;  // Debounce individual
        ultimo_tempo_b = tempo_atual;
        numero_atual = (numero_atual - 1 + 10) % 10;
        printf("Botão B pressionado: Decrementando número para %d\n", numero_atual);
    }

    exibir_numero(numero_atual);
}




int main() {
    stdio_init_all();

    // Inicializa a UART0 com 115200 bauds
    uart_init(uart0, 115200);

    // Configuração do LED RGB
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);

    // Configuração dos botões com pull-up
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);


    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &botoes_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &botoes_irq_handler);

    // Inicialização do PIO para controle da matriz WS2812
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, MATRIX_WS2812, 800000, false);

    // Exibe o primeiro número inicial
    exibir_numero(numero_atual);

    // Loop principal (pisca LED vermelho)
    while (1) {
        gpio_put(LED_R, 1);
        sleep_ms(100);
        gpio_put(LED_R, 0);
        sleep_ms(100);
    }
}
