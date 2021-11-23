#include <WiFi.h>
#include <PubSubClient.h>
#define RXp2 16
#define TXp2 17
// WiFi
const char *ssid = "Albot+2.4"; // Enter your WiFi name
const char *password = "Password@4321";  // Enter WiFi password

long count = 0;
char temp[50];
String temp_str;
//const char *serimsg = Serial2.readString().c_str();

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "esp32/test";
const char *topic1 = "esp32/test1";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() 
{
    // Set software serial baud to 115200;
    Serial.begin(115200);
    Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
    // connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }

    Serial.println("Connected to the WiFi network");
    //connecting to a mqtt broker
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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) 
    {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) 
        {
            Serial.println("Public emqx mqtt broker connected");
            Serial.println("connected");
            // Once connected, publish an announcement...
            temp_str = String(count);
            temp_str.toCharArray(temp, temp_str.length() + 1);
            client.publish(topic, temp);
            //client.publish(topic,"hello world");
            // ... and resubscribe
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
    count++;
    if (!client.connected()) 
    { 
      Serial.println("reconnecting");
       
      reconnect();
    }
    temp_str = String(count);
    temp_str.toCharArray(temp, temp_str.length() + 1);
    client.publish(topic, temp);
    delay(1000);        
    client.loop();
    
    
/*  
    //const char *serimsg = Serial2.readString().c_str();
    const char *serimsg = "Yo! Bro who Got You Smilin' like that!";
    client.publish(topic, serimsg);
    client.subscribe(topic);
    delay(1000);
    client.loop();
*/


}
