#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <EEPROM.h>

#define OLED

#ifdef OLED
    #include "SSD1306Wire.h"   
    #define ___OLED___(x) x
#else
    #define ___OLED___(x) (void)0;
#endif   

#include <AsyncMqttClient.h>

#include <Arduino.h>
#include <Hash.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "index.htm.h"
#include "script.js.h"
#include "favicon.bmp.h"
#include "style.css.h"
#include "jquery-3.2.1.min.js.h"

#include "WiFi_Auth.h"


#ifdef Prod_board

#define MQTT_PUB_TEMP1 "exhaust_fan/temperature"
#define MQTT_PUB_TEMP2 "exhaust_fan/PWM100"
#define MQTT_PUB_TEMP3 "exhaust_fan/autoPWM"

#else

#define isDebugging


#define MQTT_PUB_TEMP1 "dev_exhaust_fan/temperature"
#define MQTT_PUB_TEMP2 "dev_exhaust_fan/PWM100"
#define MQTT_PUB_TEMP3 "dev_exhaust_fan/autoPWM"

#endif


#ifdef isDebugging
//    #define debugDS
    #define Serial_D Serial1
    #define ____Debug____(x) x
#else
    #define ____Debug____(x)  (void)0;
#endif
#define strickt_wifi


#define min_pwm 140
#define max_pwm 255
#define pwm__slider_step 1
#define pwm__slider_max  100

#define minimum_autoPWMvalue 140
#define maximum_autoPWMvalue 255

unsigned long DS18B20lastTime = 0;  
#define DS18B20Delay  15000
unsigned long heartbeat_timer = 0;
bool  heartbeat_state = 0;
#define heartbeat_delay 1000

#ifdef isDebugging
float temp_min = 25.00;
float temp_max = 30.00;
#else
float temp_min = 36.00;
float temp_max = 42.00;
#endif

#define ONE_WIRE_BUS D5

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

float tempSensor1;
int weightedTemperature;

#ifdef debugDS
int numberOfDevices;
DeviceAddress Device_Address;
#endif

#define output1 D6 


uint8_t sliderValue = 0;
uint8_t GUI_Update = 0;   // 1 - has update
uint8_t MQTT_Update = 0;
uint8_t PWMvalue = 0;
uint8_t autoPWM = 0;
uint8_t autoPWMvalue = 0;

const char* PARAM_INPUT = "value";
const char* PARAM_INPUT1 = "sensor_data";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

#ifdef OLED
  SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);
#endif




#define MQTT_HOST IPAddress(192, 168, 103, 99)

#define MQTT_PORT 1883

bool MQTT_connection_status = false;



AsyncMqttClient mqttClient;

void readDSTemperatureC() {  
    sensors.requestTemperatures(); // Send the command to get temperatures
    tempSensor1 = sensors.getTempCByIndex(0);
//  tempSensor1 = sensors.getTempC(T_Sensor); // Gets the values of the temperature
  

#ifdef isDebugging
    if(tempSensor1 == -127.00) {
    Serial_D.println("Failed to read from DS18B20 sensor");
  } else {
    Serial_D.print("Temperature Celsius: ");
    Serial_D.println(tempSensor1); 
  }
#endif  
}

byte read_slider_from_eeprom(void)
  {

    byte responce = 0;
    EEPROM.get(0, responce);
#ifdef isDebugging
    Serial_D.print(" EEPROM Data: ");
    Serial_D.println(responce);
#endif
    return responce;

  }

uint8_t sliderValue_to_PWM (uint8_t value)
{
  uint8_t responce;
  switch (value)
  {
  case 0:
    ____Debug____(Serial_D.println("case 0");)
    responce = 0;
    break;
  case 1 ... 99:
    responce = ((max_pwm - min_pwm)*(value/pwm__slider_max)) + min_pwm;
    ____Debug____(Serial_D.println("case 1 .. 99");)
    break;
  case 100:
    ____Debug____(Serial_D.println("case 100");)
    responce = 255;
    break;
  default:
    ____Debug____(Serial_D.println("case default");)
    responce = 0;
    break;
  }
  return responce;
}

