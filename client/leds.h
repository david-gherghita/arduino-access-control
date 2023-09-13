// API
// void blinkOn()
// void blinkOff()

#define LED_PIN 13

void blinkOn() {
  digitalWrite(LED_PIN, HIGH);
}

void blinkOff() {
  digitalWrite(LED_PIN, LOW);
}
