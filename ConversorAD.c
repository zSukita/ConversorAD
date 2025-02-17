#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "lib/ssd1306.h"
#include "pico/bootrom.h"

// Definições de hardware
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define DISPLAY_ADDR 0x3C

#define JOYSTICK_X 27  // Pino ADC para o eixo X do joystick
#define JOYSTICK_Y 26  // Pino ADC para o eixo Y do joystick
#define JOYSTICK_BTN 22 // Pino do botão do joystick
#define BUTTON_A 5     // Pino do botão A
#define BUTTON_B 6     // Pino do botão B (para entrar em modo BOOTSEL)

#define LED_GREEN 11   // Pino do LED Verde
#define LED_BLUE 12    // Pino do LED Azul
#define LED_RED 13     // Pino do LED Vermelho

#define WIDTH 128      // Largura do display OLED
#define HEIGHT 64      // Altura do display OLED
#define JOYSTICK_CENTER_X 1929  // Valor central do joystick no eixo X
#define JOYSTICK_CENTER_Y 2019  // Valor central do joystick no eixo Y
#define DEADZONE 100   // Zona morta para evitar tremores no joystick

// Variáveis globais
ssd1306_t ssd;
bool pwm_enabled = true;
int border_style = 1;
bool green_led_state = false;

// Função para configurar o PWM em um pino específico
void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice, 4095); // Configura o limite do PWM para 4095
    pwm_set_enabled(slice, true);
    pwm_set_gpio_level(pin, 0); // Inicializa o PWM com 0 (sem brilho)
}

// Função para tratar o debounce via interrupção
void debounce_callback(uint gpio, uint32_t events) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // Verifica se o tempo desde a última interrupção é maior que 50ms (debounce)
    if (current_time - last_time > 50) {
        if (gpio == JOYSTICK_BTN) {
            green_led_state = !green_led_state;
            pwm_set_gpio_level(LED_GREEN, green_led_state ? 2047 : 0);
            border_style = (border_style == 1) ? 2 : 1;
            printf("[BOTÃO] Bordas: %d\n", border_style);
        } else if (gpio == BUTTON_A) {
            pwm_enabled = !pwm_enabled;
            printf("[PWM] Estado: %s\n", pwm_enabled ? "Ativado" : "Desativado");
        }
        last_time = current_time;
    }
}

// Função para configurar as interrupções dos botões
void setup_button_irq(uint gpio) {
    gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_FALL, true, &debounce_callback);
}

int main() {
    stdio_init_all();

    // Configuração dos botões com interrupções
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(JOYSTICK_BTN);
    gpio_set_dir(JOYSTICK_BTN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN);
    setup_button_irq(JOYSTICK_BTN);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    setup_button_irq(BUTTON_A);

    // Configuração do ADC para o joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    // Configuração dos LEDs PWM
    setup_pwm(LED_RED);
    setup_pwm(LED_BLUE);
    setup_pwm(LED_GREEN); // Adiciona o PWM para o LED verde

    // Configuração do I2C para o display OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display OLED
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Posição inicial do quadrado
    int x_pos = 59;
    int y_pos = 29;

    while (true) {
        // Ajusta a intensidade do PWM dos LEDs com base no movimento do joystick
        int pwm_red = 0;
        int pwm_blue = 0;

        // Verifica se o botão B foi pressionado para entrar em modo BOOTSEL
        if (!gpio_get(BUTTON_B)) {
            printf("[SISTEMA] Entrando em modo BOOTSEL\n");
            reset_usb_boot(0, 0);
        }

        // Leitura dos valores do joystick
        adc_select_input(0);
        uint16_t x_val = adc_read();
        adc_select_input(1);
        uint16_t y_val = adc_read();

        int adjusted_x = x_val - JOYSTICK_CENTER_X; // Movimentos no eixo X (direita/esquerda)
        int adjusted_y = y_val - JOYSTICK_CENTER_Y; // Movimentos no eixo Y (cima/baixo)

        // Movimentos corretos para o eixo X (cima/baixo) - Trocar com Y
        if (abs(adjusted_y) > DEADZONE) {
            x_pos += (adjusted_y * 5) / 2048; // Ajuste proporcional ao valor do joystick
        }

        // Calcula intensidade para o LED Vermelho (eixo Y)
        if (abs(adjusted_y) > DEADZONE)
        {
            int intensidade = abs(adjusted_y) - DEADZONE;
            pwm_red = (intensidade * 4095) / (4095 - JOYSTICK_CENTER_X - DEADZONE);
        }

        // Calcula intensidade para o LED Azul (eixo X)
        if (abs(adjusted_x) > DEADZONE)
        {
            int intensidade = abs(adjusted_x) - DEADZONE;
            pwm_blue = (intensidade * 4095) / (4095 - JOYSTICK_CENTER_Y - DEADZONE);
        }

        // Movimentos corretos para o eixo Y (direita/esquerda) - Trocar com X
        if (abs(adjusted_x) > DEADZONE)
        {
            y_pos -= (adjusted_x * 5) / 2048; // Ajuste proporcional ao valor do joystick
        }

        // Limitações da tela para não ultrapassar os limites
        if (x_pos < 0)
            x_pos = 0;
        if (x_pos > WIDTH - 8)
            x_pos = WIDTH - 8;
        if (y_pos < 0)
            y_pos = 0;
        if (y_pos > HEIGHT - 8)
            y_pos = HEIGHT - 8;

        // Inverte os valores para ajustar a posição
        int inverted_x_pos = y_pos; // Agora y_pos vai para o eixo X
        int inverted_y_pos = x_pos; // Agora x_pos vai para o eixo Y

        // Limpa a tela
        ssd1306_fill(&ssd, false);

        // Desenha as bordas
        if (border_style == 1)
        {
            ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false); // Manter o tamanho da borda original
        }
        else if (border_style == 2)
        {
            ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false); // Manter o tamanho da borda original
            ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, true, false);
            ssd1306_rect(&ssd, 2, 2, WIDTH - 4, HEIGHT - 4, true, false);
        }

        // Desenha o quadrado com as coordenadas invertidas
        ssd1306_rect(&ssd, inverted_x_pos, inverted_y_pos, 8, 8, true, true);
        ssd1306_send_data(&ssd);

        printf("[JOYSTICK] X: %4d | Y: %4d | Pos: (%3d, %3d)\n", x_val, y_val, inverted_x_pos, inverted_y_pos);

        // Ajusta a intensidade do PWM dos LEDs com base na posição do joystick
        if (pwm_enabled)
        {
            pwm_set_gpio_level(LED_RED, pwm_red);
            pwm_set_gpio_level(LED_BLUE, pwm_blue);
        }
        else
        {
            pwm_set_gpio_level(LED_RED, 0);
            pwm_set_gpio_level(LED_BLUE, 0);
        }

        // Pequeno atraso para evitar leituras muito rápidas
        sleep_ms(20);
    }
}
