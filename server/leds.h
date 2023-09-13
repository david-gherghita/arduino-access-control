// API
// void blinkGreen()
// void blinkRed()

#define LED_GREEN 32
#define LED_RED 33

void blinkGreen() {
  digitalWrite(LED_GREEN, HIGH);
  delay(2000);
  digitalWrite(LED_GREEN, LOW);
}

void blinkRed() {
  digitalWrite(LED_RED, HIGH);
  delay(2000);
  digitalWrite(LED_RED, LOW);
}
