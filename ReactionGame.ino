#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>

/************ WiFi + Leaderboard Settings ************/
const char* HUB_SSID = "ESP32-Access-Point";
const char* HUB_PASS = "123456789";

// Hub endpoint 
const char* HUB_SUBMIT_URL = "http://192.168.4.1/submit";

String playerName = "the best team";

/********************* OLED Setup *********************/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/******************** Pin Mappings ********************/
const int LED_PIN   = 26;
const int START_PIN = 18;
const int REACT_PIN = 14;

/********************* UI Layout *********************/
const int TITLE_X = 31;
const int TITLE_Y = 0;
const int CONTENT_Y = 16;

/******************** Game States ********************/
enum ScreenState { IDLE, WAITING, FALSE_START, GO, RESULT };
ScreenState screenState = IDLE;

/******************** Timing Vars ********************/
unsigned long waitStartMs = 0;
unsigned long waitDelayMs = 0;

unsigned long goStartUs = 0;
unsigned long lastScoreMs = 0;

/******************** OLED Helpers ********************/
void drawTitle() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(TITLE_X, TITLE_Y);
  display.println("Think Fast!");
}

void showIdleScreen() {
  display.clearDisplay();
  drawTitle();

  display.setTextSize(1);
  display.setCursor(0, CONTENT_Y);
  display.println("Press START");

  display.display();
}

void showWaitingScreen() {
  display.clearDisplay();
  drawTitle();

  display.setTextSize(1);
  display.setCursor(0, CONTENT_Y);
  display.println("get ready...");
  display.setCursor(0, CONTENT_Y + 12);
  display.println("wait for LED");

  display.display();
}

void showGoScreen() {
  display.clearDisplay();
  drawTitle();

  display.setTextSize(2);
  display.setCursor(40, CONTENT_Y);
  display.println("GO");

  display.display();
}

void showResultScreen(unsigned long ms) {
  display.clearDisplay();
  drawTitle();

  display.setTextSize(1);
  display.setCursor(0, CONTENT_Y);
  display.println("Your time: ");

  display.setTextSize(2);
  display.setCursor(0, CONTENT_Y + 12);
  display.print(ms);
  display.println(" ms");

  display.setTextSize(1);
  display.setCursor(0, 54);
  display.println("Press START");
  
  

  display.display();
}

void showFalseStartScreen() {
  display.clearDisplay();
  drawTitle();

  display.setTextSize(2);
  display.setCursor(0, CONTENT_Y);
  display.println("You're too");
  display.setCursor(38, CONTENT_Y + 18);
  display.setTextSize(1);
  display.setCursor(0, 54);
  display.println("Press START");
  

  display.display();
}

void connectToHub() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(HUB_SSID, HUB_PASS);

  Serial.print("Connecting to hub");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(200);
    Serial.print(".");
    tries++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! Client IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hub IP: ");
  } else {
    Serial.println("Failed to connect to hub (offline play ok).");
  }
}

void sendScoreToHub(const String& name, uint32_t ms) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(HUB_SUBMIT_URL) + "?name=" + name + "&ms=" + String(ms);

  http.begin(url);
  http.GET();
  http.end();
}

/******************** Button Helpers ********************/
// NOTE: Using INPUT_PULLUP => pressed = LOW
bool startPressed() {
  if (digitalRead(START_PIN) == LOW) {
    delay(15); 
    while (digitalRead(START_PIN) == LOW) { delay(1); } // wait release
    return true;
  }
  return false;
}

bool reactPressed() {
  if (digitalRead(REACT_PIN) == LOW) {
    delay(15);
    while (digitalRead(REACT_PIN) == LOW) { delay(1); }
    return true;
  }
  return false;
}

/******************** Setup ********************/
void setup() {
  Serial.begin(115200);
  connectToHub();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(START_PIN, INPUT_PULLUP);
  pinMode(REACT_PIN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }

  delay(200);
  showIdleScreen();
}

/******************** Main Loop ********************/
void loop() {
  bool s = startPressed();
  bool r = reactPressed();

  switch (screenState) {

    case IDLE:
      digitalWrite(LED_PIN, LOW);


      if (s) {
        waitDelayMs = random(750, 5001);
        waitStartMs = millis();
        showWaitingScreen();
        screenState = WAITING;
      }
      break;

    case WAITING:
      if (r) {
        showFalseStartScreen();
        screenState = FALSE_START;
      }
      else if (millis() - waitStartMs >= waitDelayMs) {
        digitalWrite(LED_PIN, HIGH);
        goStartUs = micros();
        showGoScreen();
        screenState = GO;
      }
      break;

    case FALSE_START:
      if (s) {
        showIdleScreen();
        screenState = IDLE;
      }
      break;

    case GO:
      if (r) {
        unsigned long tUs = micros();
        lastScoreMs = (tUs - goStartUs) / 1000;
        digitalWrite(LED_PIN, LOW);
        showResultScreen(lastScoreMs);

        sendScoreToHub(playerName, lastScoreMs);
        screenState = RESULT;
      }
      break;

    case RESULT:
      if (s) {
        showIdleScreen();
        screenState = IDLE;
      }
      break;
  }
}
