#include <Adafruit_Fingerprint.h>

// --- Configuração dos Pinos ---
#define LED_VERDE_PIN 7
#define LED_VERMELHO_PIN 8
#define RELE_PIN 4  // Pino onde o relé (Catraca) está conectado

/*
 * Configuração do Sensor no Arduino Mega
 * Usaremos a porta Serial de Hardware 'Serial2' (Pinos 16 e 17)
 */
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);

// --- Protótipos de Funções ---
void removerDigital(int id);
void cadastrarDigital(int id);
void verificarDigital();
void piscarLed(int pino, int vezes, int duracao);
void abrirCatraca();

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Serial.println("\n--- Sistema de Controle de Acesso (Serial + Relé) ---");

  pinMode(LED_VERDE_PIN, OUTPUT);
  pinMode(LED_VERMELHO_PIN, OUTPUT);
  pinMode(RELE_PIN, OUTPUT);

  digitalWrite(LED_VERDE_PIN, LOW);
  digitalWrite(LED_VERMELHO_PIN, LOW);
  digitalWrite(RELE_PIN, LOW);

  finger.begin(57600);
  delay(500);

  if (finger.verifyPassword()) {
    Serial.println("Módulo de Digital Encontrado!");
  } else {
    Serial.println("Módulo de Digital NÃO encontrado :(");
    Serial.println("Verifique as conexões (RX/TX) e a alimentação.");
    while (1) {
      piscarLed(LED_VERMELHO_PIN, 3, 200);
    }
  }

  Serial.println("------------------------------------------------");
  Serial.println("Pronto para operação.");
  Serial.println("-> Para CADASTRAR: Digite o ID (ex: 5) e envie.");
  Serial.println("-> Para REMOVER:   Digite o ID negativo (ex: -5) e envie.");
  Serial.println("------------------------------------------------");
}

void loop() {
  if (Serial.available() > 0) {
    int id = Serial.parseInt();

    if (id > 0) {
      Serial.print("Comando recebido: Cadastrar ID #");
      Serial.println(id);
      cadastrarDigital(id);
      Serial.println("--- Retornando ao modo de leitura ---");
    } else if (id < 0) {
      int id_positivo = -id;
      Serial.print("Comando recebido: Remover ID #");
      Serial.println(id_positivo);
      removerDigital(id_positivo);
      Serial.println("--- Retornando ao modo de leitura ---");
    }
  }

  verificarDigital();
  delay(50);
}

void verificarDigital() {
  if (finger.getImage() != FINGERPRINT_OK) return;

  if (finger.image2Tz() != FINGERPRINT_OK) {
    Serial.println("Erro: Imagem ruim ou tremida.");
    piscarLed(LED_VERMELHO_PIN, 2, 100);
    return;
  }

  if (finger.fingerSearch() != FINGERPRINT_OK) {
    Serial.println("ACESSO NEGADO: Digital não encontrada.");
    piscarLed(LED_VERMELHO_PIN, 3, 150);
    return;
  }

  Serial.print("ACESSO PERMITIDO! ID Encontrado: #");
  Serial.print(finger.fingerID);
  Serial.print(" (Confiança: ");
  Serial.print(finger.confidence);
  Serial.println(")");

  abrirCatraca();
}

void abrirCatraca() {
  Serial.println(">> CATRACA LIBERADA <<");

  digitalWrite(LED_VERDE_PIN, HIGH);
  digitalWrite(RELE_PIN, HIGH);

  delay(3000);

  digitalWrite(LED_VERDE_PIN, LOW);
  digitalWrite(RELE_PIN, LOW);
  Serial.println(">> Catraca Bloqueada <<");
}

void cadastrarDigital(int id) {
  Serial.println("Passo 1: Coloque o dedo no sensor...");
  piscarLed(LED_VERMELHO_PIN, 1, 500);

  int p = -1;
  while (p != FINGERPRINT_OK) { p = finger.getImage(); }

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Erro na leitura 1. Cancelando.");
    piscarLed(LED_VERMELHO_PIN, 4, 100);
    return;
  }
  Serial.println("Leitura 1 OK!");

  Serial.println("Retire o dedo...");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) { p = finger.getImage(); }

  Serial.println("Passo 2: Coloque o MESMO dedo novamente...");
  piscarLed(LED_VERMELHO_PIN, 1, 500);

  p = -1;
  while (p != FINGERPRINT_OK) { p = finger.getImage(); }

  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Erro na leitura 2. Cancelando.");
    piscarLed(LED_VERMELHO_PIN, 4, 100);
    return;
  }
  Serial.println("Leitura 2 OK!");

  Serial.println("Processando modelo...");
  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("Erro: As digitais não conferem.");
    piscarLed(LED_VERMELHO_PIN, 4, 100);
    return;
  }

  Serial.print("Salvando no ID #");
  Serial.println(id);
  if (finger.storeModel(id) != FINGERPRINT_OK) {
    Serial.println("Erro ao salvar na memória flash.");
    piscarLed(LED_VERMELHO_PIN, 4, 100);
    return;
  }

  Serial.println(">>> SUCESSO! Digital Cadastrada. <<<");
  piscarLed(LED_VERDE_PIN, 3, 200);
}

void removerDigital(int id) {
  Serial.print("Tentando remover ID #");
  Serial.println(id);

  uint8_t p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println(">>> SUCESSO! ID Removido. <<<");
    piscarLed(LED_VERDE_PIN, 2, 150);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Erro de comunicação.");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Erro: ID não existe ou inválido.");
    piscarLed(LED_VERMELHO_PIN, 3, 100);
  } else {
    Serial.println("Erro desconhecido ao remover.");
  }
}

void piscarLed(int pino, int vezes, int duracao) {
  for (int i = 0; i < vezes; i++) {
    digitalWrite(pino, HIGH);
    delay(duracao);
    digitalWrite(pino, LOW);
    if (i < vezes - 1) delay(duracao);
  }
}
