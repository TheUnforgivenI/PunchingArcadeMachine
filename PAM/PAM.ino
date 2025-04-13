#include <LedControl.h>

const int trigRest = 11;
const int echoRest = 12;
const int trigImpact = 9;
const int echoImpact = 10;

float restThreshold = 40.0;      // cm change to detect motion
float impactThreshold = 20.0;   // cm at max deflection
unsigned long punchStartTime = 0;
bool punchDetected = false;
bool punchFinished = false;

const byte digitFont[10][5] = {
  {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
  {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
  {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
  {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
  {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
  {0x06, 0x49, 0x49, 0x29, 0x1E}  // 9
};

// Create LedControl object: DIN, CLK, CS, number of devices
LedControl lc = LedControl(2, 3, 4, 4); // adjust pins/devices if needed

void initMatrix() {
  for (int i = 0; i < 4; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 1); // brightness: 0 (dim) to 15 (bright)
    lc.clearDisplay(i);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(trigRest, OUTPUT);
  pinMode(echoRest, INPUT);
  pinMode(trigImpact, OUTPUT);
  pinMode(echoImpact, INPUT);
  initMatrix();
  delay(1000); // Give sensors time to settle
  Serial.println("Ready to detect punch timing...");
}

void loop() {
  float restDistance = readDistance(trigRest, echoRest);
  float impactDistance = readDistance(trigImpact, echoImpact);

  // Detect punch start - bag moving away from resting sensor
  if (!punchDetected && restDistance > restThreshold) {
    punchStartTime = millis();
    punchDetected = true;
    Serial.println("Punch started!");
  }

  // Detect impact - bag reaching max fold (close to pipe)
  if (punchDetected && impactDistance < impactThreshold) {
    unsigned long impactTime = millis();
    unsigned long punchDuration = impactTime - punchStartTime;
    Serial.print("Punch impact time: ");
    Serial.print(punchDuration);
    Serial.println(" ms");
    displayScore(punchDuration);
    // Reset for next punch
    punchDetected = false;
    delay(1000); // short cooldown to avoid multiple triggers
  }

  delay(10); // fast refresh loop
}

void clearMatrix() {
  for (int i = 0; i < 4; i++) {
    lc.clearDisplay(i);
  }
}

void displayScore(unsigned long score) {
  clearMatrix();

  char buffer[5];
  snprintf(buffer, 5, "%4lu", score); // right-aligned up to 4 digits

  int col = 23; // Start from right side of display (column 31)
  for (int i = 0; i < 4; i++) {
    char c = buffer[i];

    if (c == ' ') {
      col -= 6; // skip space
      continue;
    }

    int digit = c - '0';
    for (int j = 0; j < 5; j++) {
      byte columnData = digitFont[digit][j];
      drawColumn(col--, columnData);
    }

    col--; // space between digits
  }
}

// Maps a byte of column data (1 byte = 8 vertical LEDs) to the matrix
void drawColumn(int col, byte data) {
  if (col < 0 || col > 31) return;

  int device = col / 8;         // which 8x8 module (0–3)
  int colInDevice = col % 8;    // column within that module (0–7)

  // Write each bit to the appropriate row
  for (int row = 0; row < 8; row++) {
    bool pixelOn = bitRead(data, row);
    lc.setLed(device, row, 7 - colInDevice, pixelOn); // reverse col if needed
  }
}

float readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  if (duration == 0) return -1; // no echo
  return duration * 0.0343 / 2;
}
