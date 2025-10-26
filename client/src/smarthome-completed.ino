/*
   ______            __                  _            __                __________
  / ____/_  ______ _/ /__________ _   __(_)__  ____  / /_____  _____   / ____/  _/
 / /   / / / / __ `/ __/ ___/ __ \ | / / / _ \/ __ \/ __/ __ \/ ___/  / /    / /  
/ /___/ /_/ / /_/ / /_/ /  / /_/ / |/ / /  __/ / / / /_/ /_/ (__  )  / /____/ /   
\____/\__,_/\__,_/\__/_/   \____/|___/_/\___/_/ /_/\__/\____/____/   \____/___/   
                                                                                
    __             ___              __             ______   __   
   / /_  __  __   /   |  ____  ____/ /__  _____   / ____/  / /   
  / __ \/ / / /  / /| | / __ \/ __  / _ \/ ___/  / /_     / /    
 / /_/ / /_/ /  / ___ |/ / / / /_/ /  __/ /     / __/  _ / /____ 
/_.___/\__, /  /_/  |_/_/ /_/\__,_/\___/_/     /_/    (_)_____(_)
      /____/   
                                           
 Smarthome Completed Project
 
 This project integrates all the smarthome lessons into a single Arduino sketch.
 It includes functionalities for:
 - Remote LED control (Lesson 4)
 - Remote RGB LED control (Lesson 5)
 - Remote buzzer control (Lesson 6)
 - Remote temperature and humidity monitoring (Lesson 7)
 - Remote servo control (Lesson 8B)
 - Remote gas/fire detection (Lesson 9)
 - Remote flame detection (Lesson 10)
 - Remote noise detection (Lesson 11)
 - Remote motion detection (Lesson 13)
 - Remote LCD message display (Lesson 14)
 - RFID based door lock (Lesson 16B)
 - Ultrasonic distance measurement (Lesson 17)
 - UDP communication (Lesson 18)

 CopyRight Ander F.L. <ander_frago@cuatrovientos.org> June 2024
*/

// Included Libraries
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <RFID.h>
#include <dht.h>
#include <Keypad.h>

// Pin Definitions
#define redLED 13
#define greenLED 12
#define buzzer 5
#define flame_sensor A1
#define sound_sensor A2
#define motion_sensor 4
#define whiteLED 9
#define yellowLED 10
#define Trig_PIN 25
#define Echo_PIN 26
#define SERVO_PIN 3
#define RFID_SDA 48
#define RFID_RST 49
#define DHT11_PIN 2
#define redPin_RGB 24
#define greenPin_RGB 23
#define bluePin_RGB 22

// WiFi Configuration
char ssid[] = "Pixel_5265"; // replace ****** with your network SSID (name)
char pass[] = "4Vientos"; // replace ****** with your network password
int status = WL_IDLE_STATUS;

// Software Serial for ESP8266
SoftwareSerial softserial(A9, A8); // A9 to ESP_TX, A8 to ESP_RX

// Web Server
WiFiEspServer server(80);
RingBuffer buf(80);

// UDP Communication
WiFiEspUDP Udp;
unsigned int localPort = 8888;
char packetBuffer[255];
char ReplyBuffer[] = "I am Smarthome!";

// LCD Display
LiquidCrystal_I2C lcd(0x27, 16, 2);
int row = 0;
boolean flag = false;

// Servo Motor
Servo head;

// RFID
unsigned char my_rfid[] = {227, 10, 252, 39, 50}; // replace with your RFID value
MFRC522 rfid(RFID_SDA, RFID_RST);

// DHT Sensor
dht DHT;

// Keypad
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {33, 35, 37, 39};
byte colPins[COLS] = {41, 43, 45, 47};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Global Variables
int ledStatus = LOW;
int buzzerStatus = LOW;
int flame_status = 0;
String gasStr;
int SoundStatus = 0;
String StatusStr;
int gasStatus = 0;
int distance_val = 0;
String distance_str;