String processor(const String& var){
  if (var == "SLIDER_LABEL"){
    sliderValue = read_slider_from_eeprom();
    ____Debug____(
      Serial_D.print(" sledrValue from EEPROM = ");
      Serial_D.println(sliderValue);
      )
    String slider_label = "FAN: "+String(sliderValue)+"&percnt;";
    return slider_label;
  }
  if (var == "SLIDERVALUE"){
    sliderValue = read_slider_from_eeprom();
#ifdef isDebugging
    Serial_D.print(" sledrValue from EEPROM = ");
    Serial_D.println(sliderValue);
#endif
    return String(sliderValue);
  }
    if(var == "SLIDERSTEP"){
    return String(pwm__slider_step); 
  }
    if(var == "SLIDERMAXVALUE"){
    return String(pwm__slider_max); 
  }
    if(var == "temp_sensor"){
    return String(tempSensor1); 
  }
  
  return String();
}

void connectToMqtt() {
  ____Debug____ (
   Serial_D.println("Connecting to MQTT...");
  )
  mqttClient.connect();
  MQTT_connection_status = true;
  
}

void onMqttConnect(bool sessionPresent) {
  ____Debug____ (
    Serial_D.println("Connected to MQTT.");
    Serial_D.print("Session present: ");
    Serial_D.println(sessionPresent);
  )

  mqttClient.subscribe(MQTT_PUB_TEMP2, 2);
  uint16_t packetIdSub = mqttClient.subscribe(MQTT_PUB_TEMP3, 2);
  ____Debug____ (
    Serial_D.print("Subscribing at QoS 2, packetId: ");
    Serial_D.println(packetIdSub);
  )
}

void onMqttPublish(uint16_t packetId) {
  ____Debug____ (
  Serial_D.print("Publish acknowledged.");
  Serial_D.print("  packetId: ");
  Serial_D.println(packetId);
  )
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  MQTT_connection_status = false;
  ____Debug____ (
  Serial_D.println("Disconnected from MQTT.");
  )
}


void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  ____Debug____ (
    Serial_D.println("Subscribe acknowledged.");
    Serial_D.print("  packetId: ");
    Serial_D.println(packetId);
    Serial_D.print("  qos: ");
    Serial_D.println(qos);
  )
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char truncated_payload[10] = "";
  strncat(truncated_payload, payload, len);
  ____Debug____ (
    Serial_D.print("Publish received, topic: ");
    Serial_D.print(topic);
    Serial_D.print(", payload: ");

    Serial_D.print(truncated_payload);
    Serial_D.print(", lenght: ");
    Serial_D.println(len);
  )

    if( (strcmp(topic,MQTT_PUB_TEMP2) == 0) && (autoPWM == 0)) // Apply 0..100 PWM value, when PWM set to manual
    {
      
        sliderValue = atoi(truncated_payload);
        EEPROM.put(0,sliderValue);
        EEPROM.commit();
        MQTT_Update = 1;
      
    }

    if( strcmp(topic,MQTT_PUB_TEMP3) == 0) // Change mode Manual (0)|Auto(1)
    {
      ____Debug____ (
      Serial_D.println("Hit AutoPWM");
      Serial_D.print("Old value: ");
      Serial_D.println(autoPWM);
      )
      autoPWM = atoi(truncated_payload);
      ____Debug____ (
      Serial_D.print("New value: ");
      Serial_D.println(autoPWM);      
      )
      EEPROM.put(1,autoPWM);
      EEPROM.commit();
      
      if (!(autoPWM)){
          mqttClient.publish(MQTT_PUB_TEMP2, 1, true, String((int)sliderValue).c_str());
        }

      MQTT_Update = 1;
    }




  // ____Debug____ (
  //   Serial_D.println("Publish received.");
  //   Serial_D.print("  topic: ");
  //   Serial_D.println(topic);
  //   Serial_D.println(payload);
  //   Serial_D.print("  qos: ");
  //   Serial_D.println(properties.qos);
  //   Serial_D.print("  dup: ");
  //   Serial_D.println(properties.dup);
  //   Serial_D.print("  retain: ");
  //   Serial_D.println(properties.retain);
  //   Serial_D.print("  len: ");
  //   Serial_D.println(len);
  //   Serial_D.print("  index: ");
  //   Serial_D.println(index);
  //   Serial_D.print("  total: ");
  //   Serial_D.println(total);
  // )
}







