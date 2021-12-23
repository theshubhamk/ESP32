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
#include "driver/uart.h" //uart

WebServer server(80);

#include "WiFiManager.h"

//uart
#define NUMERO_PORTA_SERIALE UART_NUM_2
#define BUF_SIZE (1024 * 2)
#define RD_BUF_SIZE (1024)
static QueueHandle_t uart2_queue;
 
static const char * TAG = "";                  
 
#define U2RXD 16
#define U2TXD 17
 
uint8_t rxbuf[256];     
uint16_t rx_fifo_len;    

//Timer
unsigned long myTime;

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "EcmoParamTopic";
const char *topic2 = "EcmoParamTopic2";
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
  //Configuro la porta Serial2 (tutti i parametri hanno anche un get per effettuare controlli)
    uart_config_t Configurazione_UART2 = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(NUMERO_PORTA_SERIALE, &Configurazione_UART2);
  //Firma: void esp_log_level_set(const char *tag, esp_log_level_tlevel)
    esp_log_level_set(TAG, ESP_LOG_INFO);
 
 
   
    //Firma: esp_err_tuart_set_pin(uart_port_tuart_num, int tx_io_num, int rx_io_num, int rts_io_num, int cts_io_num)
    uart_set_pin(NUMERO_PORTA_SERIALE, U2TXD, U2RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
 
 
    //Firma: uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));
    //       uart_driver_install(Numero_porta, RXD_BUFFER, TXD_Buffer, event queue handle and size, flags to allocate an interrupt)
    uart_driver_install(NUMERO_PORTA_SERIALE, BUF_SIZE, BUF_SIZE, 20, &uart2_queue, 0);
 
 
    //Create a task to handler UART event from ISR
    xTaskCreate(UART_ISR_ROUTINE, "UART_ISR_ROUTINE", 2048, NULL, 12, NULL);
 

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


//ISR
static void UART_ISR_ROUTINE(void *pvParameters)
{
    uart_event_t event;
    size_t buffered_size;
    bool exit_condition = false;
   
    //Infinite loop to run main bulk of task
    while (1) {
     
        //Loop will continually block (i.e. wait) on event messages from the event queue
        if(xQueueReceive(uart2_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
         
            //Handle received event
            if (event.type == UART_DATA) {
 
                uint8_t UART2_data[128];
                char buff2[127];
                char buff3[127];
                char buff4[127];
                int flag = 0;
                int UART2_data_length = 0;
                ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM_2, (size_t*)&UART2_data_length));
                UART2_data_length = uart_read_bytes(UART_NUM_2, UART2_data, UART2_data_length, 100);
             
                Serial.print("LEN= ");Serial.println(UART2_data_length);
 
                Serial.print("DATA= ");
                for(byte i=0; i<UART2_data_length;i++) Serial.print((char)UART2_data[i]);
                Serial.println("");
                
                //for(byte i=0; i<UART2_data_length;i++) client.publish(topic, (char)UART2_data[i]);
                //for(byte i=0,j=1; i<UART2_data_length;i++,j++) buff2[i] = UART2_data[j];
                for(byte i=0,j=1; i<UART2_data_length && buff2[i] != '\n' ;i++,j++) buff2[i] = UART2_data[j];
                if(UART2_data[0] == 'R')
                {
                  for(byte i=0,j=0; i<UART2_data_length-1;i++,j++)
                  {
                      if(buff3[i] != buff2[j])
                      {
                          client.publish(topic, (const char*)buff2);
                          for(byte i=0,j=0; i<UART2_data_length-1;i++,j++) buff3[i] = buff2[j];
                      }
                  }
                  
                  //for(byte i=0,j=1; i<UART2_data_length;i++,j++) buff2[i] = UART2_data[j];
                  //client.publish(topic, (const char*)UART2_data);
                  //client.publish(topic, (const char*)buff2);
                  
                }
                if(UART2_data[0] == 'F')
                {
                  for(byte i=0,j=0; i<UART2_data_length-1;i++,j++)
                  {
                      if(buff4[i] != buff2[j])
                      {
                          client.publish(topic2, (const char*)buff2);
                          for(byte i=0,j=0; i<UART2_data_length-1;i++,j++) buff4[i] = buff2[j];
                      }
                  }
                  
                  //for(byte i=0,j=1; i<UART2_data_length;i++,j++) buff2[i] = UART2_data[j];
                  //client.publish(topic, (const char*)UART2_data);
                  //client.publish(topic, (const char*)buff2);
                  
                }
                memset(buff2, 0, sizeof(buff2));
             
            }
           
            //Handle frame error event
            else if (event.type == UART_FRAME_ERR) {
                //TODO...
            }
           
            //Keep adding else if statements for each UART event you want to support
            //else if (event.type == OTHER EVENT) {
                //TODO...
            //}
           
           
            //Final else statement to act as a default case
            else {
                //TODO...
            }      
        }
       
        //If you want to break out of the loop due to certain conditions, set exit condition to true
        if (exit_condition) {
            break;
        }
    }
   
    //Out side of loop now. Task needs to clean up and self terminate before returning
    vTaskDelete(NULL);
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
  
  
  
  //const char *serimsg = "y";
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
