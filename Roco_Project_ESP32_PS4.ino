/**
 * ----------------------------------------------------------------------------
 * @file        Roco_Project_ESP32_PS4.ino
 * @author      Mohamed Tarek
 * @date        03/03/2025
 * @version     2.1
 * @brief       Remote control car system using ESP32 and PS4 Controller.
 * 
 * @description
 *              Remote control car system utilizing an ESP32 and PS4 Controller.
 *              
 *              Features: 
 *              - Speed control
 *              - Boost mode
 *              - LED feedback
 *              - Haptic feedback (Rumble)
 * 
 * @usage
 *              1. Update the MAC address in the code to match your PS4 controller.
 *              2. Ensure the PS4 controller is powered.
 * 
 * @note
 *              Make sure to install the 'PS4Controller' library from the 
 *              Arduino Library Manager.
 * ----------------------------------------------------------------------------
 */

#include <PS4Controller.h>
#include <math.h>

// Motor pins
#define ENA 19 // (PWM ENA)
#define ENB 18 // (PWM ENB)
#define IN1 13 // Motor 1
#define IN2 32 // (-) Motor 1
#define IN3 26 // Motor 2
#define IN4 27 // (-) Motor 2

// Dead zone for joystick
#define DEAD_ZONE 10

// Global variables
int l2Value = 0, r2Value = 0, turn = 0;
int forwardSpeed = 255, backwardSpeed = 255, turnSpeed = 0, rotateSpeed = 255;
int targetForwardSpeed = 255, targetBackwardSpeed = 255; // New: Target speeds
int currentForwardSpeed = 0, currentBackwardSpeed = 0;   // New: Smooth speeds

bool cross = false, r1 = false, l1 = false, lastCrossState = false;
bool isRotate = false, isSpeedControl = false, isCarStop = true, isBoostMode = false, isRumble = false, isSend = false, isIdle = false, wasBoosting = false, isBoostCooldown = false;

// Cooldowns
unsigned long lastUpdate = 0, lastUpdate2 = 0, lastSmoothUpdate = 0, boostStartTime = 0, lastBoostEndTime = 0;
const unsigned long interval = 10; // 10ms
const unsigned long boostDuration = 3000; // 3 seconds
const unsigned long boostCooldown = 10000; // 10 seconds cooldown
const unsigned long smoothInterval = 10;  // 10ms for speed smoothing

bool isFlashing = false;
bool hasFlashedForBoost = false;
unsigned long flashStartTime = 0;
int flashCount = 0;
uint8_t flashColorR = 0;
uint8_t flashColorG = 0;
uint8_t flashColorB = 0;
bool flashState = false;

int flashMaxCount = 6;   // default 3 flashes (on + off = 6 steps)
int flashInterval = 100; // default 100ms per state


unsigned long rumbleStartTime = 0;
bool rumbleInProgress = false;
bool rumblePhaseTwo = false;
uint8_t rumbleStrengthL = 0;
uint8_t rumbleStrengthR = 0;
unsigned long rumbleDelay = 250;


// Smooth control tuning
const int accelerationStep = 5; // How much to change speed each update (higher = faster response)

// Function prototypes
void moveCar();
void stopCar();
void debugControllerInputs();
void updateControllerEffects();
void handleRumble();
void handleLed();
void sendDataToController();
void speedControl();
void boostMode();
void blinkLed(int times, int delayMs, int r, int g, int b);
void smoothSpeedUpdate();
void handleFlashing();
void startFlashing(uint8_t r, uint8_t g, uint8_t b, int numFlashes = 3, int speed = 100);
void doubleRumble(uint8_t strengthL, uint8_t strengthR, unsigned long delayTime);

// Setup function
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 PS4 Controller Car");
  // MAC address of ESP32
  PS4.begin("00:11:22:33:44:55");
  Serial.println("Waiting for PS4 controller...");

  // Initialize motor pins
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT);

  for (int MT = 0; MT < 3; MT++) {
    stopCar();
  }
}

// Loop function
void loop() {
  // Get trigger and joystick values
  r2Value = PS4.data.analog.button.r2; // Range: 0-255
  l2Value = PS4.data.analog.button.l2; // Range: 0-255
  turn = PS4.data.analog.stick.lx;     // Center is 0, Range -127 to 127
  cross = PS4.event.button_down.cross;
  r1 = PS4.data.button.r1;
  l1 = PS4.data.button.l1;

  isCarStop = !(r2Value > 0 || l2Value > 0);
  isIdle = !(isRotate || isBoostMode || isSpeedControl);

  updateControllerEffects();
  speedControl();
  smoothSpeedUpdate();

  // Debugging
  // debugControllerInputs();

  // Process movement
  if (r2Value > 0 || l2Value > 0) {
    moveCar();
  } else {
    stopCar();
  }
}

