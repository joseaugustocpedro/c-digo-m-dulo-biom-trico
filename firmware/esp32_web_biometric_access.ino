#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Fingerprint.h>
#include <Preferences.h>
#include "secrets.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

#define LED_VERDE_PIN 18
#define LED_VERMELHO_PIN 19

#define RX_PIN 16
#define TX_PIN 17

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

WebServer server(80);
Preferences memoria;

int modoOperacao = 0;
int idAlvo = -1;
String nomeAlvo = "";
String mensagemStatus = "Sistema Pronto. Aguardando digital...";

unsigned long tempoMensagem = 0;
bool exibindoMensagemTemp = false;
unsigned long tempoLed = 0;

bool idsAtivos[128] = {false};
String nomesAtivos[128];
String stringListaIDs = "Buscando...";

void cadastrarDigital(int id, String nome);
void removerDigital(int id);
void verificarDigital();
void exibirMensagemTemporaria(String msg);
void acenderLed(int pino);
void mapearIDsSalvos();
void atualizarTextoListaIDs();
void esperarSemTravar(int tempoMs);
bool idValido(int id);
String escaparJson(const String& texto);

const char PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Acesso Virtual Biométrico</title>
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f4f7f6; color: #333; text-align: center; padding: 20px; }
    .container { max-width: 500px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
    h1 { color: #2c3e50; margin-bottom: 5px; }

    .status-box {
      background: #e8f4f8; padding: 25px 15px; border-radius: 8px; margin-bottom: 20px;
      font-weight: bold; font-size: 20px; border-left: 8px solid #3498db;
      transition: all 0.2s ease;
    }

    .list-box {
      background: #fdfdfd; padding: 15px; border-radius: 5px; border: 1px dashed #bdc3c7;
      font-size: 14px; color: #7f8c8d; margin-bottom: 20px; text-align: left;
    }

    input[type="text"], input[type="number"] { width: 80%; padding: 10px; margin: 5px 0 15px 0; border: 1px solid #ccc; border-radius: 5px; font-size: 16px; }
    button { padding: 10px 20px; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; margin: 5px; color: white; transition: 0.3s; }
    .btn-cadastrar { background-color: #27ae60; }
    .btn-cadastrar:hover { background-color: #2ecc71; }
    .btn-remover { background-color: #c0392b; }
    .btn-remover:hover { background-color: #e74c3c; }
    .form-group { text-align: left; margin-left: 10%; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Painel de Acesso</h1>

    <div class="status-box" id="statusMensagem">
      Conectando ao sistema...
    </div>

    <div class="list-box">
      <strong>Usuários Cadastrados:</strong><br>
      <span id="listaUsuarios">Carregando...</span>
    </div>

    <h3>Gerenciar Usuários</h3>

    <div class="form-group">
      <label for="inputNome"><strong>Nome do Usuário:</strong></label><br>
      <input type="text" id="inputNome" placeholder="Ex: Maria Silva">
      <br>
      <label for="inputId"><strong>Número do ID:</strong></label><br>
      <input type="number" id="inputId" placeholder="Ex: 1, 2, 3..." min="1" max="127">
    </div>

    <button class="btn-cadastrar" onclick="enviarComando('cadastrar')">Cadastrar Digital</button>
    <button class="btn-remover" onclick="enviarComando('remover')">Remover Digital</button>
  </div>

  <script>
    setInterval(function() {
      fetch('/status')
        .then(response => response.json())
        .then(dados => {
          let box = document.getElementById('statusMensagem');
          let texto = dados.msg;
          box.innerText = texto;

          if (texto.includes("PERMITIDO") || texto.includes("SUCESSO")) {
            box.style.backgroundColor = "#d4edda";
            box.style.color = "#155724";
            box.style.borderLeftColor = "#28a745";
          } else if (texto.includes("Negado") || texto.includes("NEGADO") || texto.includes("Erro") || texto.includes("Cancelado")) {
            box.style.backgroundColor = "#f8d7da";
            box.style.color = "#721c24";
            box.style.borderLeftColor = "#dc3545";
          } else {
            box.style.backgroundColor = "#e8f4f8";
            box.style.color = "#333";
            box.style.borderLeftColor = "#3498db";
          }

          document.getElementById('listaUsuarios').innerHTML = dados.ids;
        });
    }, 300);

    function enviarComando(acao) {
      let id = document.getElementById('inputId').value;
      let nome = document.getElementById('inputNome').value.trim();

      if (!id) { alert("Por favor, digite um ID!"); return; }
      if (Number(id) < 1 || Number(id) > 127) { alert("O ID deve estar entre 1 e 127."); return; }
      if (acao === 'cadastrar' && !nome) { alert("Por favor, digite o Nome!"); return; }

      let url = '/' + acao + '?id=' + encodeURIComponent(id);
      if (acao === 'cadastrar') {
        url += '&nome=' + encodeURIComponent(nome);
      }

      fetch(url);

      document.getElementById('inputId').value = '';
      document.getElementById('inputNome').value = '';
    }
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE_PIN, OUTPUT);
  pinMode(LED_VERMELHO_PIN, OUTPUT);

  memoria.begin("dadosBio", false);

  mySerial.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(500);

  if (finger.verifyPassword()) {
    Serial.println("Sensor encontrado!");
    mapearIDsSalvos();
  } else {
    mensagemStatus = "Erro: Sensor não encontrado.";
  }

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Acesse o painel em: http://");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/html; charset=utf-8", PAGE_HTML);
  });

  server.on("/status", []() {
    String json = "{\"msg\":\"" + escaparJson(mensagemStatus) + "\",\"ids\":\"" + escaparJson(stringListaIDs) + "\"}";
    server.send(200, "application/json; charset=utf-8", json);
  });

  server.on("/cadastrar", []() {
    if (!server.hasArg("id") || !server.hasArg("nome")) {
      server.send(400, "text/plain", "Parametros ausentes");
      return;
    }

    int id = server.arg("id").toInt();
    String nome = server.arg("nome");
    nome.trim();

    if (!idValido(id) || nome.length() == 0) {
      server.send(400, "text/plain", "ID ou nome invalido");
      return;
    }

    idAlvo = id;
    nomeAlvo = nome;
    modoOperacao = 1;
    mensagemStatus = "Cadastro iniciado: " + nomeAlvo + " (ID " + String(idAlvo) + ")";
    server.send(200, "text/plain", "OK");
  });

  server.on("/remover", []() {
    if (!server.hasArg("id")) {
      server.send(400, "text/plain", "Parametro ID ausente");
      return;
    }

    int id = server.arg("id").toInt();
    if (!idValido(id)) {
      server.send(400, "text/plain", "ID invalido");
      return;
    }

    idAlvo = id;
    modoOperacao = 2;
    mensagemStatus = "Removendo ID " + String(idAlvo);
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (exibindoMensagemTemp && (millis() - tempoMensagem > 4000)) {
    mensagemStatus = "Sistema Pronto. Aguardando digital...";
    exibindoMensagemTemp = false;
  }

  if ((digitalRead(LED_VERDE_PIN) || digitalRead(LED_VERMELHO_PIN)) && (millis() - tempoLed > 2000)) {
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_VERMELHO_PIN, LOW);
  }

  if (modoOperacao == 1) {
    cadastrarDigital(idAlvo, nomeAlvo);
    modoOperacao = 0;
  } else if (modoOperacao == 2) {
    removerDigital(idAlvo);
    modoOperacao = 0;
  } else {
    verificarDigital();
    delay(10);
  }
}

bool idValido(int id) {
  return id >= 1 && id <= 127;
}

String escaparJson(const String& texto) {
  String resultado;
  resultado.reserve(texto.length() + 8);

  for (size_t i = 0; i < texto.length(); i++) {
    char c = texto.charAt(i);
    switch (c) {
      case '\\': resultado += "\\\\"; break;
      case '"': resultado += "\\\""; break;
      case '\n': resultado += "\\n"; break;
      case '\r': resultado += "\\r"; break;
      case '\t': resultado += "\\t"; break;
      default: resultado += c; break;
    }
  }

  return resultado;
}

void exibirMensagemTemporaria(String msg) {
  mensagemStatus = msg;
  tempoMensagem = millis();
  exibindoMensagemTemp = true;
}

void acenderLed(int pino) {
  digitalWrite(LED_VERDE_PIN, LOW);
  digitalWrite(LED_VERMELHO_PIN, LOW);
  digitalWrite(pino, HIGH);
  tempoLed = millis();
}

void esperarSemTravar(int tempoMs) {
  unsigned long inicio = millis();
  while (millis() - inicio < (unsigned long)tempoMs) {
    server.handleClient();
    delay(10);
  }
}

void mapearIDsSalvos() {
  for (int i = 1; i <= 127; i++) {
    if (finger.loadModel(i) == FINGERPRINT_OK) {
      idsAtivos[i] = true;
      nomesAtivos[i] = memoria.getString(String(i).c_str(), "Sem Nome");
    } else {
      idsAtivos[i] = false;
    }
  }
  atualizarTextoListaIDs();
}

void atualizarTextoListaIDs() {
  String lista = "";
  for (int i = 1; i <= 127; i++) {
    if (idsAtivos[i]) {
      lista += "ID " + String(i) + " - " + nomesAtivos[i] + "<br>";
    }
  }

  if (lista == "") {
    stringListaIDs = "Nenhum usuário cadastrado.";
  } else {
    stringListaIDs = lista;
  }
}

void verificarDigital() {
  if (exibindoMensagemTemp) return;

  if (finger.getImage() != FINGERPRINT_OK) return;

  if (finger.image2Tz() != FINGERPRINT_OK) {
    exibirMensagemTemporaria("Erro: Imagem ruim. Limpe o leitor.");
    acenderLed(LED_VERMELHO_PIN);
    return;
  }

  if (finger.fingerSearch() != FINGERPRINT_OK) {
    exibirMensagemTemporaria("Acesso Negado! Digital não cadastrada.");
    acenderLed(LED_VERMELHO_PIN);
    return;
  }

  String nomeEncontrado = nomesAtivos[finger.fingerID];
  exibirMensagemTemporaria("ACESSO PERMITIDO! Bem-vindo(a), " + nomeEncontrado);
  acenderLed(LED_VERDE_PIN);
}

void cadastrarDigital(int id, String nome) {
  mensagemStatus = "PASSO 1: Coloque o dedo no sensor.";
  int p = -1;
  unsigned long tempoInicio = millis();

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    server.handleClient();
    if (millis() - tempoInicio > 15000) {
      exibirMensagemTemporaria("Tempo esgotado. Cadastro Cancelado.");
      return;
    }
  }

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    exibirMensagemTemporaria("Erro na conversão. Cancelado.");
    return;
  }

  mensagemStatus = "PASSO 2: Retire o dedo...";
  esperarSemTravar(2000);

  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
    server.handleClient();
  }

  mensagemStatus = "PASSO 3: Coloque o MESMO dedo novamente.";
  p = -1;
  tempoInicio = millis();

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    server.handleClient();
    if (millis() - tempoInicio > 15000) {
      exibirMensagemTemporaria("Tempo esgotado. Cadastro Cancelado.");
      return;
    }
  }

  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    exibirMensagemTemporaria("Erro na 2ª leitura. Cancelado.");
    return;
  }

  if (finger.createModel() != FINGERPRINT_OK) {
    exibirMensagemTemporaria("Erro: As digitais não conferem.");
    return;
  }

  if (finger.storeModel(id) != FINGERPRINT_OK) {
    exibirMensagemTemporaria("Erro ao salvar na memória.");
    return;
  }

  idsAtivos[id] = true;
  nomesAtivos[id] = nome;
  memoria.putString(String(id).c_str(), nome);

  atualizarTextoListaIDs();

  exibirMensagemTemporaria("SUCESSO! " + nome + " cadastrado(a).");
  acenderLed(LED_VERDE_PIN);
}

void removerDigital(int id) {
  uint8_t p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    idsAtivos[id] = false;
    nomesAtivos[id] = "";
    memoria.remove(String(id).c_str());

    atualizarTextoListaIDs();
    exibirMensagemTemporaria("SUCESSO: ID " + String(id) + " removido!");
    acenderLed(LED_VERDE_PIN);
  } else {
    exibirMensagemTemporaria("Erro ao apagar: ID não existe.");
    acenderLed(LED_VERMELHO_PIN);
  }
}
