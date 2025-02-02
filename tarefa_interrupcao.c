#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include <stdlib.h> 

// Definição dos pinos
#define LED_R 13
#define LED_G 11
#define LED_B 12
#define BUTTON_A 5
#define BUTTON_B 6
#define MATRIX_WS2812 7
#define JOYSTICK_BUTTON 22  

// Variáveis globais
PIO pio = pio0;  
uint sm = 0;     

// Variáveis para debounce
volatile uint32_t ultimo_tempo_a = 0;
volatile uint32_t ultimo_tempo_b = 0;
const uint32_t TEMPO_DEBOUNCE = 200; // 200ms de debounce
volatile uint32_t ultimo_tempo_joystick = 0;
volatile int numero_atual = 0;


// Variável para controlar a cor inicial e  a intensidade do brilho (0 a 255)
uint8_t intensidade_acesa = 70;    
uint8_t intensidade_apagada = 10;
volatile uint32_t cor_acesa = 0x00FF00;// Azul
volatile uint32_t cor_apagada = 0xDDA0DD; //Lilás

// Função para piscar o LED vermelho 5 vezes por segundo
bool piscar_led_repetidamente(struct repeating_timer *t) {
    static bool estado = false;
    gpio_put(LED_R, estado);
    estado = !estado;
    return true; // Manter o timer ativo
}


// Mapeamento de números 5x5 na matriz WS2812
const uint32_t numeros[10][25] = {
    {0, 1, 1, 1, 0,   1, 0, 0, 0, 1,   1, 0, 0, 0, 1,   1, 0, 0, 0, 1,   0, 1, 1, 1, 0}, // 0
    {0, 1, 1, 1, 0,   0, 0, 1, 0, 0,   0, 0, 1, 0, 0,   0, 1, 1, 0, 0,   0, 0, 1, 0, 0}, // 1
    {1, 1, 1, 1, 1,   0, 0, 1, 0, 0,   0, 1, 0, 0, 0,   1, 0, 0, 0, 1,   0, 1, 1, 1, 0}, // 2
    {1, 1, 1, 1, 1,   0, 0, 0, 0, 1,   0, 1, 1, 1, 0,   0, 0, 0, 0, 1,   1, 1, 1, 1, 1}, // 3
    {0, 1, 0, 0, 0,   1, 1, 1, 1, 1,   0, 1, 0, 1, 0,   0, 0, 1, 1, 0,   0, 1, 0, 0, 0}, // 4
    {0, 1, 1, 1, 1,   0, 0, 0, 0, 1,   0, 1, 1, 1, 1,   1, 0, 0, 0, 0,   1, 1, 1, 1, 1}, // 5
    {0, 1, 1, 1, 0,   1, 0, 0, 0, 1,   0, 1, 1, 1, 1,   1, 0, 0, 0, 0,   0, 1, 1, 1, 0}, // 6
    {0, 0, 0, 1, 0,   0, 0, 1, 0, 0,   0, 1, 0, 0, 0,   0, 0, 0, 0, 1,   1, 1, 1, 1, 1}, // 7
    {0, 1, 1, 1, 0,   1, 0, 0, 0, 1,   0, 1, 1, 1, 0,   1, 0, 0, 0, 1,   0, 1, 1, 1, 0}, // 8
    {0, 1, 1, 1, 0,   0, 0, 0, 0, 1,   1, 1, 1, 1, 0,   1, 0, 0, 0, 1,   0, 1, 1, 1, 0}  // 9
};

// Função para ajustar o brilho de uma cor com intensidade global
uint32_t aplicar_brilho(uint32_t cor, uint8_t intensidade_acesa, uint8_t intensidade_apagada) {
    // Extrair os componentes RGB da cor
    uint8_t r = (cor >> 16) & 0xFF;
    uint8_t g = (cor >> 8) & 0xFF;
    uint8_t b = cor & 0xFF;

    // Ajustar o brilho dependendo da cor (acesa ou apagada)
    if (cor == cor_acesa) {
        r = (r * intensidade_acesa) / 255;
        g = (g * intensidade_acesa) / 255;
        b = (b * intensidade_acesa) / 255;
    } else {
        r = (r * intensidade_apagada) / 255;
        g = (g * intensidade_apagada) / 255;
        b = (b * intensidade_apagada) / 255;
    }

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

    // Exibir o número na matriz com as cores atuais
    for (int i = 0; i < 25; i++) {
        uint32_t cor = (numeros[numero][i] == 1) ? cor_acesa : cor_apagada;
        leds[i] = aplicar_brilho(cor, intensidade_acesa, intensidade_apagada);
    }

    // Enviar os dados para os LEDs
    for (int i = 0; i < 25; i++) {
        ws2812_put(leds[i]);
    }
}