// Motor Control
void moveCar() {
  if (r2Value > 0 && l2Value > 0) {
    // Both triggers pressed: mix forward and backward for turning
    if (turn < -DEAD_ZONE) { // Rotate left
      analogWrite(ENA, rotateSpeed);
      analogWrite(ENB, rotateSpeed);
      digitalWrite(IN1, HIGH); 
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);  
      digitalWrite(IN4, HIGH);
    } else if (turn > DEAD_ZONE) { // Rotate right
      analogWrite(ENA, rotateSpeed);
      analogWrite(ENB, rotateSpeed);
      digitalWrite(IN1, LOW);  
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else { // Balance both: stop
      stopCar();
    }
  } else if (r2Value > 0) { // Forward logic
    if (turn < -DEAD_ZONE) { // Forward-Left
      analogWrite(ENA, currentForwardSpeed);
      analogWrite(ENB, constrain(currentForwardSpeed - turnSpeed, 0, 255));
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW); // Reduce speed on one side
      digitalWrite(IN4, LOW);
    } else if (turn > DEAD_ZONE) { // Forward-Right
      analogWrite(ENA, constrain(currentForwardSpeed - turnSpeed, 0, 255));
      analogWrite(ENB, currentForwardSpeed);
      digitalWrite(IN1, LOW); // Reduce speed on one side
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
    } else { // Straight Forward
      analogWrite(ENA, currentForwardSpeed);
      analogWrite(ENB, currentForwardSpeed);
      digitalWrite(IN1, HIGH); 
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH); 
      digitalWrite(IN4, LOW);
    }
  } else if (l2Value > 0) { // Backward logic
    if (turn < -DEAD_ZONE) { // Backward-Left
      analogWrite(ENA, currentBackwardSpeed);
      analogWrite(ENB, constrain(currentBackwardSpeed - turnSpeed, 0, 255));
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW); // Reduce speed on one side
      digitalWrite(IN4, LOW);
    } else if (turn > DEAD_ZONE) { // Backward-Right
      analogWrite(ENA, constrain(currentBackwardSpeed - turnSpeed, 0, 255));
      analogWrite(ENB, currentBackwardSpeed);
      digitalWrite(IN1, LOW); // Reduce speed on one side
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
    } else { // Straight Backward
      analogWrite(ENA, currentBackwardSpeed);
      analogWrite(ENB, currentBackwardSpeed);
      digitalWrite(IN1, LOW); 
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW); 
      digitalWrite(IN4, HIGH);
    }
    
  }
}

void stopCar() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
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

// Controller Effects
void updateControllerEffects() {
  isRotate = (r2Value > 0 && l2Value > 0);
  isRumble = (isRotate || isBoostMode);

  handleRumble();
  handleLed();
  handleFlashing();
}

// Speed Control
void speedControl() {
  if (cross && !lastCrossState && isCarStop && !isBoostMode) {
    isSpeedControl = !isSpeedControl;
    Serial.println(isSpeedControl ? "Speed Control: Enabled" : "Speed Control: Disabled");

    blinkLed(3, 100, 0, 255, 0); // Blink Green 3 times

    if (!isSpeedControl) {
      forwardSpeed = 255;
      backwardSpeed = 255;
      turnSpeed = 0;
      rotateSpeed = 255;
    }
  }

  if (isSpeedControl && !isBoostMode) {
    targetForwardSpeed = r2Value;
    targetBackwardSpeed = l2Value;
    turnSpeed = map(abs(turn), 0, 127, 0, 255);
    rotateSpeed = abs(turn);
  } else {
    targetForwardSpeed = 255;
    targetBackwardSpeed = 255;
  }

  if (isSpeedControl) {
    boostMode();
  }

  // Save current state for next check
  lastCrossState = cross;
}

