# Projeto: Controle de Matriz de LEDs WS2812 com RP2040

# By: Jaime Neto
# Link do projeto rodando: 

## 📌 Descrição
Este projeto utiliza a placa RP2040 (BitDogLab) para controlar uma matriz de LEDs WS2812 e um LED RGB com interação via botões e joystick. A matriz exibe números de 0 a 9, que podem ser alterados pelos botões, e muda de cor aleatoriamente ao pressionar o joystick.

## 🚀 Funcionalidades
- Exibição de números (0-9) em matriz WS2812.
- Controle via botões:
  - **Botão A**: Incrementa o número exibido.
  - **Botão B**: Decrementa o número exibido.
- Alterar cores da matriz ao pressionar o **botão do joystick**.
- LED vermelho piscando constantemente para indicar funcionamento.
- Interrupções para capturar eventos dos botões com debounce.

## 🔧 Hardware Utilizado
- Placa **RP2040 (BitDogLab)**
- Matriz de LEDs **WS2812**
- **LED RGB**
- **Botões** (A e B)
- **Joystick com botão**

## 🛠 Configuração dos Pinos
| Componente        | Pino RP2040 |
|-------------------|-------------|
| LED Vermelho      | 13          |
| LED Verde         | 11          |
| LED Azul          | 12          |
| Botão A           | 5           |
| Botão B           | 6           |
| Matriz WS2812     | 7           |
| Joystick Button   | 22          |

## 📜 Funcionamento
1. O programa inicia configurando os pinos, inicializando o PIO para controle da matriz WS2812 e configurando interrupções nos botões.
2. O LED vermelho pisca continuamente a cada 100ms.
3. Ao pressionar **Botão A**, o número exibido na matriz aumenta (0-9).
4. Ao pressionar **Botão B**, o número exibido diminui (9-0).
5. Ao pressionar **o botão do joystick**, novas cores aleatórias são geradas para o número e o fundo.

## 🎯 Como Executar
1. Compile e carregue o código no RP2040.
2. Conecte a matriz WS2812 e os botões conforme a tabela de pinos.
3. A matriz deve exibir um número inicial, que pode ser alterado pelos botões.

## 📂 Estrutura do Projeto
```
Tarefa_interrupcao/
├── src/
│   ├── main.c  # Código principal
│   ├── ws2812.pio  # Programa PIO para LEDs WS2812
│   ├── CMakeLists.txt  # Configuração do CMake
├── README.md  # Documentação do projeto
```

## 📝 Possíveis Melhorias
- Criar efeitos animados na matriz WS2812.
- Implementar controle de brilho via PWM.
- Suporte para mais animações e interações.

## 📜 Licença
Este projeto é de livre uso para estudos e modificações.