// =======================================================================================
// =======================================================================================
// ======================== SETUP ========================================================
// =======================================================================================
// =======================================================================================
void setup() {
#ifdef isDebugging
  
  
#endif

  ____Debug____(Serial_D.begin(115200);Serial_D.println("Booting");)

  sensors.begin();
  readDSTemperatureC();
 
  WiFi.mode(WIFI_STA);
  EEPROM.begin(4);

  ___OLED___( 
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_24);
  )


#ifdef debugDS
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  Serial_D.print("Locating devices...");
  Serial_D.print("Found ");
  Serial_D.print(numberOfDevices, DEC);
  Serial_D.println(" devices.");


        DeviceAddress T_Sensor;
        sensors.getAddress(T_Sensor,0); 
          Serial_D.print("\n\rFound \'1-Wire\' device with address:\n\r");
          for( uint8_t i = 0; i < 8; i++) {
            Serial_D.print("0x");
            if (T_Sensor[i] < 16) {
              Serial_D.print('0');
            }
            Serial_D.print(T_Sensor[i], HEX);
            if (i < 7) {
              Serial_D.print(", ");
            }
          }
              if ( OneWire::crc8( T_Sensor, 7) != T_Sensor[7]) {
              Serial_D.print("CRC is not valid!\n");
              return;
          }
          Serial_D.println();

 

#endif
  
  sliderValue = read_slider_from_eeprom();
  EEPROM.get(1, autoPWM);
  PWMvalue = sliderValue_to_PWM (sliderValue);

  analogWriteFreq(2000);

  analogWrite(output1, PWMvalue);


  WiFi.begin(ssid, password);
#ifdef strickt_wifi
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
#ifdef isDebugging
    Serial_D.println("Connection Failed! Rebooting...");
#endif
    delay(5000);
    ESP.restart();
  }
#endif


  ArduinoOTA.onStart([]() {
#ifdef isDebugging
    Serial_D.println("Start");
#endif
  });
  ArduinoOTA.onEnd([]() {
#ifdef isDebugging
    Serial_D.println("\nEnd");
#endif
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
#ifdef isDebugging
    Serial_D.printf("Progress: %u%%\r", (progress / (total / 100)));
#endif
  });
  ArduinoOTA.onError([](ota_error_t error) {
#ifdef isDebugging
    Serial_D.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial_D.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial_D.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial_D.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial_D.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial_D.println("End Failed");
#endif
  });
  ArduinoOTA.begin();
#ifdef isDebugging
  Serial_D.println("Ready");
  Serial_D.print("IP address: ");
  Serial_D.println(WiFi.localIP());
#endif

  // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_htm, processor);
  });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/javascrip", script_js, processor);
  });
    server.on("/jquery-3.2.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/javascrip", jquery_3_2_1_min_js, processor);
  });

    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "image/bmp", favicon_bmp, processor);
  });

  
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/css", style_css, processor);
  });
    server.on("/temp_sensor", HTTP_GET, [](AsyncWebServerRequest *request){
//    readDSTemperatureC();
    request->send_P(200, "text/plain", String(tempSensor1).c_str());
  });
    server.on("/resetesp", HTTP_POST, [](AsyncWebServerRequest *request){
      request->send(200);
      ESP.restart();

  });

 
  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = inputMessage.toInt();
      GUI_Update = 1;
//      PWMvalue = sliderValue_to_PWM (sliderValue);
      ____Debug____(
        Serial_D.print ( "Slider Value = ");
        Serial_D.println (sliderValue, DEC);
        Serial_D.print ( "PWM Value = ");
        Serial_D.println (PWMvalue, DEC);
      )
//      analogWrite(output1, PWMvalue);
      EEPROM.put(0,sliderValue);
      EEPROM.commit();

    }
    else {
      inputMessage = "No message sent";
    }
#ifdef isDebugging
    Serial_D.print("/slider input message: ");
    Serial_D.println(inputMessage);
#endif
    request->send(200, "text/plain", "OK");
  });


  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/index_get_data", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    ____Debug____(
      Serial_D.println("Recieved get data request");
    )
    if (request->hasParam(PARAM_INPUT1)) {
      inputMessage = request->getParam(PARAM_INPUT1)->value();

    }
    else {
      inputMessage = "No message sent";
    }