// Boost Mode
void boostMode() {
  unsigned long currentMillis = millis();

  // Start Boost if R1 + L1 pressed, car stopped, boost ready
  if (r1 && l1 && !isBoostMode && !isBoostCooldown) {
    isBoostMode = true;
    hasFlashedForBoost = false; // Reset flash protection
    boostStartTime = currentMillis;
    Serial.println("Boost Mode Activated!");
  }

  // Handle Flashing only ONCE when pressing R1 + L1
  if (r1 && l1 && isBoostCooldown && !hasFlashedForBoost && !isBoostMode) {
    // Boost NOT ready
    startFlashing(255, 0, 0, 2, 70); // Red, 2 fast flashes, 70ms each
    hasFlashedForBoost = true;
}

  if (!r1 || !l1) {
    // Reset when buttons are released
    hasFlashedForBoost = false;
  }

  // Handle Boost Active
  if (isBoostMode) {
    if (currentMillis - boostStartTime <= boostDuration) {
      targetForwardSpeed = min(r2Value * 2, 255);
      targetBackwardSpeed = min(l2Value * 2, 255);
      turnSpeed = min((int)map(abs(turn), 0, 127, 0, 255) * 2, 255);
      rotateSpeed = abs(turn);
    } else {
      isBoostMode = false;
      isBoostCooldown = true;
      lastBoostEndTime = currentMillis;
      wasBoosting = true;
      Serial.println("Boost Mode Ended. Cooldown started.");
    }
  }

  // Handle Cooldown expiration
  if (isBoostCooldown && (currentMillis - lastBoostEndTime >= boostCooldown)) {
    // Boost READY
    isBoostCooldown = false;
    Serial.println("Boost Cooldown Finished. Ready again.");
    doubleRumble(0x80, 0x20, 250); // (intensity 150, 100ms each rumble)
    startFlashing(255, 140, 0, 3, 100); // Orange, 3 normal flashes, 100ms each
  }
}

// Smooth acceleration / deceleration
void smoothSpeedUpdate() {
  unsigned long now = millis();
  if (now - lastSmoothUpdate >= smoothInterval) {
    lastSmoothUpdate = now;

    if (currentForwardSpeed < targetForwardSpeed) {
      currentForwardSpeed = min(currentForwardSpeed + accelerationStep, targetForwardSpeed);
    } else if (currentForwardSpeed > targetForwardSpeed) {
      currentForwardSpeed = max(currentForwardSpeed - accelerationStep, targetForwardSpeed);
    }

    if (currentBackwardSpeed < targetBackwardSpeed) {
      currentBackwardSpeed = min(currentBackwardSpeed + accelerationStep, targetBackwardSpeed);
    } else if (currentBackwardSpeed > targetBackwardSpeed) {
      currentBackwardSpeed = max(currentBackwardSpeed - accelerationStep, targetBackwardSpeed);
    }
  }
}

void handleLed() {
  static unsigned long lastMillis = 0;
  static float phase = 0.0;

  if (!isFlashing) {
    if (isRotate) {
      PS4.setLed(255, 153, 0);
      isSend = true;
    } else if (isBoostMode && !isRotate) {
      unsigned long now = millis();
      float dt = (now - lastMillis) / 1000.0;
      lastMillis = now;

      phase += dt * 2.0 * PI * 1.5;
      if (phase > TWO_PI) phase -= TWO_PI;

      float brightness = (sin(phase) + 1.0) / 2.0;
      int ledBrightness = (int)(brightness * 255);
      PS4.setLed(ledBrightness, ledBrightness * 0.4, 0);
      isSend = true;
    } else if (isSpeedControl) {
      PS4.setLed(0, 255, 0);
      isSend = true;
    } else if (isIdle) {
      PS4.setLed(0, 255, 255);
      isSend = true;
    }
  
    if (isSend) {
      sendDataToController();
      isSend = false;
    }
  }
}  

