#include <ESP8266WiFi.h>
#include <Espalexa.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>             
#include <EEPROM.h>

#define EEPROM_SIZE 2

char static_ip[16] = "192.168.0.218";         
char static_gw[16] = "192.168.0.1";
char static_sn[16] = "255.255.255.0";
bool shouldSaveConfig = false;  

void relay1(uint8_t brightness);
void relay2(uint8_t brightness);
void colorLightChanged(uint8_t brightness, uint32_t rgb);

void saveConfigCallback ()              
{
  shouldSaveConfig = true;
}


int data=255;
int r,g,b;
int rele1State;
int rele2State ;

#define REDPIN   13
#define GREENPIN 12
#define BLUEPIN  15

int bt = 4;
int btBoot = 0;
boolean buttonPress = false;   

Espalexa espalexa;

void setup() {

  Serial.begin(115200);
  
  EEPROM.begin(EEPROM_SIZE);
  rele1State = EEPROM.read(0);
  rele2State = EEPROM.read(1);
  
  pinMode(5, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(REDPIN,   OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN,  OUTPUT);
  pinMode(bt, INPUT_PULLUP);
  pinMode(btBoot, INPUT_PULLUP);

  digitalWrite(5, rele1State);
  digitalWrite(14, rele2State);

  if (SPIFFS.begin())
  {
  
    if (SPIFFS.exists("/config.json"))                      // carregar arquivo caso exista
    {
      
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);       
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success())
        {
          
          if (json["ip"])
          {
            
            strcpy(static_ip, json["ip"]);
            strcpy(static_gw, json["gateway"]);
            strcpy(static_sn, json["subnet"]);
           
          }
          
        }
        
      }
    }
  }
  
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);      //salva a configuração recebida
  IPAddress _ip, _gw, _sn;                                    //salvar IP
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);
  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);

  // wifiManager.resetSettings();                               //reset para apagar senha wifi

  wifiManager.setMinimumSignalQuality();                        //confiura qualidade minima de sinal do wifi padrão 8%

  
  if (!wifiManager.autoConnect("AlexandreDev-Device-Quarto", "12345678")) //verififcar se o wifi se conectou
  {
    delay(3000);
    ESP.reset();                                                //reinicia o ESP caso não conecte em nenhum rede
    delay(5000);
  }
                      
  if (shouldSaveConfig)                                         //salva as configuração recebidas
  {
    
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["ip"] = WiFi.localIP().toString();
    json["gateway"] = WiFi.gatewayIP().toString();
    json["subnet"] = WiFi.subnetMask().toString();
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
     // verifica se ouve erro na leitura dos arquivos
    }
    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();    //fechar e salvar a configuração
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Definir os dispositivos
    espalexa.addDevice("Rele 1", relay1);
    espalexa.addDevice("Rele 2", relay2); 
    espalexa.addDevice("Fita RGB", colorLightChanged);
    
    espalexa.begin();
  } else {
    Serial.println("Failed to connect to WiFi");
  }

}

void loop() {
  WiFiManager wifiManager;
  espalexa.loop();
  delay(1);
  
   if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Restarting...");
    ESP.restart();
  }

  digitalRead(btBoot) == HIGH;
  digitalRead(bt) == HIGH;

  if(digitalRead(btBoot) == LOW){
    delay(50);
    digitalWrite(14, !digitalRead(14));
    digitalRead(btBoot) == HIGH;
    delay(2000);
    if(digitalRead(btBoot) == LOW){
      ESP.reset();
    }
    
    }
  
   if(digitalRead(bt) == LOW && buttonPress == false){
    delay(50);
      if(digitalRead(bt) == LOW){
        digitalWrite(5, !digitalRead(5));
        buttonPress = true;
        delay(1000);
        buttonPress = false;
        }
      }

      if (digitalRead(bt) == LOW)                           // Se reset foi pressionado
     {
         delay(5000);                                              // Aguarda 3 minutos segundos
         if (digitalRead(bt) == LOW)                         // Se reset continua pressionado
    {
     
      delay(2000);
      wifiManager.resetSettings();    
      delay(8000);
      ESP.reset();                       
    }      
    }
}

void relay1(uint8_t brightness) {
  digitalWrite(5, !digitalRead(5));
  EEPROM.write(0, digitalRead(5));
  EEPROM.commit();
}

void relay2(uint8_t brightness) {
  digitalWrite(14, !digitalRead(14));
  EEPROM.write(1, digitalRead(14));
  EEPROM.commit();
}

void colorLightChanged(uint8_t brightness, uint32_t rgb) {
  //do what you need to do here, for example control RGB LED strip
  Serial.print("Brightness: ");
  Serial.print(brightness);
  Serial.print(", Red: ");
  Serial.print((rgb >> 16) & 0xFF); //get red component
  Serial.print(", Green: ");
  Serial.print((rgb >>  8) & 0xFF); //get green
  Serial.print(", Blue: ");
  Serial.println(rgb & 0xFF); //get blue
float hell = brightness / 255.0;
r=((rgb >> 16) & 0xFF)*hell;
g=((rgb >>  8) & 0xFF)*hell;
b=(rgb & 0xFF)*hell;

    uint8_t red = (rgb >> 16) & 0xFF;
    uint8_t green = (rgb >> 8) & 0xFF;
    uint8_t blue = rgb & 0xFF;

    // Ajusta o brilho de cada cor
    red = (red * brightness) / 255;
    green = (green * brightness) / 255;
    blue = (blue * brightness) / 255;

    // Define as cores nos pinos correspondentes
    analogWrite(REDPIN, red);
    analogWrite(GREENPIN, green);
    analogWrite(BLUEPIN, blue);
//static1(r, g, b,data);
}