#ifdef isDebugging
    Serial_D.print("/index_get_data input message: ");
    Serial_D.println(inputMessage);
#endif
    String response = String(tempSensor1)+";"+String((int)sliderValue)+";"+String((int)weightedTemperature)+";"+String(autoPWM);
    ____Debug____(
      Serial_D.println("Responce " + response);
    )    
    request->send(200, "text/plain", response);
  });


  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/switch1", HTTP_POST, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
        // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      if (inputMessage.equals("true")){
        autoPWM = 1;
        GUI_Update = 1;
        EEPROM.put(1,autoPWM);
        EEPROM.commit();

      } else {
        autoPWM = 0;
        GUI_Update = 1;
        EEPROM.put(1,autoPWM);
        EEPROM.commit();
      }
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    request->send(200, "text/plain", "OK");
  });

  
  // Start server
  server.begin();

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials("mqtt-user","Mrainb0wT");
 

  connectToMqtt();
 


}
// =======================================================================================
// =======================================================================================
// ======================== END SETUP ========================================================
// =======================================================================================
// =======================================================================================



void loop() {
  
  ArduinoOTA.handle();

  


      if ((millis() - DS18B20lastTime) > DS18B20Delay) {
      readDSTemperatureC();
      
      weightedTemperature = ((tempSensor1 - temp_min)/(temp_max - temp_min))*100;
      if (weightedTemperature >= 100.00) { autoPWMvalue = maximum_autoPWMvalue; weightedTemperature = 100;}
      else if (weightedTemperature <= 1.00)  { autoPWMvalue = minimum_autoPWMvalue; weightedTemperature = 0;}
        else 
        {
          autoPWMvalue = sliderValue_to_PWM(weightedTemperature);
        }
        if (autoPWM){
            analogWrite(output1, autoPWMvalue);
                        ____Debug____(
            Serial_D.print("Auto PWM");
            Serial_D.print("Auto value (weighted to 100%) = ");
            Serial_D.println(weightedTemperature);
            Serial_D.print("Auto PWM value: ");
            Serial_D.println(autoPWMvalue);
                        )

        }
      char buffer[64];
      
      String bufferS;
      snprintf(buffer, sizeof buffer, "%.2f", tempSensor1);  
      ____Debug____(
      Serial_D.printf("Buffer: ");
      Serial_D.println( buffer);  
      Serial_D.println();
      )
      ___OLED___(
      display.clear();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      bufferS = buffer;
      if (autoPWM == 0){
        bufferS += "° M";
      }
      else {
        bufferS += "° A";
        
      }
      display.drawString(0, 0, bufferS);
      display.display();
      )

      if (!MQTT_connection_status) connectToMqtt();

      mqttClient.publish(MQTT_PUB_TEMP1, 1, true, String(tempSensor1).c_str());

      if (autoPWM == 1){
          mqttClient.publish(MQTT_PUB_TEMP2, 1, true, String((int)weightedTemperature).c_str());
        }



      DS18B20lastTime = millis();
      

    }  

      if (GUI_Update) {

        mqttClient.publish(MQTT_PUB_TEMP2, 1, true, String((int)sliderValue).c_str());
        mqttClient.publish(MQTT_PUB_TEMP3, 1, true, String(autoPWM).c_str());

        if (!(autoPWM)) {
          ____Debug____(Serial_D.println("Slider update");)
          PWMvalue = sliderValue_to_PWM (sliderValue);
          analogWrite(output1, PWMvalue);          
        }
        GUI_Update = 0;

      }

      if (MQTT_Update){
        if (!(autoPWM)) {
          ____Debug____(Serial_D.println("Slider update");)
          PWMvalue = sliderValue_to_PWM (sliderValue);
          analogWrite(output1, PWMvalue); 
        }
        MQTT_Update = 0;  
      }



      if ((millis() - heartbeat_timer) > heartbeat_delay) {
          if (heartbeat_state){
            heartbeat_state = false;
            ___OLED___(
            display.drawRect (125, 0, 2, 2);
            display.display();
            )

          }
          else {
            heartbeat_state = true;
            ___OLED___(
            display.setColor(BLACK);
            display.drawRect (125, 0, 2, 2);
            display.setColor(WHITE);
            display.display();
            )

          }
          heartbeat_timer = millis();
      }
    
    
}