void handleRumble() {
  static unsigned long fadeStartMillis = 0;
  static int lastRumble = -1;
  static bool isFadingOut = false;
  static bool wasRotating = false;
  static bool wasBoostingBefore = false; // <--- NEW

  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= interval) {
    lastUpdate = currentMillis;

    int rumbleIntensity = 0;

    if (isRotate) {
      // Rotating (priority)
      int avgTrigger = (r2Value + l2Value) / 2;
      rumbleIntensity = min(avgTrigger, 150);
      lastRumble = rumbleIntensity;
      isFadingOut = false;
      wasRotating = true;
      isSend = true;
    }
    else if (isBoostMode) {
      // Boosting (rev engine while boosting)
      float phase = (millis() % 1000) / 1000.0 * TWO_PI; // 1 second cycle
      float rev = (sin(phase) + 1.0) / 2.0; // smooth oscillation
      rumbleIntensity = 100 + (int)(rev * 100); // Rev from 100 to 200
      lastRumble = rumbleIntensity;
      isFadingOut = false;
      wasBoostingBefore = true;
      isSend = true;
    }
    else if (wasRotating) {
      // Fade out after rotating
      if (!isFadingOut) {
        fadeStartMillis = currentMillis;
        isFadingOut = true;
      }
      int fadeProgress = currentMillis - fadeStartMillis;
      rumbleIntensity = map(fadeProgress, 0, 2000, lastRumble, 0);
      rumbleIntensity = constrain(rumbleIntensity, 0, 255);
      isSend = true;
      if (fadeProgress >= 2000) {
        rumbleIntensity = 0;
        isFadingOut = false;
        wasRotating = false;
        isSend = true;
      }
    }
    else if (wasBoostingBefore) {
      // Fade out after boost ends
      if (!isFadingOut) {
        fadeStartMillis = currentMillis;
        isFadingOut = true;
      }
      int fadeProgress = currentMillis - fadeStartMillis;
      float normalized = fadeProgress / 2000.0;
      float engineCurve = 1.0 - (normalized * normalized); // quadratic decay
      rumbleIntensity = (int)(engineCurve * 200);
      rumbleIntensity = constrain(rumbleIntensity, 0, 200);
      isSend = true;

      if (fadeProgress >= 2000) {
        rumbleIntensity = 0;
        isFadingOut = false;
        wasBoostingBefore = false;
        isSend = true;
      }
    }
    else {
      rumbleIntensity = 0; // Safe fallback
    }

    if (isSend) {
      PS4.setRumble(rumbleIntensity, 0);
      sendDataToController();
      isSend = false;
    }
  }
}

void doubleRumble(uint8_t strengthL = 0xFF, uint8_t strengthR = 0xFF, unsigned long delayTime = 250) {
    if (!rumbleInProgress) {
        PS4.setRumble(strengthL, strengthR);  // Start first rumble
        rumbleStartTime = millis();
        rumbleInProgress = true;
        rumblePhaseTwo = false;
        rumbleStrengthL = strengthL;
        rumbleStrengthR = strengthR;
        rumbleDelay = delayTime;
    } else {
        unsigned long currentTime = millis();
        if (!rumblePhaseTwo && (currentTime - rumbleStartTime >= rumbleDelay)) {
            PS4.setRumble(0x00, 0x00);  // Stop first rumble
            rumbleStartTime = currentTime;
            rumblePhaseTwo = true;
        } else if (rumblePhaseTwo && (currentTime - rumbleStartTime >= rumbleDelay)) {
            PS4.setRumble(rumbleStrengthL, rumbleStrengthR);  // Start second rumble
            rumbleStartTime = currentTime;
            rumblePhaseTwo = false;
        } else if (!rumblePhaseTwo && (currentTime - rumbleStartTime >= rumbleDelay)) {
            PS4.setRumble(0x00, 0x00);  // Stop second rumble
            rumbleInProgress = false;   // Sequence done
        }
    }
}

// LED Blinking for feedback
void blinkLed(int times, int delayMs, int r, int g, int b) {
  for (int i = 0; i < times; i++) {
    PS4.setLed(r, g, b);
    sendDataToController();
    delay(delayMs);
    PS4.setLed(0, 0, 0);
    sendDataToController();
    delay(delayMs);
  }
}

void startFlashing(uint8_t r, uint8_t g, uint8_t b, int numFlashes, int speed) {
  flashColorR = r;
  flashColorG = g;
  flashColorB = b;
  flashStartTime = millis();
  flashCount = 0;
  flashState = false;
  isFlashing = true;
  flashMaxCount = numFlashes * 2; // because ON + OFF counts
  flashInterval = speed;
}

void handleFlashing() {
  if (!isFlashing) return;

  unsigned long currentMillis = millis();

  if (currentMillis - flashStartTime >= flashInterval) {
    flashStartTime = currentMillis;
    flashState = !flashState;

    if (flashState) {
      // LED ON
      PS4.setLed(flashColorR, flashColorG, flashColorB);
      if (flashCount == 0) {
        // Only rumble during first ON
        PS4.setRumble(150, 150);
      } else {
        PS4.setRumble(0, 0);
      }
    } else {
      // LED OFF
      PS4.setLed(0, 0, 0);
      PS4.setRumble(0, 0);
      flashCount++;
    }
    sendDataToController();

    if (flashCount >= flashMaxCount) {
      isFlashing = false;
    }
  }
}

// Send Data To Controller
void sendDataToController() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate2 >= interval) {
    lastUpdate2 = currentMillis;
    PS4.sendToController();
  }
}