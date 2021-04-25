#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

//VERSION 18/04/2001

#include <ESP8266WiFi.h>

#include "config.h"

int Nb=0;
int Nb2=0;

float Temps1;
int total_step_left=1;
int total_step_right=1;

int right_state=0;
int left_state=0;

int right_move=0;
int left_move=0;

unsigned long FinalStep;

WiFiClient espClient;
PubSubClient client(espClient);

#define MoteurGauche 5//D1
#define MoteurDroit 4 //D2
#define PinSensGauche 0 //D3
#define PinSensDroit 2 //D4
#define Bouton 14//D5
#define Bouton2 16 //D0

long lastMsg = millis();

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  //Serial.println();
  //Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<256> doc;
  deserializeJson(doc, payload, length);
  String str = doc["idx"];

  Serial.println(str);

  if (str == "61"){
    int f = doc["svalue1"];
    Serial.println(f);
    right_get_step(f);
    left_get_step(f);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), USERNAME, PASSWORD)) {
      Serial.println("connected");
      client.subscribe("homeassistant/cover/StoreMQTT/set_position");
      client.subscribe("homeassistant/cover/StoreMQTT/set");
      
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishSetup() {
  DynamicJsonDocument doc(1048);
  doc["name"] = "Store MQTT";
  doc["command_topic"] = "homeassistant/cover/StoreMQTT/set";
  doc["position_topic"] = "homeassistant/cover/StoreMQTT/position";
  doc["set_position_topic"] = "homeassistant/cover/StoreMQTT/set_position";
  doc["position_open"] = 100;
  doc["position_closed"] = 0;

  char string[1048];
  serializeJson(doc, &string, 1048);
   
  client.publish("homeassistant/cover/StoreMQTT/config", string, true);
}

void setup() {
  pinMode(MoteurGauche, OUTPUT);
  pinMode(MoteurDroit, OUTPUT);
  pinMode(PinSensDroit, OUTPUT);
  pinMode(PinSensGauche, OUTPUT);
  pinMode(Bouton2, INPUT);
  pinMode(Bouton, INPUT);
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  
  Serial.println("End Setup");
  ArduinoOTA.begin();
  publishSetup();
}

void SetupGauche(){
  
  delay(500);
  total_step_left = 0;
  int Statut = 1;
  Serial.print(Statut);
  digitalWrite(PinSensGauche, LOW);
  Serial.println("Down");
  while (Statut == 1){
    
    digitalWrite(MoteurGauche, HIGH);
    delayMicroseconds(1000);
    digitalWrite(MoteurGauche, LOW);
    delayMicroseconds(1000);
    yield();
    
    if (digitalRead(Bouton)){
    Statut = 2;
    Serial.println("Up dans 2 sec");
    delay(2000);
    digitalWrite(PinSensGauche, HIGH);
    
    }
  
  }
  while ( Statut == 2){
    
    digitalWrite(MoteurGauche, HIGH);
    delayMicroseconds(1000);
    digitalWrite(MoteurGauche, LOW);
    delayMicroseconds(1000);
    total_step_left=1+total_step_left;
    
    yield();
    if(digitalRead(Bouton)){
      delay(1000);
      Statut = 3;
      left_state = total_step_left;
  }
  

  }
  Serial.println("Configuration gauche finie");
  Serial.println(total_step_left);
  
}

void SetupDroit(){
  
  delay(500);
  total_step_right = 0;
  int Statut = 1;
  Serial.print(Statut);
  Serial.println("Down");

  digitalWrite(PinSensDroit, HIGH);
  while (Statut == 1){
    
    
    digitalWrite(MoteurDroit, HIGH);
    delayMicroseconds(1000);
    digitalWrite(MoteurDroit, LOW);
    delayMicroseconds(1000);
    
    yield();
    if (digitalRead(Bouton2)){
    Statut = 2;
    Serial.println("Up dans 2 sec");
    delay(2000);
    digitalWrite(PinSensDroit, LOW);
    }
  
  }
  while ( Statut == 2){
    
    digitalWrite(MoteurDroit, HIGH);
    delayMicroseconds(1000);
    digitalWrite(MoteurDroit, LOW);
    delayMicroseconds(1000);
    total_step_right=1+total_step_right;
    
    yield();
    if(digitalRead(Bouton2)){
      delay(1000);
      Statut = 3;
      right_state = total_step_right;
  }
  

  }
  Serial.println("Configuration finie");
  Serial.println(total_step_right);
  
  
}


void loop() {
  ArduinoOTA.handle();
  yield();
  if (left_move != 0 && right_move !=0){
    digitalWrite(MoteurDroit, HIGH);
    digitalWrite(MoteurGauche, HIGH);
    delayMicroseconds(1000);
    digitalWrite(MoteurDroit, LOW);
    digitalWrite(MoteurGauche, LOW);
    delayMicroseconds(1000); 
}
else if (left_move !=0){
   
    digitalWrite(MoteurGauche, HIGH);
    delayMicroseconds(1000);
    digitalWrite(MoteurGauche, LOW);
    delayMicroseconds(1000);       
    
  
}
else if (right_move!=0){
    digitalWrite(MoteurDroit, HIGH);
    delayMicroseconds(1000);
    digitalWrite(MoteurDroit, LOW);
    delayMicroseconds(1000);
}
  leftMove();
  rightMove();
  
  if (digitalRead(Bouton)&& Nb==0){
    
    
    Temps1 = millis();
    Nb = 1;
    
    delay(100);
  }
  if (digitalRead(Bouton2)&& Nb2==0){
    
    
    Temps1 = millis();
    Nb2 = 1;
    
    delay(100);
  }
  if ((digitalRead(Bouton)== 0) && Nb==1){
    float Temps2 = millis();
    float TempsT= (Temps2 - Temps1)/1000 ;
    Nb=0;

    delay(200);
    if (TempsT >= 2){
      SetupGauche();
    }
    else if (TempsT < 2){
      
      delay(200);
    }
  }
    if ((digitalRead(Bouton2)== 0) && Nb2==1){
    float Temps2 = millis();
    float TempsT= (Temps2 - Temps1)/1000 ;
    Nb2=0;

    delay(200);
    if (TempsT >= 2){
      SetupDroit();
    }

    else if (TempsT < 2){
      
      delay(200);
    }
  }
if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  
}




void right_get_step(int high){
  if (high >100){
    high = 100;
  }
  else if (high<0){
    high=0;
  }
  high = (float(high)*float(total_step_right))/100; 
  int move_diff =  high - right_state;
  

  right_move = int(move_diff);
  //Serial.print("right_move : ");
  //Serial.println(right_move);
  //right_state = high;
}


void left_get_step(int high){
    if (high >100){
    high = 100;
  }
  else if (high<0){
    high=0;
  }
  high = (float(high)*float(total_step_left))/100; 
  int move_diff =  high - left_state;


  left_move = int(move_diff);
  //left_state = high;
}



void leftMove(){
  
  if (left_move>0){
      left_move--;
      left_state++;
      digitalWrite(PinSensGauche,HIGH);
  }
  else if (left_move<0){
      digitalWrite(PinSensGauche,LOW);
      left_state--;
      left_move++;      
  }
 
}
void rightMove(){
  if (right_move>0){
      //Serial.println(right_move);
      right_move--;
      right_state++;
      digitalWrite(PinSensDroit,LOW);
  }
  else if (right_move<0){ 
      //Serial.println(right_move);
      digitalWrite(PinSensDroit,HIGH);
      right_move++;
      right_state--;
      
  } 
}
