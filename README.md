# Embedded Biometric Access Control

Sistema embarcado de controle de acesso biométrico desenvolvido com **Arduino Mega 2560**, sensor óptico de impressão digital, relé e sinalização por LEDs.

O protótipo foi desenvolvido no contexto da Engenharia de Controle e Automação e demonstra a integração completa entre **sensor → microcontrolador → firmware → decisão lógica → atuador**.

## Funcionalidades

- cadastro de novas impressões digitais;
- verificação biométrica em tempo real;
- remoção de usuários cadastrados;
- liberação de acesso por relé após autenticação positiva;
- feedback visual por LEDs verde/vermelho;
- mensagens de diagnóstico pelo Monitor Serial;
- tratamento de falhas de comunicação e leitura do sensor.

## Arquitetura do sistema

```text
Usuário
   ↓
Sensor biométrico
   ↓ Serial2 (UART)
Arduino Mega 2560
   ├── Firmware C/C++
   ├── LED verde/vermelho
   └── Relé
          ↓
   Fechadura / catraca simulada
```

## Hardware

- **Microcontrolador:** Arduino Mega 2560
- **Sensor:** sensor biométrico óptico compatível com AS608/R307
- **Atuador:** módulo relé 1 canal, 5 V
- **Sinalização:** LEDs verde e vermelho
- **Montagem:** protoboard, resistores e jumpers

O Arduino Mega foi utilizado por disponibilizar portas seriais de hardware adicionais, permitindo comunicação dedicada com o sensor biométrico.

## Software

- Arduino IDE
- C/C++
- Adafruit Fingerprint Sensor Library

## Estrutura do repositório

```text
├── biometric_access_control.ino
└── README.md
```

## Como utilizar

1. Instale a **Arduino IDE**.
2. Instale a biblioteca `Adafruit Fingerprint Sensor Library` pelo Gerenciador de Bibliotecas.
3. Abra `biometric_access_control.ino`.
4. Conecte o sensor à porta `Serial2` do Arduino Mega (RX2/TX2 — pinos 16 e 17), respeitando a alimentação e o cruzamento TX/RX exigido pelo módulo utilizado.
5. Ajuste as conexões de LEDs e relé conforme as constantes definidas no início do firmware.
6. Compile e envie o código para a placa.
7. Abra o Monitor Serial em **9600 baud**.

### Comandos pelo Monitor Serial

- número positivo, por exemplo `5`: cadastra uma impressão no ID 5;
- número negativo, por exemplo `-5`: remove o ID 5;
- durante a operação normal, uma digital reconhecida aciona o relé por 3 segundos.

## Fluxo de autenticação

```text
Captura da impressão
        ↓
Conversão da imagem
        ↓
Busca no banco interno do sensor
     ↙      ↘
 não         sim
  ↓           ↓
acesso      aciona relé
negado      + LED verde
```

## Competências demonstradas

- sistemas embarcados;
- programação C/C++;
- microcontroladores;
- integração de sensores e atuadores;
- comunicação serial;
- eletrônica e prototipagem;
- lógica de controle;
- diagnóstico de falhas.

## Contexto acadêmico

Projeto desenvolvido no **Instituto Federal de Educação, Ciência e Tecnologia de São Paulo (IFSP)** como aplicação prática em Engenharia de Controle e Automação.

### Autores

- Arnaldo de C. Junior
- Guilherme R. de Souza
- João V. A. Ferreira
- **José A. C. Pedro**

## Próximas evoluções possíveis

- substituição do relé por acionamento de fechadura eletromagnética real com circuito de potência adequado;
- registro de eventos de acesso;
- display local para interface do usuário;
- integração com rede ou banco de dados externo;
- gabinete dedicado para o protótipo.

> Projeto acadêmico e educacional. Qualquer aplicação real de controle de acesso exige cuidados adicionais de segurança elétrica, mecânica e de proteção de dados biométricos.
