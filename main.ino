/*
 * Copyright (c) 2016, Amplified IT
 * Copyright (c) 2021, Matthew McAllister
 *
 * Based on http://labs.amplifiedit.com/centipede
 *
 * Published under an MIT License https://opensource.org/licenses/MIT
 */

#include <Keyboard.h>

/* Modify the following definitions to fit your wireless and enrollment credentials. */

#define wifi_name "example" // Define SSID for your wireless connection.
#define wifi_pass "notasecret" // Define the password for your wireless connection.
#define wifi_security 2 // Set to 0 for open, 1 for WEP, 2 for WPA, 3 for EAP
#define username "user@example.com" // Define the user name for enrolling the device.
#define password "notasecret" // The password for the enrollment GAFE account.

// Special characters definition
#define KEY_LEFT_CTRL  0x80
#define KEY_LEFT_SHIFT 0x81
#define KEY_LEFT_ALT   0x82
#define KEY_RIGHT_CTRL 0x84
#define KEY_RIGHT_SHIFT    0x85
#define KEY_RIGHT_ALT  0x86
#define KEY_UP_ARROW   0xDA
#define KEY_DOWN_ARROW 0xD9
#define KEY_LEFT_ARROW 0xD8
#define KEY_RIGHT_ARROW    0xD7
#define KEY_BACKSPACE  0xB2
#define KEY_TAB        0xB3
#define KEY_ENTER 0xB0
#define KEY_ESC        0xB1
#define KEY_CAPS_LOCK  0xC1

int buttonPin = 2;  // Set a button to any pin
int RXLED = 17;
static uint8_t __clock_prescaler = (CLKPR & (_BV(CLKPS0) | _BV(CLKPS1) | _BV(CLKPS2) | _BV(CLKPS3)));

void blink() {
  digitalWrite(RXLED, LOW);
  // TXLED1;
  delay(250);
  digitalWrite(RXLED, HIGH);
  // TXLED0;
  delay(250);
}

void wait(int cycles = 1) {
  for (int i = 0; i < cycles; i++) {
    blink();
  }
}

void setup() {
  setPrescaler();
  Keyboard.begin();
  pinMode(buttonPin, INPUT);  // Set the button as an input
  pinMode(RXLED, OUTPUT);
  digitalWrite(buttonPin, HIGH);
  digitalWrite(RXLED, HIGH);
  wait(16);
}

void loop() {
  wait(4);
	if (digitalRead(buttonPin) == 1) {
	  wifiConfig();
	  agreeToTerms();
    enterEnrollment();
	  TXLED1;
	  wait(5);
	  while (digitalRead(buttonPin) != 1) {
	  	wait(1);
	  }
	  TXLED0;
	  enterCredentials();
	  showSuccess();
	}
  bootLoop();
}

void bootLoop() {
  // digitalWrite(RXLED, LOW);   // set the LED on
  TXLED1; //TX LED is not tied to a normally controlled pin
  delay(200); // wait for a second
  TXLED0;
  delay(200);
  TXLED1; // TX LED is not tied to a normally controlled pin
  delay(200);  // wait for a second
  TXLED0;
  delay(800);
}

void showSuccess() {
  digitalWrite(RXLED, HIGH);    // set the LED off
  while (true) {
    bootLoop();
  }
}

void inputKey(byte key) {
  Keyboard.write(key);
  wait(1);
}

void repeatKey(byte key, int num) {
  for (int i = 0; i < num; i++) {
    Keyboard.write(key);
    wait(1);
  }
}

void enterCredentials() {
  // Give a few seconds for load
  wait(5);

  Keyboard.print(username);
  wait(2);
  Keyboard.write(KEY_ENTER);
  wait(5);
  Keyboard.print(password);
  wait(2);
  Keyboard.write(KEY_ENTER);
  wait(2);

  // Wait for enrollment to complete
  wait(25);

  // Skip additional asset information
  repeatKey(KEY_TAB, 2);
  inputKey(KEY_ENTER);
  wait(2);
  inputKey(KEY_ENTER);
  wait(2);
}

void enterEnrollment() {
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_ALT);
  inputKey('e');
  Keyboard.release(KEY_LEFT_ALT);
  Keyboard.release(KEY_LEFT_CTRL);
  wait(3);
}

void agreeToTerms() {
  // Give a second for page to load
  wait(5);

  // Escape the dreaded terms-of-use prison
  // Keyboard.press(KEY_LEFT_SHIFT);
  // repeatKey(KEY_TAB, 2);
  // Keyboard.release(KEY_LEFT_SHIFT);
  // repeatKey(KEY_TAB, 4);

  // Disable telemetry
  repeatKey(KEY_TAB, 3);
  inputKey(KEY_ENTER);

  // Next screen
  repeatKey(KEY_TAB, 3);
  inputKey(KEY_ENTER);
  wait(1);
  // Crazy workaround for a sporadic bug
  repeatKey(KEY_TAB, 6);
  Keyboard.press(KEY_ENTER);
  wait(1);
  Keyboard.release(KEY_ENTER);

  // Give several seconds for page to load
  wait(25);
}

void wifiConfig() {
  // Navigate to settings
  repeatKey(KEY_TAB, 2);
  inputKey(KEY_ENTER);

  // Navigate to WiFi
  repeatKey(KEY_TAB, 4);
  inputKey(KEY_ENTER);

  // Navigate to "Add network"
  // N.B. This will undershoot if already connected to WiFi
  repeatKey(KEY_TAB, 3);
  inputKey(KEY_ENTER);
  wait(2);

  // Input WiFi config
  Keyboard.print(wifi_name);
  inputKey(KEY_TAB);
  if (wifi_security == 0) {
  	repeatKey(KEY_TAB, 2);
  } else {
  	repeatKey(KEY_DOWN_ARROW, wifi_security);
  	inputKey(KEY_TAB);
    Keyboard.print(wifi_pass);
    wait(1);
  }
  wait(2);
  inputKey(KEY_ENTER);
  wait(2);

  // Delay a second to connect
  wait(5);

  // Skip WiFi setup page
  repeatKey(KEY_TAB, 3);
  inputKey(KEY_ENTER);
  wait(2);
  inputKey(KEY_ENTER);
  wait(2);
}

void setPrescaler() {
  // Disable interrupts.
  uint8_t oldSREG = SREG;
  cli();

  // Enable change.
  CLKPR = _BV(CLKPCE); // write the CLKPCE bit to one and all the other to zero

  // Change clock division.
  CLKPR = 0x0; // write the CLKPS0..3 bits while writing the CLKPE bit to zero

  // Copy for fast access.
  __clock_prescaler = 0x0;

  // Recopy interrupt register.
  SREG = oldSREG;
}
