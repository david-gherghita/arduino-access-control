// API
// void generateRandomChallenge()
// long randomChallenge[CHALLENGE_SIZE]

#define RANDOM_PIN 25
#define CHALLENGE_SIZE 8 // 8 * 4 * 8 = 256 bits

long randomChallenge[CHALLENGE_SIZE];

void generateRandomChallenge() {
  for (uint8_t i = 0; i < CHALLENGE_SIZE; i++) {
    randomChallenge[i] = random(LONG_MAX);
  }
}
