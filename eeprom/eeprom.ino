#include <EEPROM.h>
void setup()
{

  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor

  for (int i = 0; i < 96; ++i) 
  {
      EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

void loop()
{
  ;
}
