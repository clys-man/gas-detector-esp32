#include <WiFi.h>
#include <PubSubClient.h>
#include <song.h>

#define pinoAnalogico 33
#define pinoDigital 32
#define pinoLed 23
#define pinoBuzzer 5

/* CONFIGURAÇÕES DO WIFI */
const char *ssid = "TESTE";            // Nome da rede WiFi
const char *password = "1234567890ca"; // Senha da rede WiFi

/* CONFIGURAÇÕES DO MQTT*/
const char *mqttServer = "broker.hivemq.com"; // Endereço do Broker MQTT
const int mqttPort = 1883;                    // Porta TCP do Broker MQTT
const char *mqttUser = "";                    // Usuário MQTT
const char *mqttPassword = "";                // Senha MQTT

WiFiClient espClient;               // Cliente de Rede WiFi
PubSubClient clientMqtt(espClient); // Cria uma instancia de um cliente MQTT

song buzzer(pinoBuzzer);

/* A função de callback é chamada quando uma mensagem chega, devido a assinatura de um tópico.
Ou seja, toda vez que chegar uma mensagem em um tópico inscrito por este cliente, a função
callback será chamada. */
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Uma mensagem chegou no tópico: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println(" -----------------------");
}

void setup()
{
  Serial.begin(9600); // Configura a porta serial

  pinMode(pinoAnalogico, INPUT);
  pinMode(pinoDigital, INPUT);
  pinMode(pinoLed, OUTPUT);

  WiFi.begin(ssid, password); // Configura o WiFi

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("Tentando se conectar na rede: ");
    Serial.println(ssid);
  }

  Serial.println("Conectado na Rede WiFi.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  clientMqtt.setServer(mqttServer, mqttPort);
  clientMqtt.setCallback(callback);

  /* Verifica se o cliente está conectado ao Broker */
  while (!clientMqtt.connected())
  {

    Serial.println("Conectando ao Broker MQTT...");

    String clientId = "ESP32Client_" + String(random(0xffff), HEX);
    Serial.println("clientId = " + clientId);

    /* Conecta o cliente ao Broker MQTT.
       Configurações de credenciais e Last Will podem ser configuradas aqui*/
    if (clientMqtt.connect(clientId.c_str(), mqttUser, mqttPassword))
    {

      Serial.println("O cliente " + clientId + " foi conectado com sucesso");
    }
    else
    {

      // Estado do cliente MQTT. Quando a conexão falhar pode ser usado para obter informações sobre a falha
      int clientState = clientMqtt.state();

      Serial.print("Falha ao se conectar. ");
      Serial.println("Estado do cliente: " + (String)clientState);

      delay(2000);
    }
  }

  Serial.print("Tentando enviar a mensagem");

  clientMqtt.publish("unifor/cps/gas", "Hello from ESP32");
}

void enviaAlerta()
{
  digitalWrite(pinoLed, HIGH);
  delay(1000);
  digitalWrite(pinoLed, LOW);
  delay(1000);
}

void loop()
{
  /* loop: função que deve ser chamada regularmente para permitir que o cliente
  processe as mensagens recebidas e mantenha sua conexão com o Broker.*/
  clientMqtt.loop();

  // Lê o pino analógico do sensor
  int leitura_analogica = analogRead(pinoAnalogico);
  // Lê o pino digital do sensor
  int leitura_digital = digitalRead(pinoDigital);

  // Apresenta a leitura analógica no monitor serial
  Serial.print("leitura do sensor: ");
  Serial.println(leitura_analogica);

  // Verifica se existe gás tóxico presente no ambiente
  if (leitura_analogica > 400)
  {
    Serial.println("Gás tóxico detectado!");
    enviaAlerta();
  }
  else
  {
    Serial.println("Gás tóxico não detectado");
    digitalWrite(pinoLed, LOW);
  }

  String leitura = String(leitura_analogica);

  clientMqtt.publish("unifor/cps/gas", leitura.c_str());

  delay(100);
}