void setup() {
  // Initialize Serial
  Serial.begin(9600);

  // Initialize Pins
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(flame_sensor, INPUT);
  pinMode(sound_sensor, INPUT);
  pinMode(motion_sensor, INPUT);
  pinMode(whiteLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(Trig_PIN, OUTPUT);
  pinMode(Echo_PIN, INPUT);
  pinMode(redPin_RGB, OUTPUT);
  pinMode(greenPin_RGB, OUTPUT);
  pinMode(bluePin_RGB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Smarthome Ready!");

  // Initialize WiFi
  softserial.begin(115200);
  softserial.write("AT+CIOBAUD=9600\r\n");
  softserial.write("AT+RST\r\n");
  softserial.begin(9600);
  WiFi.init(&softserial);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();

  // Initialize Web Server
  server.begin();

  // Initialize UDP
  Udp.begin(localPort);

  // Initialize SPI and RFID
  SPI.begin();
  rfid.init();
}

void loop() {
  // Handle Web Server Clients
  handleWebServer();

  // Handle UDP Packets
  handleUdp();

  // Read Sensors
  readSensors();

  // Handle Keypad Input
  handleKeypad();
  
  // Handle RFID
  handleRFID();
}

void handleWebServer() {
  WiFiEspClient client = server.available();
  if (client) {
    Serial.println("New client");
    buf.init();
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        buf.push(c);

        if (buf.endsWith("\r\n\r\n")) {
          sendHttpResponse(client);
          break;
        }

        // Lesson 4: LED Control
        if (buf.endsWith("GET /H")) {
          ledStatus = HIGH;
          digitalWrite(LED_BUILTIN, HIGH);
        } else if (buf.endsWith("GET /L")) {
          ledStatus = LOW;
          digitalWrite(LED_BUILTIN, LOW);
        }

        // Lesson 5: RGB Control
        else if (buf.endsWith("GET /R")) {
          color(255, 0, 0);
        } else if (buf.endsWith("GET /G")) {
          color(0, 255, 0);
        } else if (buf.endsWith("GET /B")) {
          color(0, 0, 255);
        }

        // Lesson 6: Buzzer Control
        else if (buf.endsWith("GET /Z1")) {
          buzzerStatus = HIGH;
          digitalWrite(buzzer, HIGH);
        } else if (buf.endsWith("GET /Z0")) {
          buzzerStatus = LOW;
          digitalWrite(buzzer, LOW);
        }
        
        // Lesson 14: LCD Message
        if (buf.endsWith("usr=")) {
          lcd.clear();
          lcd.backlight();
          lcd.print("welcome");
          flag = true;
        }
        if (flag) {
          if (c != '&' && c != '=') {
            lcd.setCursor(row, 1);
            lcd.print(c);
            row++;
          } else {
            flag = false;
            row = 0;
          }
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}

void handleUdp() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println(packetBuffer);

    // Lesson 8B and 16B: Remote Servo Control
    char c = packetBuffer[0];
    switch (c) {
      case 'L':
        close_door();
        break;
      case 'A':
        half_open();
        break;
      case 'R':
        open_door();
        break;
    }
    
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }
}

void readSensors() {
  // Lesson 9 & 10: Flame and Gas Sensor
  flame_status = digitalRead(flame_sensor);
  if (flame_status == 1) {
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    gasStr = "<font color=GREEN><b>Safe</b></font>";
  } else {
    digitalWrite(buzzer, HIGH);
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    gasStr = "<font color=RED><b>Fire Detected!</b></font>";
  }

  // Lesson 11: Sound Sensor
  SoundStatus = digitalRead(sound_sensor);
  if (SoundStatus == 1) {
    StatusStr = "<font color=GREEN><b>No Noise!</b></font>";
  } else {
    StatusStr = "<font color=RED><b>Noise Detected!</b></font>";
  }

  // Lesson 13: Motion Sensor
  gasStatus = digitalRead(motion_sensor);
  if (gasStatus == 0) {
    gasStr = "<font color=GREEN><b>No Intruder!</b></font>";
  } else {
    gasStr = "<font color=RED><b>Intruder Detected!</b></font>";
  }

  // Lesson 17: Ultrasonic Sensor
  distance_val = watch();
  if (distance_val > 40) {
    distance_str = "<font color=000000><b>No object in range</b></font>";
  } else if (distance_val > 20) {
    distance_str = "<font color=red><b>Object is Very Far</b></font>";
  } else if (distance_val > 10) {
    distance_str = "<font color=green><b>Object is far</b></font>";
  } else if (distance_val > 5) {
    distance_str = "<font color=yellow><b>Object is  close</b></font>";
  } else {
    distance_str = "<font color=ffffff><b>Object is Very Close!</b></font>";
  }
  
  // Lesson 7: DHT11 Sensor
  DHT.read11(DHT11_PIN);
}

void handleKeypad() {
  char customKey = customKeypad.getKey();
  if (customKey == '*') {
    close_door();
  }
  if (customKey == '#') {
    open_door();
  }
  if (customKey == '0') {
    half_open();
  }
}

void handleRFID() {
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      if (compare_rfid(rfid.serNum, my_rfid)) {
        open_door();
      } else {
        close_door();
        digitalWrite(buzzer, HIGH);
        delay(1000);
        digitalWrite(buzzer, LOW);
      }
    }
    rfid.halt();
  }
}

