
#define RXp2 16
#define TXp2 17
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
}
void loop() {
    //delay(1000);
    //Serial.println(Serial2.readString());
    if(Serial2.available() > 0)
    {
        /*size_t len = Serial2.available();
        uint8_t sbuf[len];
        Serial2.readBytes(sbuf, len);
        Serial.println(sbuf);*/
        Serial.println("Message Received: ");
        Serial.println(Serial2.readString());
     }
     
    //Serial.println();
}
