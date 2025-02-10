#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "ws2818b.pio.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Definição dos pinos e variáveis globais
#define LED_COUNT 25      // Número de LEDs na matriz
#define LED_PIN 7         // GPIO do LED WS2812
#define LED_GREEN 11      // GPIO do canal verde do LED RGB
#define LED_BLUE 12       // GPIO do canal azul do LED RGB
#define LED_RED 13        // GPIO do canal vermelho do LED RGB
#define BUTTON_A 5        // GPIO do botão A
#define BUTTON_B 6        // GPIO do botão B
#define I2C_PORT i2c1     // Porta I2C utilizada para comunicação
#define I2C_SDA 14        // GPIO para a linha de dados SDA do I2C
#define I2C_SCL 15        // GPIO para a linha de clock SCL do I2C
#define endereco 0x3C     // Endereço I2C do OLED

volatile int numero = 0;
volatile bool led_verde_estado = false;
volatile bool led_azul_estado = false;
ssd1306_t ssd; // Inicializa a estrutura do display

// Estrutura para armazenar a cor de um pixel (LED da matriz WS2812)
struct pixel_t { uint32_t G, R, B; };
typedef struct pixel_t pixel_t;
pixel_t leds[LED_COUNT];
PIO np_pio;
uint sm;

// Inicializa o controlador PIO para os LEDs WS2812
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
}

// Define a cor de um LED específico na matriz
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

// Apaga todos os LEDs da matriz
void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i) npSetLED(i, 0, 0, 0);
}

// Atualiza a matriz de LEDs enviando os dados ao hardware
void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G << 24);
        pio_sm_put_blocking(np_pio, sm, leds[i].R << 24);
        pio_sm_put_blocking(np_pio, sm, leds[i].B << 24);
    }
}

// Função que mapeia os LEDs em zigue-zague, de acordo com a BitDogLab
int zigzag_map(int row, int col) {
    if (row % 2 == 0) {
        return (row * 5) + col;  // Linhas pares (esquerda para direita)
    } else {
        return (row * 5) + (4 - col);  // Linhas ímpares (direita para esquerda)
    }
}

// Exibe um número de 0 a 9 na matriz de LEDs
void exibir_numero(int num) {
    npClear();

    const uint8_t numeros[10][5][5] = {  
        { // Número 0
            {0,1,1,1,0},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,0,0,0,1},
            {0,1,1,1,0}
        },
        { // Número 1
            {0,0,1,0,0},
            {0,1,1,0,0},
            {1,0,1,0,0},
            {0,0,1,0,0},
            {1,1,1,1,1}
        },
        { // Número 2
            {1,1,1,1,1},
            {0,0,0,0,1},
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,1,1,1,1}
        },
        { // Número 3
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,1,1,1,1},
            {0,0,0,0,1},
            {1,1,1,1,1}
        },
        { // Número 4
            {1,0,0,0,1},
            {1,0,0,0,1},
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,0,0,1}
        },
        { // Número 5
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,1,1,1,1},
            {0,0,0,0,1},
            {1,1,1,1,1}
        },
        { // Número 6
            {1,1,1,1,1},
            {1,0,0,0,0},
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,1,1,1,1}
        },
        { // Número 7
            {1,1,1,1,1},
            {0,0,0,0,1},
            {0,0,1,1,0},
            {0,0,1,0,0},
            {0,0,1,0,0}
        },
        { // Número 8
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,1,1,1,1}
        },
        { // Número 9
            {1,1,1,1,1},
            {1,0,0,0,1},
            {1,1,1,1,1},
            {0,0,0,0,1},
            {1,1,1,1,1}
        }
    };

    for (int row = 4; row >= 0; row--) {  
        for (int col = 4; col >= 0; col--) {  
            int led_index = zigzag_map(4 - row, 4 - col);  
            if (numeros[num][row][col]) {
                npSetLED(led_index, 0, 0, 255);  
            }
        }
    }

    npWrite();
}

// Função de debounce para os botões
bool debounce(uint gpio) {
    static uint32_t ultimo_tempo_A = 0; 
    static uint32_t ultimo_tempo_B = 0;
    
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time()); 

    if (gpio == BUTTON_A) {
        if (tempo_atual - ultimo_tempo_A > 200) { // Verifica se passaram mais de 200 ms desde a última ativação
            ultimo_tempo_A = tempo_atual; 
            return true;
        }
    } else if (gpio == BUTTON_B) {
        if (tempo_atual - ultimo_tempo_B > 200) {
            ultimo_tempo_B = tempo_atual;
            return true;
        }
    }
    return false;
}

// Interrupções dos botões A e B para ativar ou desativas os LED verde e azul
void isr_botoes(uint gpio, uint32_t events) { 
    if (debounce(gpio)) {
        if (gpio == BUTTON_A) {
            led_verde_estado = !led_verde_estado;
            gpio_put(LED_GREEN, led_verde_estado);
            if (led_verde_estado){
                printf("LED Verde: Ligado\n"); 
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "G ON", 5, 30); // Desenha uma string
                ssd1306_send_data(&ssd);
            } else{
                printf("LED Verde: Desligado\n"); 
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "G OFF", 5, 30); // Desenha uma string
                ssd1306_send_data(&ssd);
            }

        } else if (gpio == BUTTON_B) {
            led_azul_estado = !led_azul_estado;
            gpio_put(LED_BLUE, led_azul_estado);
            if (led_azul_estado){
                printf("LED Azul: Ligado\n"); 
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "B ON", 5, 30); // Desenha uma string
                ssd1306_send_data(&ssd);
            } else{
                printf("LED Azul: Desligado\n"); 
                ssd1306_fill(&ssd, false);
                ssd1306_draw_string(&ssd, "B OFF", 5, 30); // Desenha uma string
                ssd1306_send_data(&ssd);
            }
        }
    }
}

// Configuração inicial do sistema
void setup() {
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_draw_string(&ssd, "#Ola", 20, 30); // Desenha mensagem
    ssd1306_send_data(&ssd); // Envia os dados para o display
    
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &isr_botoes);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true); 

    npInit(LED_PIN);
    npClear();
    exibir_numero(numero); // inicia o matriz LED com 0
    npWrite();
}

int main() {
    setup();
    
    while (true) {
        if (stdio_usb_connected()) { // Certifica-se de que o USB está conectado
            int c = getchar_timeout_us(1000000);


            if (c != PICO_ERROR_TIMEOUT) {  // Só processa se um caractere foi recebido
                char str[2];
                str[0] = c;  // Transforma c em string
                str[1] = '\0';
            
                if (c >= '0' && c <= '9') {
                    numero = c - '0';
                    printf("Número recebido: %d\n", numero);
                    ssd1306_fill(&ssd, false);
                    ssd1306_draw_string(&ssd, str, 50, 20);
                    ssd1306_send_data(&ssd);
                    exibir_numero(numero);
                } else {
                    printf("Caractere recebido: %c\n", c);
                    ssd1306_fill(&ssd, false);
                    ssd1306_draw_string(&ssd, str, 50, 20);
                    ssd1306_send_data(&ssd);
                }
            }
        }
    }
}