void sendHttpResponse(WiFiEspClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.print("<html><head><title>Smarthome Control</title>");
  client.print("<meta http-equiv=\"refresh\" content=\"10\">");
  client.print("</head><body>");
  client.print("<h1>Smarthome Control Panel</h1>");

  // Lesson 4: LED Control
  client.print("<h2>LED Control</h2>");
  client.print("The LED is ");
  if (ledStatus == 1)
    client.print("ON");
  else
    client.print("OFF");
  client.println("<p>");
  client.println("Click <a href=\"/H\">here</a> to turn the LED on<br>");
  client.println("Click <a href=\"/L\">here</a> to turn the LED off<br>");

  // Lesson 5: RGB Control
  client.print("<h2>RGB LED Control</h2>");
  client.println("Click <a href=\"/R\">here</a> for Red<br>");
  client.println("Click <a href=\"/G\">here</a> for Green<br>");
  client.println("Click <a href=\"/B\">here</a> for Blue<br>");

  // Lesson 6: Buzzer Control
  client.print("<h2>Buzzer Control</h2>");
  client.println("Click <a href=\"/Z1\">here</a> to turn the Buzzer on<br>");
  client.println("Click <a href=\"/Z0\">here</a> to turn the Buzzer off<br>");

  // Lesson 7: DHT11 Sensor
  client.print("<h2>Environment Status</h2>");
  client.print("Humidity: ");
  client.print(DHT.humidity, 1);
  client.print("%<br>");
  client.print("Temperature: ");
  client.print(DHT.temperature, 1);
  client.print("C<br>");

  // Lesson 9 & 10: Fire/Gas Status
  client.print("<h2>Safety Status</h2>");
  client.print("Fire/Gas Status: ");
  client.print(gasStr);
  client.print("<br>");

  // Lesson 11: Noise Status
  client.print("Noise Status: ");
  client.print(StatusStr);
  client.print("<br>");

  // Lesson 13: Motion Status
  client.print("Motion Status: ");
  client.print(gasStr);
  client.print("<br>");
  
  // Lesson 17: Distance Status
  client.print("Distance Status: ");
  client.print(distance_str);
  client.print("<br>");

  // Lesson 14: LCD Message
  client.print("<h2>Send Message to LCD</h2>");
  client.print("<FORM method=\"GET\" action=\"#\" >Message:<input type=text name=usr><input type=submit name=Submit></form>");

  client.print("</body></html>");
  client.println();
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}

// Lesson 5 Helper
void color(unsigned char red, unsigned char green, unsigned char blue) {
  analogWrite(redPin_RGB, red);
  analogWrite(greenPin_RGB, green);
  analogWrite(bluePin_RGB, blue);
}

// Lesson 8B & 16B Helpers
void open_door() {
  head.attach(SERVO_PIN);
  delay(300);
  head.write(180);
  delay(400);
  head.detach();
  digitalWrite(SERVO_PIN, LOW);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
}

void half_open() {
  head.attach(SERVO_PIN);
  delay(300);
  head.write(90);
  delay(400);
  head.detach();
  digitalWrite(SERVO_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
}

void close_door() {
  head.attach(SERVO_PIN);
  delay(300);
  head.write(0);
  delay(400);
  head.detach();
  digitalWrite(SERVO_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
}

boolean compare_rfid(unsigned char x[], unsigned char y[]) {
  for (int i = 0; i < 5; i++) {
    if (x[i] != y[i]) return false;
  }
  return true;
}

// Lesson 17 Helper
int watch() {
  long echo_distance;
  digitalWrite(Trig_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(Trig_PIN, HIGH);
  delayMicroseconds(15);
  digitalWrite(Trig_PIN, LOW);
  echo_distance = pulseIn(Echo_PIN, HIGH);
  echo_distance = echo_distance * 0.01657;
  return round(echo_distance);
}