// Função para calcular a distância (diferenças) entre duas cores RGB
int calcular_distancia_rgb(uint32_t cor1, uint32_t cor2) {
    // Extrai os componentes RGB de cada cor
    uint8_t r1 = (cor1 >> 16) & 0xFF;
    uint8_t g1 = (cor1 >> 8) & 0xFF;
    uint8_t b1 = cor1 & 0xFF;

    uint8_t r2 = (cor2 >> 16) & 0xFF;
    uint8_t g2 = (cor2 >> 8) & 0xFF;
    uint8_t b2 = cor2 & 0xFF;

    // Calcula a distância Euclidiana no espaço RGB
    int dr = r1 - r2;
    int dg = g1 - g2;
    int db = b1 - b2;
    return dr * dr + dg * dg + db * db;  // Retorna a soma dos quadrados das diferenças
}

void botoes_irq_handler(uint gpio, uint32_t events) {

    printf(" Interrupção detectada no GPIO %u\n", gpio);
    
    // Obtém o tempo atual (em milissegundos) desde o boot
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    // 1. Interrupção do Botão A (Incrementar Número)
    if (gpio == BUTTON_A) {
        //Verifica o tempo de debounce para evitar múltiplos acionamentos rápidos
        if (tempo_atual - ultimo_tempo_a < TEMPO_DEBOUNCE) return;
        ultimo_tempo_a = tempo_atual;

        // Incrementa o número atual e mantém dentro do intervalo de 0 a 9
        numero_atual = (numero_atual + 1) % 10;
        printf("Botão A pressionado: Incrementando número para %d\n", numero_atual);

        // Atualiza a exibição do número na matriz de LEDs
        exibir_numero(numero_atual);
    } 
    
    // 2. Interrupção do Botão B (Decrementar Número)
    else if (gpio == BUTTON_B) {
        // Verifica o tempo de debounce para evitar múltiplos acionamentos rápidos
        if (tempo_atual - ultimo_tempo_b < TEMPO_DEBOUNCE) return;
        ultimo_tempo_b = tempo_atual;

        // Decrementa o número atual e mantém dentro do intervalo de 0 a 9
        numero_atual = (numero_atual - 1 + 10) % 10;
        printf("Botão B pressionado: Decrementando número para %d\n", numero_atual);

        // Atualiza a exibição do número na matriz de LEDs
        exibir_numero(numero_atual);
    }
    
    // Interrupção do Joystick (Alterar Cores Aleatórias)
    else if (gpio == JOYSTICK_BUTTON) { 
        // Verifica o tempo de debounce para evitar múltiplos acionamentos rápidos
        if (tempo_atual - ultimo_tempo_joystick < TEMPO_DEBOUNCE) return;
        ultimo_tempo_joystick = tempo_atual;

        uint32_t nova_cor_acesa, nova_cor_apagada;
        int distancia_minima = 15000; // Distância mínima desejada entre as cores (posso ajustar)

        do {
            // Gera uma cor aleatória para a cor acesa
            uint8_t r_acesa = rand() % 256;
            uint8_t g_acesa = rand() % 256;
            uint8_t b_acesa = rand() % 256;
            nova_cor_acesa = (r_acesa << 16) | (g_acesa << 8) | b_acesa;

            // Gera uma cor aleatória para a cor apagada
            uint8_t r_apagada = rand() % 256;
            uint8_t g_apagada = rand() % 256;
            uint8_t b_apagada = rand() % 256;
            nova_cor_apagada = (r_apagada << 16) | (g_apagada << 8) | b_apagada;

            // Verifica a distância entre as cores acesa e apagada
        } while (calcular_distancia_rgb(nova_cor_acesa, nova_cor_apagada) < distancia_minima); // Garante que a distância seja maior que a mínima

        // Atribui as novas cores geradas
        cor_acesa = nova_cor_acesa;
        cor_apagada = nova_cor_apagada;

        // Imprime as novas cores geradas
        printf("🎮 Joystick pressionado: Cores aleatórias alteradas! (Cor acesa: #%06X, Cor apagada: #%06X)\n", cor_acesa, cor_apagada);

        // Atualiza a exibição do número na matriz de LEDs com as novas cores
        exibir_numero(numero_atual);  // Exibe o número com as novas cores
    }
}


int main() {
    stdio_init_all();


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

// Configuração do botão do joystick com pull-up
    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &botoes_irq_handler);

    // Inicialização do PIO para controle da matriz WS2812
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, MATRIX_WS2812, 800000, false);

    // Exibe o primeiro número inicial
    exibir_numero(numero_atual);

 // Configuração do timer para piscar o LED vermelho
    struct repeating_timer timer;
    add_repeating_timer_ms(-100, piscar_led_repetidamente, NULL, &timer);

    // Loop principal (pisca LED vermelho)
    while (1) {
        tight_loop_contents();

    }
}
