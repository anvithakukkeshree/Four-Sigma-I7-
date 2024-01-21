#include <ESP8266WiFi.h>

#include <DallasTemperature.h>

#include <OneWire.h>

#include "DHT.h" 

#include "Adafruit_MQTT.h"

#include "Adafruit_MQTT_Client.h"

#include <ArduinoJson.h>

#include "ada.h"

#include "global.h"

#include "random.h"


const char *ssid =  "APPUGOWDA75";    

const char *pass =  "appugowda";


WiFiClient client;

#define MQTT_SERV "io.adafruit.com"

#define MQTT_PORT 1883

#define MQTT_NAME "anvi12345" 

#define MQTT_PASS "aio_eIiL73RBRoKoU135J7ig7Q7IxMKS" 

const char server[] = "api.openweathermap.org";

String nameOfCity = "Mysuru,IN";

String apiKey = "d39817fec2ef86467c5e11912ee6a5d7";

String text;

const char* icon="";

int jsonend = 0;

boolean startJson = false;

int status = WL_IDLE_STATUS;

#define JSON_BUFF_DIMENSION 2500

unsigned long lastConnectionTime = 10 * 60 * 1000;     

const unsigned long postInterval = 10 * 60 * 1000;  

const int ldrPin = D1;

const int ledPin = D0;

const int moisturePin = A0;  

const int motorPin = D8;

float moisturePercentage;              

int temperature, humidity, soiltemp;

#define ONE_WIRE_BUS 4   

#define DHTTYPE DHT11 

#define dht_dpin D4

DHT dht(dht_dpin, DHTTYPE);

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

const unsigned long Interval = 50000;

unsigned long previousTime = 0;


Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

Adafruit_MQTT_Publish Moisture = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/Moisture");  

Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/Temperature");

Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/Humidity");

Adafruit_MQTT_Publish SoilTemp = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/SoilTemp");

Adafruit_MQTT_Publish WeatherData = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/WeatherData");


 Adafruit_MQTT_Subscribe LED = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/LED");

 Adafruit_MQTT_Subscribe Pump = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/Pump");  

void setup()

{

  Serial.begin(9600);

  delay(10);

  dht.begin();

  sensors.begin();

  mqtt.subscribe(&LED);

  mqtt.subscribe(&Pump);

  pinMode(motorPin, OUTPUT);

  pinMode(ledPin, OUTPUT);

  pinMode(ldrPin, INPUT);

  digitalWrite(motorPin, LOW); 

  digitalWrite(ledPin, HIGH);

  text.reserve(JSON_BUFF_DIMENSION);

  Serial.println("Connecting to ");

  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)

  {
    {

    delay(500);

    Serial.print(".");              

  }

  Serial.println("");

  Serial.println("WiFi connected");

}


  Serial.begin(9600);

  pinMode(2,OUTPUT);

  set_wifi();

  init_Ada_parse(); 

  Init_Ada_connect();
}

void loop()

