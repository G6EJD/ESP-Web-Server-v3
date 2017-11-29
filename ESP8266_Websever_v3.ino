// ESP8266 WebServer - using DHT11 sensor and no SD Card
// (c) D L Bird 2017
//
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <DHT.h>

#define DHTTYPE DHT11 // or DHT22
#define DHTPIN  2
#define RELAY_Pin 5   //Initialise D1 aka GPIO-5 as an output on the WeMOS board
#define temp_error_offset     -2
#define humidity_error_offset 16
#define version 4

DHT dht(DHTPIN, DHTTYPE);

float humidity, temperature;  // Values read from sensor
bool   Relay_status,LED_status,LED_Register_Status,RELAY_Register_Status;
String webpage = "";

const char *ssid     = "your_ssid";
const char *password = "your_password";

ESP8266WebServer server(80);  // Set the port you wish to use, a browser default is 80, but any port can be used, if you set it to 5555 then connect with http://nn.nn.nn.nn:5555/
  
void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();
  delay(100);
  update_temperature_and_humidity(); // Initial read to proevent unknown values that sometimes occur
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Wait for connection
  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ <= 10) {//wait 10 seconds
    delay(1000);
  }
  if(i == 11){
    Serial.print("Could not connect to network...");
    while(1) delay(500);
  }
  Serial.print("Connected to IP address: ");
  Serial.println(WiFi.localIP()); // Use the IP address printed here to connect to the server e.g. http://192.168.0.42

  server.on("/",              home_page);         // Define what happens when a client requests attention // for example http://your_ip_address/
  server.on("/LED_on",        led_on);            // Define what happens when a client requests attention // for example http://your_ip_address/LED_on
  server.on("/LED_off",       led_off);           // Define what happens when a client requests attention
  server.on("/LED_status",    led_status);        // Define what happens when a client requests attention
  server.on("/RELAY_on",      relay_on);          // Define what happens when a client requests attention
  server.on("/RELAY_off",     relay_off);         // Define what happens when a client requests attention
  server.on("/RELAY_status",  relay_status);      // Define what happens when a client requests attention
  server.on("/DHT_readings",  temp_and_humidity); // Define what happens when a client requests attention
  // Comment these lines out if extra functions are not required
  server.on("/NEW_function1", new_function1);     // Comment this line out if not required
  server.on("/NEW_function2", new_function2);     // Comment this line out if not required

  server.onNotFound(handleNotFound);
  server.begin();                 // Start the webserver
  pinMode(RELAY_Pin, OUTPUT);     // Initialize D1 aka GPIO5 as an output
  digitalWrite(RELAY_Pin, LOW);   // Turn the RELAY off (Note that relay is active HIGH, so LOW is off
  Relay_status = false;           // Relay is now on
  pinMode(LED_BUILTIN, OUTPUT);   // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW); // Turn the LED on, set pin Low
  LED_status = false;
}

void loop(){
  server.handleClient();   // Wait for a client to connect and when they do process their requests
}

//----------- Subroutines/functions
//
// Read DHT for temperature and humidity, create web page (html) and display it
void temp_and_humidity() {
  update_temperature_and_humidity();
  append_page_header();
  webpage += "<div id=\"section\"><h2>DHT11 Readings</h2>";
  webpage += "Temperature: " + String((int)temperature) + "&degC&nbsp&nbsp&nbsp"; // and some no-break spaces
  webpage += "Humidity: "    + String((int)humidity)    + "%rh";
  webpage += "</div>";
  append_page_footer();
  server.send(200, "text/html", webpage);
}

// Read DHT for temperature and humidity
void update_temperature_and_humidity() {
  temperature = dht.readTemperature();
  humidity    = dht.readHumidity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}

// Switch on-board LED ON, create web page and display it
void home_page() {
  append_page_header();
  webpage += "<div id=\"section\"><h2>Home Page - Welcome to my webserver</h2>";
  
  webpage += "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Maecenas porttitor congue massa.";
  webpage += "Fusce posuere, magna sed pulvinar ultricies, purus lectus malesuada libero, sit amet commodo magna eros quis urna.";
  webpage += "Nunc viverra imperdiet enim. Fusce est. Vivamus a tellus.";
  webpage += "Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Proin pharetra nonummy pede. Mauris et orci.";
  
  webpage += "<br></div>";
  append_page_footer();
  server.send(200, "text/html", webpage);
}


// Switch on-board LED ON, create web page and display it
void led_on() {
  append_page_header();
  webpage += "<div id=\"section\"><h2>LED Control</h2>";
  webpage += "LED has been switched ON";
  webpage += "</div>";
  append_page_footer();
  webpage += "<br></div>";
  server.send(200, "text/html", webpage);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on, set pin Low
  LED_status = true;
  delay(1000);
  Serial.println("Switched LED ON");
}

// Switch on-board LED OFF, create web page and display it
void led_off() {
  append_page_header();
  webpage += "<div id=\"section\"><h2>LED Control</h2>";
  webpage += "LED has been switched OFF";
  webpage += "<br></div>";
  append_page_footer();
  server.send(200, "text/html", webpage);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off, set pin High
  LED_status = false;
  delay(1000);
  Serial.println("Switched LED OFF");
}

// Show LED status
void led_status() {
  append_page_header();
  webpage += "<div id=\"section\"><h2>LED Status</h2>";
  LED_Register_Status =  digitalRead(LED_BUILTIN);
  if (LED_status) webpage += "LED state is ON"; else webpage += "LED state is OFF";
  webpage += "<br><br>";
  if (LED_Register_Status) webpage += "LED Register Reports LED is ON"; else webpage += "LED Register Reports LED is OFF";
  webpage += "<br></div>";
  append_page_footer();
  server.send(200, "text/html", webpage);
}

