#include <PS4Controller.h>

// Motor pins
#define IN1 25 // Motor 1
#define IN2 33 // (-) Motor 1
#define IN3 14 // Motor 2
#define IN4 12 // (-) Motor 2

// Dead zone threshold for joystick
#define DEAD_ZONE 10

// Global variables
int l2Value = 0, turn = 0, r2Value = 0;

// Function prototypes
void moveCar();
void stopCar();
void debugControllerInputs();

// Setup function
void setup() {
  Serial.begin(115200);

  Serial.println("ESP32 PS4 Controller Car");
  // MAC address of ESP32
  PS4.begin("EC:62:60:A7:02:C2");
  Serial.println("Bluetooth Started! Pair your device with 'Group 1 (Roco)'\n");
  Serial.println("Waiting for PS4 controller...\n");

  // Initialize motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

// Loop function
void loop() {
  // Get trigger and joystick values
  r2Value = PS4.data.analog.button.r2; // Range: 0-255
  l2Value = PS4.data.analog.button.l2; // Range: 0-255
  turn = PS4.data.analog.stick.lx;     // Center is 0, Range -127 to 127

  // Debugging
  debugControllerInputs();

  // Process movement
  if (r2Value > 0 || l2Value > 0) {
    moveCar();
  } else {
    stopCar();
  }
}


void moveCar() {
  if (r2Value > 0 && l2Value > 0) {
    // Both triggers pressed: mix forward and backward for turning
    if (turn < -DEAD_ZONE) { // Rotate left
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
    } else if (turn > DEAD_ZONE) { // Rotate right
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else { // Balance both: stop
      stopCar();
    }
  } else if (r2Value > 0) { // Forward logic
    if (turn < -DEAD_ZONE) { // Forward-Left
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW); // Reduce speed on one side
      digitalWrite(IN4, LOW);
    } else if (turn > DEAD_ZONE) { // Forward-Right
      digitalWrite(IN1, LOW); // Reduce speed on one side
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else { // Straight Forward
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    }
  } else if (l2Value > 0) { // Backward logic
    if (turn < -DEAD_ZONE) { // Backward-Left
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW); // Reduce speed on one side
      digitalWrite(IN4, LOW);
    } else if (turn > DEAD_ZONE) { // Backward-Right
      digitalWrite(IN1, LOW); // Reduce speed on one side
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
    } else { // Straight Backward
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
    }
  }
}

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// Function to debug controller inputs
void debugControllerInputs() {
  Serial.print("R2: ");
  Serial.print(r2Value);
  Serial.print(" L2: ");
  Serial.print(l2Value);
  Serial.print(" Turn: ");
  Serial.println(turn);
}
