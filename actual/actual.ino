#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>

#include <DallasTemperature.h>
#include <OneWire.h>

#define moisture_pin A0 // moisture sensor A0 pin connected to arduino A0
#define ONE_WIRE_BUS 2
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tempSensors(&oneWire);


// Allocate a temporary JsonDocument
// Use arduinojson.org/v6/assistant to compute the capacity.
StaticJsonDocument<512> doc;
JsonArray sensorValues;
int id = 1;
int interval = 5; // Enter time interval in seconds

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetServer server(80);

void setup() {
  // Initialize serial port
  Serial.begin(9600);
  
  //Moisture Part
  Serial.println("Measuring Soil Moisture Level with Arduino.");
  pinMode(moisture_pin, INPUT); //set moisture pin as input
  delay(1000);
  
  //Temperature Part
  Serial.println("Dallas Temperature IC Control Library Demo");
    // Start up the library
  tempSensors.begin();
  delay(1000);
  
  //JSON Part
  while (!Serial) continue;

  // Initialize Ethernet libary
  if (!Ethernet.begin(mac)) {
    Serial.println(F("Failed to initialize Ethernet library"));
    return;
  }

  // Start to listen
  server.begin();

  Serial.println(F("Server is ready."));
  Serial.print(F("Please connect to http://"));
  Serial.println(Ethernet.localIP());

  sensorValues = doc.createNestedArray("posts");
}

int getMoisture() {
  int moisture_level = map(analogRead(moisture_pin), 0, 1023, 100, 0); //read sensor and scale the reading 100% to 0%

  //print moisture level on serial monitor
  Serial.print("Moisture Level: ");
  Serial.print(moisture_level);
  Serial.print('%');
  Serial.println(); //go to next line
  return moisture_level;
}

float getTemperature() {
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print(" Requesting temperatures...");
  tempSensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  float temperature = tempSensors.getTempCByIndex(0);// Why "byIndex"? 
    // You can have more than one IC on the same bus. 
    // 0 refers to the first IC on the wire
  
  Serial.print("Temperature is: ");
  Serial.print(temperature);
  return temperature;
}

void loop() {
  // Wait for an incomming connection
  delay(1000);
  EthernetClient client = server.available();

  // Do we have a client?
  if (!client) return;

  Serial.println(F("New client"));

  // Read the request (we ignore the content in this example)
  while (client.available()) client.read();


  //Get Sensor Values
  delay(1000);
  int moisture = getMoisture();
  delay(1000);
  float temperature = getTemperature();
  
  //JSON Object Creation
  delay(1500);
  JsonObject object = sensorValues.createNestedObject();
  object["id"] = id;
  object["date"] = "6-11-2020";
  object["time"] = "00:23";
  object["temperature"] = temperature;
  object["moisture"] = moisture;
  object["ph"] = 6.8;
    
  
  Serial.print(F("Sending: "));
  serializeJson(doc, Serial);
  Serial.println();

  // Write response headers
  client.println(F("HTTP/1.0 200 OK"));
  client.println(F("Access-Control-Allow-Origin: *"));   
  client.println(F("Access-Control-Allow-Methods: GET"));
  client.println(F("Content-Type: application/json"));
  client.println(F("Connection: close"));
  client.print(F("Content-Length: "));
  client.println(measureJsonPretty(doc));
  client.println();

  // Write JSON document
  serializeJsonPretty(doc, client);

  id++;

  // Disconnect
  delay(interval * 1000);
  client.stop();
}
