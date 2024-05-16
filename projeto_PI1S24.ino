
// Bibliotecas MQTT
#include "ESP8266WiFi.h"
#include <PubSubClient.h>

//Bibliotecas Pesagem
#include <HX711.h>

#include <string.h>

// DEFINIÇÕES DE PINOS
#define pinDT D6
#define pinSCK D7

//Parametros de conexão
const char* ssid = "JONATHAS"; // REDE
const char* password = "F94A2D5B"; // SENHA


// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";  //Host do broket
const char *topic = "JONATHAS/TESTE";            //Topico a ser subscrito e publicado
const char *mqtt_username = "";         //Usuario
const char *mqtt_password = "";         //Senha
const int mqtt_port = 1883;             //Porta

//Variáveis
bool mqttStatus = 0;
bool publicacaoConcluida = false;


HX711 scale;
float medida = 0;

//Objetos
WiFiClient espClient;
PubSubClient client(espClient);

//Prototipos
bool connectMQTT();
void callback(char *topic, byte * payload, unsigned int length);

void connect_test();
void send_mass();
//CONTROLADORES EXTERNOS


void setup(void)
{
  Serial.begin(9600);

  // Conectar
  WiFi.begin(ssid, password);

  //Aguardando conexão
  Serial.println();
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //Envia IP através da UART
  Serial.println(WiFi.localIP());

  mqttStatus =  connectMQTT();

  //Depois de conectado, vamos inicializar a balança
  scale.begin(pinDT, pinSCK); // CONFIGURANDO OS PINOS DA BALANÇA
  scale.set_scale(953.036); // LIMPANDO O VALOR DA ESCALA

  delay(2000);
  scale.tare(); // ZERANDO A BALANÇA PARA DESCONSIDERAR A MASSA DA ESTRUTURA

  Serial.println("Balança Zerada");

}

void loop() {

  if ( mqttStatus){
    
    client.loop();    


  }


}



bool connectMQTT() {
  byte tentativa = 0;
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  do {
    String client_id = "ESP8266-JONATHAS";
    client_id += String(WiFi.macAddress());

    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Exito na conexão:");
      Serial.printf("Cliente %s conectado ao broker\n", client_id.c_str());
    } else {
      Serial.print("Falha ao conectar: ");
      Serial.print(client.state());
      Serial.println();
      Serial.print("Tentativa: ");
      Serial.println(tentativa);
      delay(2000);
    }
    tentativa++;
  } while (!client.connected() && tentativa < 5);

  if (tentativa < 5) {
    // publish and subscribe   
    client.publish(topic, "{teste123,113007042022}"); 
    client.subscribe(topic);
    return 1;
  } else {
    Serial.println("Não conectado");    
    return 0;
  }

}

void callback(char *topic, byte * payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  char firstLetter = payload[0];
  //Armazena a primeira letra e, com base nela, executa um comando
  //Vamos comparar apenas a primeira letra para facilitar e evitar erros de comparação de strings

  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");

  //TOMANDO UMA AÇÃO COM BASE NO COMANDO RECEBIDO VIA BROKER
  Serial.println((char) firstLetter);
  
  /*
  Se o comando for:
  C ->  CONNECT_TEST - teste de conexão
  S -> SEND_MASS - solicita o envio da medição
  */

  if(firstLetter=='C'){
    Serial.println("comando a - teste de publicacao");
    connect_test();
  } else if (firstLetter=='S'){
    send_mass();
  }

}

void connect_test(){
  if (client.connected()){
    client.publish(topic, "conectado");
  } else {
    Serial.println("erro no envio da mensagem!");
  }
}

void send_mass(){

  
    if (client.connected()){
    
    //Fazendo uma média de 3 medidas
    medida = scale.get_units(5);

    //Mostrando no serial, com 3 casas decimais
    Serial.println(medida, 3);

    //Armazendo um float dentro de um array de char
    char buffer[20];
    sprintf(buffer, "%f", medida);

    client.publish(topic, buffer);
  } else {
    Serial.println("erro no envio da mensagem!");
  }
}

