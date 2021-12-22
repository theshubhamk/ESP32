/*
 ***********************************************************************************************************************
 * The firmware program adds following features to the ESP32 board::                                                   *                 
 *    - Manage Wifi connection                                                                                         *
 *    - Communicate(RX/TX) over serial(UART) channel with other devices(Embedded hardwares like STM32 dev board etc.   *
 *    - Handle MQTT(RX/TX data in plain text format over MQTT). Helps devices TALK. e.g IoT usages.                    *                                           
 * //compiled                                                                                                      *
 * ©SHUBHAM_K_S/Albot_technologies/v3.1.1                                                                              *
 ***********************************************************************************************************************
*/

//#include "EEPROM.h"
#include "HTML.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h> //mqtt

WebServer server(80);

#include "WiFiManager.h"

#define RXp2 16 //serial
#define TXp2 17 //serial

//Timer
unsigned long myTime;

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "EcmoParamTopic";
const char *topic1 = "esp32/test1";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient; //mqtt
PubSubClient client(espClient); //mqtt

//test vars
long count = 0;
char temp[50];
String temp_str;

void setup() 
{
  pinMode(2, OUTPUT); //for LED blink on board
  pinMode(15,INPUT); //for resetting WiFi credentials
  EEPROM.begin(400);
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2); //serial

  //WIFI
  if(!CheckWIFICreds()) //check wifi credentials in EEPROM (true or false ret value)
  {
    Serial.println("No WIFI credentials stored in memory. Loading form...");
    digitalWrite(2,HIGH);
    while(loadWIFICredsForm()); //creds unavailable then start AP,server and Load HTML for user input.
  }
  loadWIFICredsForm2(); //start server and webpage for changing WIFI creds at any time.(ALWAYS ON WEB-PAGE)

  
  //MQTT //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  
    // publish and subscribe
    //client.publish(topic, "Hi EMQ X I'm ESP32 ^^");
    //const char *serimsg = Serial2.readString().c_str();
    //client.publish(topic, serimsg);
    //client.subscribe(topic);

}


void callback(char *topic1, byte *payload, unsigned int length) 
{
    Serial.print("Message arrived in topic: ");
    Serial.println(topic1);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) 
    {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

void reconnect()
{
  //loop everytime until we are connected 
  while (!client.connected()) 
  {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) 
      {
          Serial.println("Public emqx mqtt broker connected");
          //test msg to mQTT
         /* temp_str = String(count);
          temp_str.toCharArray(temp, temp_str.length() + 1);
          client.publish(topic, temp);
         */
          
          //client.publish(topic, serimsg);
          client.subscribe(topic1);
      } 
      else 
      {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
}

void loop() 
{

  //timer prnt
  myTime = millis();
  count++;
  if(digitalRead(15) == HIGH) //CHECK FOR HARDWARE RESET SIGNAL I.E PIN 15 PULLED HIGH BY JUMPER WIRE
  {
    Serial.println("Wiping WiFi credentials from memory...");
    wipeEEPROM();
    while(loadWIFICredsForm());
  }
  //LED BLINK
  digitalWrite(2,HIGH);
  delay(1000);
  digitalWrite(2,LOW);
  delay(1000);

  //MQTT
  if (!client.connected()) 
  { 
      Serial.println("reconnecting");
      reconnect();
  }
  /*
  if(Serial2.available() > 0)
  {
    const char *serimsg = Serial2.readString().c_str(); //UART RX
    Serial.println(serimsg);
    client.publish(topic,serimsg);
  }*/
  
  if(Serial2.available() > 0)
  {
      const int len = 10;
      char sbuf[len] = {NULL};
      Serial2.readBytes(sbuf, len);
      Serial.println(sbuf);
      client.publish(topic,sbuf);
  }   
  
  //const char *serimsg = "Yo!_Bro_who_Got_You_Smilin'_like_that!";
  /*
  temp_str = String(count);
  temp_str.toCharArray(temp, temp_str.length() + 1);
  client.publish(topic, temp);
  */
  
  //client.subscribe(topic1);
  //delay(1000);
  client.loop();
  
  server.handleClient();
  
  /*count++;
  Serial.println(count);
  if(count == 150)
  {
    ESP.restart();
  }
  
  if(!client.connected())
  {
    Serial.println("resetting");
    for(int j = 0; j < 10; j++)
    {
      Serial.print(".");
      delay(1000);
    }
    ESP.restart();
  }
  */
  
}

void wipeEEPROM()
{
  for(int i=0;i<400;i++){
    EEPROM.writeByte(i,0);
  }
  EEPROM.commit();
}
