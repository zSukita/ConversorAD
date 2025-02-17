# Projeto BitDogLab - Controle de LEDs e Display SSD1306 com Joystick

Este projeto utiliza o **Raspberry Pi Pico** para controlar LEDs RGB via **PWM**, exibir a posição de um joystick em um display **SSD1306** e interagir com botões. O código foi desenvolvido para a placa de desenvolvimento **BitDogLab**.

## **Requisitos de Hardware**
- **Raspberry Pi Pico**
- **BitDogLab** com os seguintes componentes:
  - **LED RGB** conectado às GPIOs 11 (verde), 12 (azul) e 13 (vermelho).
  - **Joystick** conectado às GPIOs 26 (eixo Y) e 27 (eixo X).
  - **Botão do Joystick** conectado à GPIO 22.
  - **Botão A** conectado à GPIO 5.
  - **Display OLED SSD1306** conectado via I2C (GPIO 14 - SDA, GPIO 15 - SCL).

## **Funcionalidades**
1. **Controle de LEDs RGB**:
   - **LED Azul**: O brilho é controlado pelo eixo Y do joystick. Quando o joystick está no centro, o LED está apagado. Movendo o joystick para cima ou para baixo, o brilho aumenta proporcionalmente.
   - **LED Vermelho**: O brilho é controlado pelo eixo X do joystick, seguindo o mesmo princípio do LED azul.
   - **LED Verde**: O estado do LED verde é alternado a cada vez que o botão do joystick é pressionado.
   - **PWM**: Os LEDs são controlados via PWM para uma variação suave de intensidade.

2. **Display SSD1306**:
   - **Quadrado Móvel**: Um quadrado de 8x8 pixels é exibido no display e se move proporcionalmente à posição do joystick.
   - **Bordas**: O estilo da borda do display é alterado a cada vez que o botão do joystick é pressionado.

3. **Botão A**:
   - **Ativar/Desativar PWM**: O botão A liga ou desliga o controle de intensidade dos LEDs vermelho e azul.

## **Como Utilizar o Programa**

### **1. Compilação e Upload do Código**
1. **Configuração do Ambiente**:
   - Certifique-se de que o **Raspberry Pi Pico SDK** está instalado e configurado corretamente.
   - Instale o **CMake** e o **GCC** para compilar o projeto.

2. **Compilação**:
   - No terminal, navegue até o diretório do projeto e crie uma pasta `build`:
     ```
     mkdir build
     cd build
     ```
   - Execute o `cmake` para configurar o projeto:
     ```
     cmake ..
     ```
   - Compile o código:
     ```
     make
     ```

3. **Upload para o Pico**:
   - Conecte o Raspberry Pi Pico ao computador via USB enquanto pressiona o botão **BOOTSEL**.
   - Arraste o arquivo `.uf2` gerado na pasta `build` para o dispositivo RPI-RP2 que aparece no sistema de arquivos.

### **2. Utilização do Sistema**
- **Joystick**:
  - Mova o joystick para controlar o brilho dos LEDs vermelho e azul.
  - O joystick também move o quadrado no display OLED.
- **Botão do Joystick**:
  - Pressione o botão do joystick para alternar o estado do LED verde e mudar o estilo da borda do display.
- **Botão A**:
  - Pressione o botão A para ligar ou desligar o controle de intensidade dos LEDs vermelho e azul.

### **3. Visualização do Display**
- O quadrado no display OLED se move de acordo com a posição do joystick.
- O estilo da borda do display muda a cada vez que o botão do joystick é pressionado.

### **4. Reinicialização para Modo BOOTSEL**
- Pressione o **botão B** (conectado à GPIO 6) para entrar no modo **BOOTSEL** e reprogramar o Pico.

## **Estrutura do Código**
- **Funções Principais**:
  - `setup_pwm(uint pin)`: Configura o PWM para um pino específico.
  - `debounce_callback(uint gpio, uint32_t events)`: Trata as interrupções dos botões com debouncing.
  - `setup_button_irq(uint gpio)`: Configura as interrupções dos botões.
  - `main()`: Inicializa os componentes e entra no loop principal do programa.

- **Bibliotecas Utilizadas**:
  - `pico/stdlib.h`: Para controle básico do Pico.
  - `hardware/adc.h`: Para leitura analógica do joystick.
  - `hardware/pwm.h`: Para controle PWM dos LEDs.
  - `hardware/i2c.h`: Para comunicação I2C com o display OLED.
  - `ssd1306.h`: Para controle do display SSD1306.

## **Considerações Finais**
Este projeto demonstra o uso de **ADC**, **PWM**, **I2C** e **interrupções** no Raspberry Pi Pico, integrando vários componentes para criar uma aplicação interativa. O código está bem estruturado e comentado para facilitar a compreensão e modificação.

Para mais informações sobre o **Raspberry Pi Pico SDK**, consulte a [documentação oficial](https://www.raspberrypi.com/documentation/pico-sdk/).
