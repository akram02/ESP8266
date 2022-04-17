#include<ESP8266WiFi.h>

const char* ssid = "Limited";
const char* pass = "*****";

WiFiServer server(80);

String header;

String state = "Off";

const int pin = 14;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if(client) {
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while(client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if(client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if(c=='\n') {
          if(currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if(header.indexOf("GET /on") >= 0) {
              state = "On";
              digitalWrite(pin, HIGH);
            }
            else if(header.indexOf("GET /off") >= 0) {
              state = "Off";
              digitalWrite(pin, LOW);
            }
            
            client.println("<head><link rel=\"icon\" href=\"data:,\"></head>");
            
            client.println("Pin - State " + state);
            if (state=="Off") {
              client.println("<a href=\"/on\"><button>ON</button></a>");
            } else {
              client.println("<a href=\"/off\"><button>OFF</button></a>");
            } 


            break;
          }
          else {
            currentLine = "";
          }
        } else if( c!='\r') {
          currentLine += c;
        }
      }
    }

    header = "";
    client.stop();
  }
}
