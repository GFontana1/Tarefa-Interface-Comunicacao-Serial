# Interface Comunicação Serial - Tarefa Embarcatech 03/02

## Descrição do projeto

O programa implementa uma interface interativa em que, a partir de uma entrada de dados via monitor serial e interações com botões é possível controlar:

- 1 **LED Azul**
- 1 **LED Verde**
- Uma **matriz de LEDs WS2812**
- Um **display SSD1306**

## Funcionalidades

1. Entrada de Caracteres via PC
- Utiliza o Monitor Serial do VS Code para entrada de caracteres.
- Caracteres digitados no Monitor Serial são exibidos no display SSD1306.
- Caso um número entre 0 e 9 seja digitado, o símbolo correspondente é exibido na matriz de LEDs 5x5 WS2812 e no display.

2. Interação com o Botão A
- Pressionar o botão A alterna o estado do LED RGB Verde (aceso/apagado).
- As operações são registradas de duas formas:
  - Mensagem informativa sobre o estado do LED exibida no display SSD1306.
  - Texto descritivo enviado ao Monitor Serial.

3. Interação com o Botão B
- Pressionar o botão B alterna o estado do LED RGB Azul (aceso/apagado).
- As operações são registradas de duas formas:
  - Mensagem informativa sobre o estado do LED exibida no display SSD1306.
  - Texto descritivo enviado ao Monitor Serial.

## Como rodar o código

1. *Necessário para compilar o código:*
    - Ter uma placa Raspberry Pi Pico W que contenha os componentes descritos na *Descrição do projeto*.
    - Ter o SDK do Raspberry Pi Pico W configurado.
    - Compilar o código utilizando CMake e GCC ARM.

2. *Transferir para o Raspberry Pi Pico W*:
   - Conecte sua placa ao computador com ela no modo **Bootsel**.
   - Monte a unidade **RPI-RP2** no computador.
   - Copie o arquivo compilado `.uf2` para a unidade montada ou aperte em Run na interface do VS Code caso tenha configurado a placa com o Zadig.

## Vídeo Demonstrativo
https://www.youtube.com/shorts/QY-vuIrUla8