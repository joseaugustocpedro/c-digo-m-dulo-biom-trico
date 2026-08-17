# Embedded Biometric Access Control

> Sistema embarcado de cadastro e autenticação biométrica com **ESP32**, sensor de impressão digital, **painel Web local**, persistência de usuários em memória não volátil e sinalização visual por LEDs.

![C++](https://img.shields.io/badge/C%2B%2B-Embedded-blue?logo=cplusplus)
![ESP32](https://img.shields.io/badge/ESP32-Wi--Fi%20MCU-black?logo=espressif)
![Arduino](https://img.shields.io/badge/Arduino-Framework-00878F?logo=arduino)
![Biometrics](https://img.shields.io/badge/Biometrics-Fingerprint-success)
![Status](https://img.shields.io/badge/status-prototype-orange)

## Visão geral

Este projeto implementa um protótipo de **controle de acesso biométrico** no qual o ESP32 integra aquisição de impressão digital, identificação do usuário, persistência de dados e uma interface Web para administração local.

A solução demonstra uma cadeia completa de engenharia:

```text
Usuário
  │
  ▼
Sensor biométrico
  │ UART
  ▼
ESP32
  ├── processamento da autenticação
  ├── armazenamento de nomes em NVS / Preferences
  ├── servidor HTTP local
  ├── interface Web
  └── sinalização por LEDs
        │
        ├── verde    → acesso autorizado
        └── vermelho → acesso negado / falha
```

O banco biométrico permanece armazenado no próprio módulo de impressão digital. O ESP32 mantém a associação entre **ID biométrico e nome do usuário** utilizando a API `Preferences`, baseada na memória não volátil do microcontrolador.

## Funcionalidades

- cadastro de impressão digital em dois estágios de captura;
- associação entre **ID e nome do usuário**;
- reconhecimento biométrico em tempo real;
- autorização ou negação visual de acesso;
- remoção de digitais cadastradas;
- painel Web responsivo acessível pela rede local;
- atualização dinâmica do estado do sistema no navegador;
- listagem dos usuários cadastrados;
- persistência dos nomes após reinicialização do ESP32;
- mapeamento dos IDs existentes no sensor durante a inicialização;
- timeout durante o processo de cadastro;
- validação de IDs entre `1` e `127`;
- proteção das credenciais Wi-Fi contra versionamento acidental.

## Arquitetura do sistema

```text
                         REDE WI-FI LOCAL
                               │
                     HTTP / porta 80
                               │
                               ▼
┌──────────────────────────────────────────────────────┐
│                       ESP32                          │
│                                                      │
│  WebServer        Controle biométrico     Preferences│
│      │                    │                    │      │
│      │                    │                    └── NVS│
│      │                    │                           │
│      └──────────────┬─────┘                           │
│                     │                                 │
│          UART2 - GPIO 16 / GPIO 17                    │
└─────────────────────┼─────────────────────────────────┘
                      │
                      ▼
             Sensor de impressão digital
                      │
               Banco de templates

ESP32 GPIO 18 ─────────────► LED verde
ESP32 GPIO 19 ─────────────► LED vermelho
```

## Hardware

| Componente | Função |
|---|---|
| ESP32 | Processamento, Wi-Fi, servidor Web e controle do sistema |
| Sensor biométrico compatível com Adafruit Fingerprint | Captura, armazenamento e comparação das impressões digitais |
| LED verde | Indicação de autenticação bem-sucedida |
| LED vermelho | Indicação de acesso negado ou erro |
| Resistores para LEDs | Limitação de corrente |
| Protoboard / jumpers | Interligação do protótipo |
| Fonte adequada | Alimentação do ESP32 e do sensor |

### Pinagem utilizada

| Sinal | ESP32 |
|---|---:|
| LED verde | GPIO 18 |
| LED vermelho | GPIO 19 |
| RX do ESP32 / UART2 | GPIO 16 |
| TX do ESP32 / UART2 | GPIO 17 |
| Comunicação do sensor | 57600 baud |
| Monitor Serial | 115200 baud |

> A ligação TX/RX deve ser cruzada: o TX do sensor deve chegar ao RX do ESP32 e o RX do sensor ao TX do ESP32. Confirme também a tensão de alimentação e os níveis lógicos suportados pelo módulo biométrico utilizado.

## Software e bibliotecas

O firmware foi desenvolvido para o ecossistema Arduino utilizando:

- **ESP32 Arduino Core**;
- `WiFi.h`;
- `WebServer.h`;
- `Preferences.h`;
- `Adafruit_Fingerprint.h` / **Adafruit Fingerprint Sensor Library**.

## Estrutura do repositório

```text
embedded-biometric-access-control/
├── firmware/
│   ├── esp32_web_biometric_access.ino   # versão atual: ESP32 + painel Web
│   └── secrets.example.h                # modelo das credenciais Wi-Fi
├── biometric_access_control.ino         # versão legada: Arduino Mega + relé
├── .gitignore
└── README.md
```

## Configuração do projeto

### 1. Preparar a Arduino IDE

Instale o suporte à placa ESP32 na Arduino IDE e selecione o modelo correspondente à sua placa.

Depois, pelo Gerenciador de Bibliotecas, instale:

```text
Adafruit Fingerprint Sensor Library
```

As bibliotecas `WiFi`, `WebServer` e `Preferences` fazem parte do ambiente ESP32 Arduino.

### 2. Configurar o Wi-Fi sem publicar a senha

Dentro da pasta `firmware`, copie:

```text
secrets.example.h
```

para:

```text
secrets.h
```

Edite apenas o arquivo local `secrets.h`:

```cpp
#pragma once

#define WIFI_SSID "NOME_DA_REDE"
#define WIFI_PASSWORD "SENHA_DA_REDE"
```

O arquivo `firmware/secrets.h` está listado no `.gitignore` e não deve ser enviado ao GitHub.

### 3. Compilar e enviar

Abra:

```text
firmware/esp32_web_biometric_access.ino
```

Compile e envie o firmware para o ESP32.

Com o Monitor Serial configurado em `115200 baud`, após a conexão Wi-Fi será exibido um endereço semelhante a:

```text
Acesse o painel em: http://192.168.1.100
```

Abra esse endereço em um dispositivo conectado à mesma rede local.

## Operação do painel Web

A interface permite cadastrar e remover usuários sem utilizar o Monitor Serial.

### Cadastro biométrico

1. Digite o **nome do usuário**.
2. Informe um **ID entre 1 e 127**.
3. Clique em **Cadastrar Digital**.
4. Coloque o dedo no sensor.
5. Retire o dedo quando solicitado.
6. Posicione o mesmo dedo novamente.
7. O sensor cria o modelo biométrico e o associa ao ID informado.
8. O ESP32 salva o nome correspondente na memória não volátil.

Fluxo lógico:

```text
Solicitação pelo navegador
          │
          ▼
Validação de ID e nome
          │
          ▼
Primeira captura
          │
          ▼
Conversão para template #1
          │
          ▼
Retirada do dedo
          │
          ▼
Segunda captura
          │
          ▼
Conversão para template #2
          │
          ▼
Comparação das capturas
      ┌───┴────┐
      │        │
   falha     iguais
      │        │
      ▼        ▼
 cancela   armazena ID
               │
               ▼
        grava nome na NVS
```

### Autenticação

Durante a operação normal o sensor é consultado continuamente.

```text
Digital capturada
       │
       ▼
Conversão da imagem
       │
       ▼
Busca no banco biométrico
    ┌──┴────┐
    │       │
  não      sim
    │       │
    ▼       ▼
LED       recupera nome
vermelho      │
              ▼
          LED verde
              │
              ▼
     "ACESSO PERMITIDO"
```

## Persistência dos dados

O projeto utiliza dois mecanismos de armazenamento:

| Dado | Local de armazenamento |
|---|---|
| Template da impressão digital | Memória interna do sensor biométrico |
| ID biométrico | Sensor biométrico |
| Nome associado ao ID | NVS do ESP32 por meio de `Preferences` |

O namespace utilizado no ESP32 é:

```text
dadosBio
```

Na inicialização, o firmware percorre os IDs `1` a `127`, verifica quais templates existem no sensor e reconstrói a lista exibida no painel Web.

## Interface HTTP

O ESP32 opera como um pequeno servidor HTTP local.

| Endpoint | Método | Função |
|---|---|---|
| `/` | GET | Entrega o painel Web |
| `/status` | GET | Retorna status do sistema e usuários cadastrados em JSON |
| `/cadastrar?id=<ID>&nome=<NOME>` | GET | Solicita novo cadastro biométrico |
| `/remover?id=<ID>` | GET | Remove o template e o nome associado |

A interface consulta `/status` periodicamente para atualizar o navegador sem recarregar a página inteira.

## Decisões de engenharia

### UART de hardware

O sensor biométrico utiliza a `HardwareSerial(2)` do ESP32. A escolha evita implementar comunicação serial por software e mantém a interface com o sensor independente da porta utilizada para diagnóstico.

### Separação entre biometria e identificação textual

O sensor armazena templates biométricos e retorna um ID numérico. O nome não é armazenado no sensor; ele é mantido no ESP32. Essa separação permite que a camada de apresentação trabalhe com nomes legíveis sem alterar o mecanismo interno de identificação biométrica.

### Persistência com Preferences

A API `Preferences` foi utilizada para preservar a relação `ID → nome` após desligamentos ou reinicializações, sem necessidade de banco de dados externo no estágio atual do protótipo.

### Servidor embarcado

A interface Web é armazenada no próprio firmware e servida diretamente pelo ESP32. Assim, o protótipo não depende de computador, servidor externo ou aplicação móvel para executar as funções básicas de administração.

## Segurança e limitações

Este repositório representa um **protótipo acadêmico/experimental** e não deve ser tratado como um sistema de segurança pronto para produção.

Pontos que exigiriam evolução em uma implantação real:

- autenticação administrativa no painel Web;
- HTTPS ou outra camada de comunicação protegida;
- substituição das operações administrativas via `GET` por métodos apropriados e proteção contra requisições indevidas;
- controle de sessão e autorização;
- registro de auditoria de acessos;
- proteção física do microcontrolador e do barramento UART;
- avaliação formal da proteção dos dados biométricos;
- política de backup, revogação e atualização de usuários;
- tratamento de indisponibilidade da rede;
- acionamento elétrico isolado e protegido caso seja integrada uma fechadura real.

> Credenciais Wi-Fi nunca devem ser versionadas. Use somente `secrets.h` local, conforme descrito neste README.

## Evolução do projeto

O repositório também preserva uma versão anterior baseada em **Arduino Mega 2560**, comunicação serial e acionamento de relé:

```text
biometric_access_control.ino
```

A versão ESP32 representa uma evolução arquitetural importante:

```text
Arduino Mega + Serial + Relé
             │
             ▼
ESP32 + Wi-Fi + WebServer + Preferences + identificação nominal
```

Isso permite acompanhar a evolução do protótipo desde um controle biométrico local até uma plataforma embarcada com gerenciamento pela rede.

## Competências demonstradas

- sistemas embarcados;
- Internet das Coisas (IoT);
- ESP32;
- programação C/C++;
- comunicação UART;
- sensores biométricos;
- memória não volátil;
- desenvolvimento de servidor HTTP embarcado;
- HTML, CSS e JavaScript integrados ao firmware;
- integração hardware/software;
- arquitetura de sistemas;
- diagnóstico de falhas;
- prototipagem eletrônica;
- controle de acesso.

## Próximas evoluções

- [ ] histórico de entradas com data e horário;
- [ ] sincronização de tempo via NTP;
- [ ] autenticação do administrador;
- [ ] integração com relé ou fechadura eletromagnética;
- [ ] dashboard com logs e métricas;
- [ ] exportação dos registros de acesso;
- [ ] API REST estruturada;
- [ ] integração com MQTT ou banco de dados externo;
- [ ] atualização OTA do firmware;
- [ ] gabinete dedicado para o protótipo;
- [ ] testes automatizados das funções independentes de hardware.

## Contexto acadêmico

Projeto desenvolvido no contexto da **Engenharia de Controle e Automação** como aplicação prática de sistemas embarcados, instrumentação, comunicação digital e integração hardware/software.

### Autores da versão acadêmica original

- Arnaldo de C. Junior
- Guilherme R. de Souza
- João V. A. Ferreira
- **José A. C. Pedro**

---

**Objetivo do repositório:** documentar não apenas o código-fonte, mas também as decisões de arquitetura, interfaces, limitações e possibilidades de evolução do sistema — elementos fundamentais na documentação de um projeto de engenharia.
