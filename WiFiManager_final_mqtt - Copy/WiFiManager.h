/*
 * Function to handle unknown URLs
 */
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

/*
 * Function for writing WiFi creds to EEPROM
 * Returns: true if save successful, false if unsuccessful
 */
bool writeToMemory(String ssid, String pass){

  //erase FIRST
  for(int i=0;i<512;i++){
    EEPROM.writeByte(i,0);
  }
  EEPROM.commit();
  
  char buff1[30];
  char buff2[30];
  ssid.toCharArray(buff1,30);
  pass.toCharArray(buff2,30); 
  EEPROM.writeString(100,buff1);
  EEPROM.writeString(200,buff2);
  delay(100);
  String s = EEPROM.readString(100);
  String p = EEPROM.readString(200);
  //#if DEBUG
  Serial.print("Stored SSID, password, are: ");
  Serial.print(s);
  Serial.print(" / ");
  Serial.print(p);
  //#endif
  if(ssid == s && pass == p){
    return true;  
  }else{
    return false;
  }
}


/*
 * Function for handling form
 */
void handleSubmit(){
  String response_success="<h1>Success</h1>";
  response_success +="<h2>Device will restart in 3 seconds</h2>";

  String response_error="<h1>Error</h1>";
  response_error +="<h2><a href='/'>Go back</a>to try again";
  
  if(writeToMemory(String(server.arg("ssid")),String(server.arg("password")))){
     server.send(200, "text/html", response_success);
     EEPROM.commit();
     delay(3000);
     ESP.restart();
  }else{
     server.send(200, "text/html", response_error);
  }
}

/*
 * Function for home page
 */
void handleRoot() {
  if (server.hasArg("ssid")&& server.hasArg("password")) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", INDEX_HTML);
  }
}

/*
 * Function for loading form
 * Returns: false if no WiFi creds in EEPROM
 */
bool loadWIFICredsForm(){
  String s = EEPROM.readString(100);
  String p = EEPROM.readString(200);
  
  const char* ssid     = "ALBOT_ECMO(192.168.4.1)";
  const char* password = "12345678";

  Serial.println("Setting Access Point...");
  
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  
  Serial.println("HTTP server started");
 
  while(s.length() <= 0 && p.length() <= 0){
    server.handleClient();
    delay(100);
  }
  
  return false;
}

//ALWAYS ON WEBSERV
void loadWIFICredsForm2(){
  String s = EEPROM.readString(100);
  String p = EEPROM.readString(200);
 
  Serial.println("Starting Web server...");
  
  //WiFi.softAP(ssid, password);
   WiFi.begin(s.c_str(), p.c_str());

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    if(WiFi.status() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("WiFi connected.");
    }
  
  
  IPAddress IP = WiFi.localIP();
  
  Serial.print("LOCAL IP address: ");
  Serial.println(IP);
  
  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  
  Serial.println("HTTP server started");
 
  while(s.length() <= 0 && p.length() <= 0){
    server.handleClient();
    delay(100);
    Serial.print("memory is blank");
  }
  
  //return false;
}

/*
 * Function checking WiFi creds in memory 
 * Returns: true if not empty, false if empty
 */
bool CheckWIFICreds(){
  Serial.println("Checking WIFI credentials");
  String s = EEPROM.readString(100);
  String p = EEPROM.readString(200);
  //#if DEBUG
  Serial.print("Found credentials: ");
  Serial.print(s);
  Serial.print("/");
  Serial.print(p);
  delay(5000);
  //#endif
  if(s.length() > 0 && p.length() > 0){
    return true;
  }else{
    return false;
  }
}
