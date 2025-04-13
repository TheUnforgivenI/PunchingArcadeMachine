const int trigRest = 11;
const int echoRest = 12;
const int trigImpact = 9;
const int echoImpact = 10;

float restThreshold = 40.0;      // cm change to detect motion
float impactThreshold = 20.0;   // cm at max deflection
unsigned long punchStartTime = 0;
bool punchDetected = false;
bool punchFinished = false;


void setup() {
  Serial.begin(9600);

  pinMode(trigRest, OUTPUT);
  pinMode(echoRest, INPUT);
  pinMode(trigImpact, OUTPUT);
  pinMode(echoImpact, INPUT);

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

    // Reset for next punch
    punchDetected = false;
    delay(1000); // short cooldown to avoid multiple triggers
  }

  delay(10); // fast refresh loop
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
