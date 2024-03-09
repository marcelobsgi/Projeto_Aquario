
// Projeto Aquario

#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "DHTesp.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimeLib.h>


unsigned long lastFeedTime = 0;  // Variável para armazenar a última vez que o servo foi acionado
const int feedInterval = 86400000;  // Intervalo de alimentação em milissegundos (um dia)

DHTesp dht;

Servo servo;
const int led = 2;
int green = 16; // Pino do LED D0  //Iniciar alimentação
int blue = 4; // Pino do LED D2    //Comida liberada
int red = 13; // Pino do LED D7
const char* ssid = "Rede Wifi";
const char* password = "senha wifi";

// Defina o pino do sensor DS18B20
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);


ESP8266WebServer server(80);

void handleRoot() {
  digitalWrite(led, 1);
  float tempAgua = sensor.getTempCByIndex(0);  // Obter a temperatura da água
  String umidade = String(dht.getHumidity());
  String temperaturaAmbiente = String(dht.getTemperature());
  String temperaturaAgua = String(tempAgua);  // Converter a temperatura da água para String

  String textoHTML;

 
  textoHTML = "<b>Projeto Aquario ESP8266</b>";
  textoHTML += "<p>Umidade do ambiente</p>" + umidade + " %";
  textoHTML += "<p>Temperatura do ambiente</p>" + temperaturaAmbiente + " C";
  textoHTML += "<p>Temperatura da agua</p>" + temperaturaAgua + " C";

  //textoHTML += analogRead(2);
   
  server.send(200, "text/html", textoHTML);

  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);

}


//______________________________________________________________//


void setup() {
  servo.attach(5); // D1
  servo.write(180);    // Inicia o servo com a saida na posição lateral
  dht.setup(14, DHTesp::DHT11); // D5
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");


  pinMode(green, OUTPUT); // Configura o pino do LED
  pinMode(blue, OUTPUT);
  pinMode(red, OUTPUT);
  delay(200);

  //Server ---início do código
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  //dht11
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("192.168.0.135");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // Iniciar o sensor DS18B20
  sensor.begin();

}




void loop() {
  //Server
  server.handleClient();
  delay(1500);
  float u = dht.getHumidity();
  float t = dht.getTemperature();

  Serial.print("Umidade (%): ");
  Serial.print(u, 1);
  Serial.print("\t");
  Serial.print("Temperatura (C): ");
  Serial.print(t, 1);
  Serial.print("\t");
  Serial.println();


  // Solicitar leitura da temperatura água
  sensor.requestTemperatures();
  // Ler a temperatura em Celsius para o dispositivo 1 no índice 0
  float temperatura = sensor.getTempCByIndex(0);

  // Imprimir a temperatura no monitor serial
  Serial.print("Temperatura da agua: ");
  Serial.print(temperatura);
  Serial.println(" graus Celsius");

  // Verificar se a temperatura está abaixo de 20ºC
  if (temperatura < 20.0) {
    // Ligar o LED
    digitalWrite(red, HIGH);
  } else {
    // Desligar o LED
    digitalWrite(red, LOW);
  }


  // Abastecer reservatório da comida
  // Quando desligado ficará na posição do furo lateralizada. 2giros
  // Condicionnal
    if (millis() - lastFeedTime >= feedInterval) {
         // Acender o LED para iniciar 
        digitalWrite(green, HIGH);
        delay(250);
        digitalWrite(green, LOW);
        delay(250);
        digitalWrite(green, HIGH);
        delay(250);
        digitalWrite(green, LOW);
        delay(250);
        
        // Aciona o servo para alimentar
        servo.write(180);
        delay(400);
        servo.write(0);
        delay(400);
        servo.write(180);
        delay(400);
        servo.write(0);
        delay(400);
        servo.write(180);

        // Atualiza o tempo da última alimentação
        lastFeedTime = millis();
            
        // Liberar comida
        //// Quando ligado ficará na posição do furo para baixo
      
        delay(400);
        //Acender o Led de Ok
        digitalWrite(blue, HIGH);
        delay(1000);
        digitalWrite(blue, LOW);

      }


  // Final - Aguardar!
  Serial.println("Modo stand by");

  delay(9000);

}
