#include <WiFi.h>


//
// IMPORTANT: compile with "ESP32 Dev Module, disabled, Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS),
// 240MHz (WiFi/BT), QIO, 80MHz, 4MB (32Mb) 921600, None of /dev/ttyUSB0
//
// debug log, set to 1 to enable
#define ENABLE_DEBUG_LOG 1

// wifi config
//const char* ssid     = "Pete’s iPhone";
const char* ssid = "BTWholeHome-68S";
//const char* password = "7tf9qz9dvcx0p";
const char* password = "U7VyTaKhKaNL";

// ethernet config
//const IPAddress local_IP(172, 20, 10, 5);
//const IPAddress gateway(172, 20, 10, 1);
//const IPAddress subnet(255, 255, 255, 240);

const IPAddress local_IP(192, 168, 0, 3);
const IPAddress gateway(192, 168, 0, 1);
const IPAddress subnet(255, 255, 255, 0);

const IPAddress primaryDNS(8, 8, 8, 8);
const IPAddress secondaryDNS(8, 8, 4, 4);

// rs-server config
const int serverPort = 1234;

// rs port config
const int baudrate = 115200;
const int rs_config = SERIAL_8N1;

// reading buffor config
#define BUFFER_SIZE 1024

// global objects
WiFiServer server;
HardwareSerial TecSerial(1);

const int fan1 = 13;
const int fan2 = 12;

const int fan_freq = 25000;
const int fan1_channel = 0;
const int fan2_channel = 1;
const int fan_resolution = 8;

byte buff[BUFFER_SIZE];

void debug_log(char* str) {
#if ENABLE_DEBUG_LOG == 1
  Serial.println(str);
#endif
}

void setup() {
  // init rs port
  Serial.begin(115200);
  debug_log("qegin setup");

  ledcSetup(fan1_channel, fan_freq, fan_resolution);
  ledcSetup(fan2_channel, fan_freq, fan_resolution);

  ledcAttachPin(fan1, fan1_channel);
  ledcAttachPin(fan2, fan2_channel);
  
  TecSerial.begin(38400, SERIAL_8N1, 3, 1); // receive and transmit on pins labelled RX0/TX0

  // init wifi connection
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    debug_log("Failed to configure network settings");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    debug_log("connecting to WiFi network");
    delay(500);
  }

#if ENABLE_DEBUG_LOG == 1
  Serial.println("connected to WiFi");
  Serial.println("IP adddr: ");
  Serial.println(WiFi.localIP());
#endif
  delay(1000);

  //start server
  server = WiFiServer(serverPort);
  server.begin();
  delay(1000);
  debug_log("server started");

  ledcWrite(fan1_channel, 192);
  ledcWrite(fan2_channel, 192);
}

void loop() {
  // wait for client
  WiFiClient client = server.available();
  if (!client)
    return;

  debug_log("client found");
  while (client.connected()) {
    int size = 0;

    // read data from wifi client and send to serial
    while (size = client.available()) {
      size = (size >= BUFFER_SIZE ? BUFFER_SIZE : size);
      client.read(buff, size);
      TecSerial.write(buff, size);
      TecSerial.flush();
    }

    // read data from serial and send to wifi client
    while (size = TecSerial.available()) {
      size = (size >= BUFFER_SIZE ? BUFFER_SIZE : size);
      TecSerial.readBytes(buff, size);
      client.write(buff, size);
      client.flush();
    }
  }
  debug_log("client disconnected");
  client.stop();
}