// Switch RELAY ON, create web page and display it
void relay_on() {
  append_page_header();
  webpage += "<div id=\"section\"><h2>RELAY Control</h2>";
  webpage += "RELAY has been switched ON";
  webpage += "<br></div>";
  append_page_footer();
  server.send(200, "text/html", webpage);
  pinMode(RELAY_Pin, OUTPUT);     // Initialize the D1 aka GPIO5 as an output
  digitalWrite(RELAY_Pin, HIGH);  // Turn the RELAY on (Note the Relay is active high
  Relay_status = true; // Relay is now on
  Serial.println("Switched RELAY ON");
}

// Switch RELAY OFF create web page and display it
void relay_off() {
  append_page_header();
  webpage += "<div id=\"section\"><h2>RELAY Control</h2>";
  webpage += "RELAY has been switched OFF";
  webpage += "<br></div>";
  append_page_footer();
  server.send(200, "text/html", webpage);
  pinMode(RELAY_Pin, OUTPUT);     // Initialize D1 aka GPIO5 as an output
  digitalWrite(RELAY_Pin, LOW);   // Turn the RELAY off (Note that relay is active HIGH, so LOW is off
  Relay_status = false; // Relay is now on
  Serial.println("Switched RELAY OFF");
}

// Show RELAY status
void relay_status() {
  append_page_header();
  webpage += "<div id=\"section\"><h2>RELAY Status</h2>";
  RELAY_Register_Status =  digitalRead(RELAY_Pin);
  if (Relay_status) webpage += "RELAY state is ON"; else webpage += "RELAY state is OFF";
  webpage += "<br><br>";
  if (RELAY_Register_Status) webpage += "RELAY Register Reports RELAY is ON"; else webpage += "RELAY Register Reports RELAY is OFF";
  webpage += "<br></div>";
  append_page_footer();
  server.send(200, "text/html", webpage);
}

// New function for expansion, or delete if not required having commented out of deleted the other lines that call this
// Reword the text to suit
void new_function1() {
  append_page_header();
  webpage += "<div id=\"section\"><h2>New Function-1</h2>";
  webpage += "Do something here for New Function-1";
  webpage += "<br></div>";
  append_page_footer();
  server.send(200, "text/html", webpage);
}

// New function for expansion, or delete if not required having commented out of deleted the other lines that call this
// Reword the text to suit
void new_function2() {
  append_page_header();
  webpage += "<div id=\"section\"><h2>New Function-2</h2>";
  webpage += "Do something here for New Function-2";
  webpage += "<br></div>";
  append_page_footer();
  server.send(200, "text/html", webpage);
}

// Handle file not found situation
void handleNotFound(){
  String message = "";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.print(message);
}


void append_page_header() {
  webpage  = "<!DOCTYPE html><html><head>";
  webpage += "<title>LED and RELAY Control Webserver</title>";
  webpage += "<style>ul{list-style-type:none;margin:0;padding:0;overflow:hidden;background-color:#d8d8d8;font-size:14px;}";
  webpage += "li{float:left;border-right:1px solid #bbb;}last-child {border-right: none;}";
  webpage += "li a{display: block;padding:3px 10px;text-decoration:none;}";
  webpage += "li a:hover{background-color:#FFFFFF;}";
  webpage += "section {font-size:14px;}";
  webpage += "p {background-color:#E3D1E2}";
  webpage += "h1{background-color:#d8d8d8;}";
  webpage += "h3{color:orange;font-size:24px;}";
  webpage += "body {width:1024px;margin:0 auto;font-family:arial;font-size:16px;text-align:center;color:blue;background-color:#F7F2Fd;}";
  webpage += "</style><h1>Sensor, LED and RELAY Control Webserver "+ String(version) + "</h1>";
}

void append_page_footer(){ // Saves repeating many lines of code for HTML page footers
  webpage += "<br><ul>";

  webpage += "<li><a href='/'>Home Page</a></li>";
  webpage += "<li><a href='/RELAY_on'>Relay ON</a></li>";
  webpage += "<li><a href='/RELAY_off'>Relay OFF</a></li>";
  webpage += "<li><a href='/RELAY_status'>Relay Status</a></li>";
  webpage += "<li><a href='/LED_on'>LED ON</a></li>";
  webpage += "<li><a href='/LED_off'>LED OFF</a></li>";
  webpage += "<li><a href='/LED_status'>LED Status</a></li>";
  webpage += "<li><a href='/DHT_readings'>Sensor Readings</a></li>";
  // Comment these lines out if extra functions are not required
  webpage += "<li><a href='/NEW_function1'>New Function 1</a></li>";
  webpage += "<li><a href='/NEW_function2'>New Function 2</a></li>";
    
  webpage += "</ul>";
  webpage += "<p>&copy;"+String(char(byte(0x40>>1)))+String(char(byte(0x88>>1)))+String(char(byte(0x5c>>1)))+String(char(byte(0x98>>1)))+String(char(byte(0x5c>>1)));
  webpage += String(char((0x84>>1)))+String(char(byte(0xd2>>1)))+String(char(0xe4>>1))+String(char(0xc8>>1))+String(char(byte(0x40>>1)));
  webpage += String(char(byte(0x64/2)))+String(char(byte(0x60>>1)))+String(char(byte(0x62>>1)))+String(char(0x6e>>1))+"</p>";
  webpage += "</body></html>";
}