{

 unsigned long currentTime = millis();

 MQTT_connect();

 if (millis() - lastConnectionTime > postInterval) {

    

    lastConnectionTime = millis();

    makehttpRequest();

  }

{
  Google_Callback();
}

//}

 int ldrStatus = analogRead(ldrPin);

    if (ldrStatus <= 200) {

       digitalWrite(ledPin, HIGH);

       Serial.print("Its DARK, Turn on the LED : ");

       Serial.println(ldrStatus);    

    } 

    else {    

      digitalWrite(ledPin, LOW);

      Serial.print("Its BRIGHT, Turn off the LED : ");

      Serial.println(ldrStatus);

     }

  moisturePercentage = ( 100.00 - ( (analogRead(moisturePin) / 1023.00) * 100.00 ) ); 

  Serial.print("Soil Moisture is  = ");

  Serial.print(moisturePercentage);

  Serial.println("%");   

if (moisturePercentage < 35) {

  digitalWrite(motorPin, HIGH);       

}

if (moisturePercentage > 38) {

  digitalWrite(motorPin, LOW);        

}

 temperature = dht.readTemperature();  

 humidity = dht.readHumidity();


 sensors.requestTemperatures();

 soiltemp = sensors.getTempCByIndex(0);



if (currentTime - previousTime >= Interval) { 

    if (! Moisture.publish(moisturePercentage))

         {                     

            }

    if (! Temperature.publish(temperature)) 

         {                     

           }

    if (! Humidity.publish(humidity)) 

         {                     

           

          }

    if (! SoilTemp.publish(soiltemp)) 

       {                            

          }

    if (! WeatherData.publish(icon)) 

       {          

         }

          previousTime = currentTime;

}

Adafruit_MQTT_Subscribe * subscription;

while ((subscription = mqtt.readSubscription(5000))) 

     {

    if (subscription == &LED)

     {

      

      Serial.println((char*) LED.lastread);

        if (!strcmp((char*) LED.lastread, "OFF"))

       {

         digitalWrite(ledPin, LOW);

        }

        if (!strcmp((char*) LED.lastread, "ON"))

          {

         digitalWrite(ledPin, HIGH);

         }

     }  

    if (subscription == &Pump)

      {

      

      Serial.println((char*) Pump.lastread);

     

      if (!strcmp((char*) Pump.lastread, "OFF"))

       {

        digitalWrite(motorPin, HIGH);

       }

     if (!strcmp((char*) Pump.lastread, "ON"))

       {

        digitalWrite(motorPin, LOW);

       }

     }

    }

  delay(9000);

 

}

void MQTT_connect() 

{

  int8_t ret;

  

  if (mqtt.connected()) 

  {

    return;

  }

  uint8_t retries = 3;

  while ((ret = mqtt.connect()) != 0) 

  { 

       mqtt.disconnect();

       delay(5000); 

       retries--;

       if (retries == 0) 

       {

         

         while (1);

       }

  }

}

void makehttpRequest() {

  

  client.stop();

  

  if (client.connect(server, 80)) {

    client.println("GET /data/2.5/forecast?q=" + nameOfCity + "&APPID=" + apiKey + "&mode=json&units=metric&cnt=2 HTTP/1.1");

    client.println("Host: api.openweathermap.org");

    client.println("User-Agent: ArduinoWiFi/1.1");

    client.println("Connection: close");

    client.println();

    unsigned long timeout = millis();

    while (client.available() == 0) {

      if (millis() - timeout > 5000) {

        Serial.println(">>> Client Timeout !");

        client.stop();

        return;

      }

    }   

    char c = 0;

    while (client.available()) {

      c = client.read();

    

      if (c == '{') {

        startJson = true;     

        jsonend++;

      }

      if (c == '}') {

        jsonend--;

      }

      if (startJson == true) {

        text += c;

      }

       

      if (jsonend == 0 && startJson == true) {

        parseJson(text.c_str());  

        text = "";                

        startJson = false;     

      }

    }

  }

  else {

    

    Serial.println("connection failed");

    return;

  }

}



void parseJson(const char * jsonString) {


  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 4*JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 2*JSON_OBJECT_SIZE(7) + 2*JSON_OBJECT_SIZE(8) + 720;

  DynamicJsonBuffer jsonBuffer(bufferSize);



  

  JsonObject& root = jsonBuffer.parseObject(jsonString);

  if (!root.success()) {

    Serial.println("parseObject() failed");

    return;

  }

  JsonArray& list = root["list"];

  JsonObject& nowT = list[0];

  JsonObject& later = list[1];

  JsonObject& tommorow = list[2];

  String city = root["city"]["name"];

  String weatherNow = nowT["weather"][0]["description"];

  String weatherLater = later["weather"][0]["description"];

  String list12 = later["weather"][0]["list"];

  Serial.println(list12);

  Serial.println(weatherLater);

  if(weatherLater == "few clouds"){

    icon = "Few Clouds"; 

    Serial.print(icon);

  }

  else if(weatherLater == "rain"){

    icon = "Rain";

    Serial.print(icon);

  }

  else if(weatherLater == "broken clouds"){
 
    icon = "Broken Clouds";

    Serial.print(icon);

  }

  else {

    icon = "Sunny";

    }

}