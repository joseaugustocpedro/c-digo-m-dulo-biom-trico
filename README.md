# Sistema Embarcado para Cadastro e Verificação Biométrica

Este repositório contém o código-fonte e a documentação técnica do projeto desenvolvido para a disciplina de Engenharia de Controle e Automação. O projeto consiste em um protótipo de controle de acesso biométrico de baixo custo utilizando a plataforma Arduino.

## 📄 Sobre o Projeto

Este sistema foi projetado para resolver vulnerabilidades de métodos tradicionais de controle de acesso (chaves e cartões). Utilizando um sensor de impressão digital óptico e um microcontrolador Arduino Mega 2560, o sistema é capaz de:

* Cadastrar (Enroll): Registrar novas impressões digitais na memória flash do sensor.
* Verificar (Match): Comparar uma digital lida com o banco de dados e liberar/negar o acesso em tempo real.
* Controlar Acesso: Acionar um relé (simulando uma fechadura ou portão) mediante autenticação positiva.

Este projeto serve como base técnica para o artigo científico: *"Sistema Embarcado para Cadastro e Verificação Biométrica Baseado em Impressão Digital"*.

## 🛠️ Hardware Utilizado

A lista de materiais para reprodução do protótipo inclui:

* **Microcontrolador:** Arduino Mega 2560 (Utilizado devido às múltiplas portas Hardware Serial).
* **Sensor Biométrico:** Modelo óptico (compatível com AS608/R307).
* **Atuador:** Módulo Relé de 1 canal (5V).
* **Interface:** Botões (para iniciar cadastro) e LEDs (Verde/Vermelho para feedback de status).
* **Componentes:** Resistores, Protoboard e Jumpers.


## 💻 Software e Dependências

O firmware foi desenvolvido na **Arduino IDE** utilizando C/C++.

### Bibliotecas Necessárias
Para compilar o código, é necessário instalar a seguinte biblioteca através do Gerenciador de Bibliotecas da Arduino IDE:

* `Adafruit Fingerprint Sensor Library` (ou compatível com o sensor utilizado).


## 👥 Autores

* **Arnaldo de C. Junior**
* **Guilherme R. de Souza**
* **João V. A. Ferreira**
* **José A. C. Pedro**

---
*Projeto desenvolvido no Instituto Federal de Educação, Ciência e Tecnologia de São Paulo (IFSP).*
