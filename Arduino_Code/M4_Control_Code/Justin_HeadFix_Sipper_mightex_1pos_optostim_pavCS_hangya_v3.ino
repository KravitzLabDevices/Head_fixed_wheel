/*
  Justin Head Fix 5 Port Sipper device
  Kravitz Lab
  March 2023
*/

#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define BUTTON_A 9
#define BUTTON_B 6

// ============ LASER CONFIGURATION ============
// Set to true to enable laser trials, false to disable
#define LASER_ENABLED false
#define TONE_STIM false             // Tone-triggered laser stimulation
#define LASER_CLOSED_LOOP false     // Closed-loop 20Hz pulsed laser stimulation
#define MULTI_STIM false             // Multi-burst stimulation mode
                                    // NOTE: TONE_STIM, LASER_CLOSED_LOOP, and MULTI_STIM are mutually exclusive
#define LICK_BOUT_THRESHOLD 1       // Minimum licks to constitute a bout
#define LICK_BOUT_WINDOW 1000       // Time window in ms (1 second)
#define LASER_DELAY_AFTER_BOUT 0    // Delay after bout detection in ms
#define LASER_PULSE_DURATION 5000   // Laser pulse duration in ms (not used in closed-loop or multi-stim mode)

// Closed-loop laser parameters
#define MODULATION_TIME 1000        // Duration of laser modulation window in ms (resets on lick)
#define PULSE_ON_TIME 10            // Laser on duration per pulse in ms (20Hz = 10ms on, 40ms off)
#define PULSE_OFF_TIME 40           // Laser off duration per pulse in ms

// Multi-stim laser parameters
#define NUM_BURST 5                 // Number of stimulation bursts
#define BURST_DURATION 5000         // Duration of each burst in ms (5 seconds)
#define INTER_BURST_DURATION 15000  // OFF time between bursts in ms (15 seconds)
// ============================================

// ============ TRIAL TIMING PARAMETERS ============
#define LOCKOUT_MIN 1.5               // Minimum lockout duration in seconds
#define LOCKOUT_MAX 1.5               // Maximum lockout duration in seconds
#define LOCKOUT_LAMBDA 0.2            // Lambda parameter for exponential distribution (adjust for shape)
#define ITI_MIN 2.0                   // Minimum ITI duration in seconds
#define ITI_MAX 4.0                  // Maximum ITI duration in seconds
#define TONE_DURATION 2000            // Tone duration in milliseconds
#define TONE_FREQUENCY 8000           // Tone frequency in Hz
#define TONE_TO_DISPENSE_DELAY 500   // Delay from tone offset to initial dispense in ms

//============ TRIAL TIMING PARAMETERS ============ 
const int trialDuration = 10;  // Trial duration in seconds

// ============ BEHAVIORAL MODE CONFIGURATION ============
#define SELF_PACED false              // Set to true to enable lick-triggered dispense, false for initial dispense only
#define PURE_SELF_PACED false         // Set to true to enable pure self-paced mode (bypasses all trial structure)
// =======================================================

// ============ DISCRIMINATION TASK PARAMETERS ============
#define REWARD_TONE_FREQ 8000        // Reward tone frequency (Hz)
#define PUNISHMENT_TONE_FREQ 12000     // Punishment tone frequency (Hz)
int numPulses = 0; 
#define AIR_PUFF_DURATION 100        // Air puff duration (ms)
#define AIR_PUFF_PIN A1               // Pin for air puff solenoid

// ============ POSITION SHUFFLE CONFIGURATION ============
#define POSITION_SHUFFLE_ENABLED false  // Set to true to enable position shuffling between trials
// Change this for restricting the number of active positions
#define NUM_COUNT 1
int numbers[NUM_COUNT] = {2};
int pickedNumbers[NUM_COUNT];
int lastPickedIndex = -1;
// ========================================================

// Function to generate truncated exponential random variable
float truncatedExponential(float minVal, float maxVal, float lambda) {
  float range = maxVal - minVal;
  float sample;
  
  do {
    // Generate uniform random number between 0 and 1
    float u = random(10000) / 10000.0;
    
    // Transform to exponential distribution: X = -ln(1-U)/lambda
    sample = -log(1.0 - u) / lambda;
    
  } while (sample > range); // Reject if outside truncated range
  
  return minVal + sample;
}

// Global variable declarations
int random_delay = 0;
unsigned long startTimeout = 0; 
bool isToneActive = false;

// OLED display object
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

// User-specified volume in uL
int volume = 2;
int Trial = 1; // Starting from the first trial

Servo servo_retract;  // Create servo object to control a servo
Servo servo_rotate;   // Create servo object to control a servo

int pos = 0;  // Variable to store the servo position
int offset = 0;
unsigned long timer = 0;
int currentServoPos = 120;  // Track current servo position

int selection = 2;  // Default spout selection for dispensing
bool isDispensing = false; // Flag to prevent repeated dispenses
unsigned long lastDispenseTime = 0; // Tracks the last dispense time

int toneStartTime = 0;
int lockStartTime = 0;

// Function prototypes
void UpdateDisplay(bool inLockout = false, float lockoutRemainingTime = 0, bool inITI = false, bool inTone = false, bool inPreDispense = false);
void UpdateDisplayPureSelfPaced(unsigned long lickCount);
void dispense(int selection, bool updateDisplay);
void spoutPosition(int selection);
void playTone(int frequency, int duration);
void sendTrialSignal(int numPulses);
void lockout();
int positionShuffle();
void checkbuttons();
void startup();
void extend(int selection, bool updateDisplayDuringExtend = false, unsigned long* toneStartTime = nullptr, unsigned long toneDuration = 0);
void retract();
void runITI();
volatile bool lickDetectedFlag = false;
volatile bool inITIPhase = false;
volatile unsigned long lickDetectionTime = 0;  // NEW: Store when lick was detected


void lickISR() {
  lickDetectedFlag = true;    
  lickDetectionTime = millis();  // Capture exact detection time
  #if !PURE_SELF_PACED
    if (inITIPhase) {  // Only set pin 1 HIGH during ITI in standard mode
      digitalWrite(1, HIGH);
    }
  #endif
}

void setup() {
  Serial.begin(9600);
  servo_retract.attach(11);  // Attaches the servo on pin 11
  servo_rotate.attach(10);   // Attaches the servo on pin 10

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(4, INPUT);
  attachInterrupt(digitalPinToInterrupt(4), lickISR, RISING);
  pinMode(5, OUTPUT);    
  digitalWrite(5, LOW);
  pinMode(1, OUTPUT);
  digitalWrite(1, LOW);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(AIR_PUFF_PIN, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(25, OUTPUT);
  digitalWrite(13, LOW);
  
  digitalWrite(25, LOW); // Ensure laser starts LOW

  analogWriteResolution(8); // Setting resolution for PWM modulation at 8-bit

  Wire.begin(); // Start the I2C communication

  delay(250); // Wait for the OLED to power up
  display.begin(0x3C, true); // Address 0x3C default
  display.clearDisplay();
  display.setRotation(1);
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.display();

  spoutPosition(3); // Set spout to position 3

  // timer = millis();
  // while (millis() - timer < 5000) {
  //   checkbuttons(); // Use buttons to set offset
  // }

  startup(); // Setup button menu

  selection = positionShuffle(); // Initialize spout position via positionShuffle
  spoutPosition(selection);
  extend(selection);

  // Print mode status at startup
  if (PURE_SELF_PACED) {
    Serial.println("*** PURE SELF-PACED MODE ENABLED ***");
    Serial.println("All trial structure bypassed - continuous lick-triggered dispense");
    Serial.print("Spout position: ");
    Serial.println(selection);
  } else {
    // Print laser status at startup
    if (LASER_ENABLED) {
      if (TONE_STIM) {
        Serial.println("*** LASER MODE ENABLED - TONE STIMULATION ***");
        Serial.println("Laser will trigger with tone onset for full trial duration");
      } else if (LASER_CLOSED_LOOP) {
        Serial.println("*** LASER MODE ENABLED - CLOSED-LOOP 20Hz PULSED ***");
        Serial.print("Lick bout threshold: ");
        Serial.print(LICK_BOUT_THRESHOLD);
        Serial.print(" licks within ");
        Serial.print(LICK_BOUT_WINDOW);
        Serial.println(" ms");
        Serial.print("Laser delay after bout: ");
        Serial.print(LASER_DELAY_AFTER_BOUT);
        Serial.println(" ms");
        Serial.print("Modulation window: ");
        Serial.print(MODULATION_TIME);
        Serial.println(" ms (resets on lick)");
        Serial.print("Pulse parameters: ");
        Serial.print(PULSE_ON_TIME);
        Serial.print("ms ON, ");
        Serial.print(PULSE_OFF_TIME);
        Serial.println("ms OFF (20Hz)");
      } else if (MULTI_STIM) {
        Serial.println("*** LASER MODE ENABLED - MULTI-BURST 20Hz PULSED ***");
        Serial.print("Number of bursts: ");
        Serial.println(NUM_BURST);
        Serial.print("Burst duration: ");
        Serial.print(BURST_DURATION);
        Serial.println(" ms");
        Serial.print("Inter-burst interval: ");
        Serial.print(INTER_BURST_DURATION);
        Serial.println(" ms");
        Serial.print("Pulse parameters: ");
        Serial.print(PULSE_ON_TIME);
        Serial.print("ms ON, ");
        Serial.print(PULSE_OFF_TIME);
        Serial.println("ms OFF (20Hz)");
      } else {
        Serial.println("*** LASER MODE ENABLED - LICK BOUT TRIGGERED ***");
        Serial.print("Lick bout threshold: ");
        Serial.print(LICK_BOUT_THRESHOLD);
        Serial.print(" licks within ");
        Serial.print(LICK_BOUT_WINDOW);
        Serial.println(" ms");
        Serial.print("Laser delay after bout: ");
        Serial.print(LASER_DELAY_AFTER_BOUT);
        Serial.println(" ms");
      }
    } else {
      Serial.println("*** LASER MODE DISABLED ***");
    }

    // Print behavioral mode status
    if (SELF_PACED) {
      Serial.println("*** BEHAVIORAL MODE: SELF-PACED (lick-triggered dispense enabled) ***");
    } else {
      Serial.println("*** BEHAVIORAL MODE: FIXED DISPENSE (initial dispense only) ***");
    }

    // Print position shuffle status
    if (POSITION_SHUFFLE_ENABLED) {
      Serial.println("*** POSITION SHUFFLE ENABLED ***");
      Serial.print("Active positions: ");
      for (int i = 0; i < NUM_COUNT; i++) {
        Serial.print(numbers[i]);
        if (i < NUM_COUNT - 1) Serial.print(", ");
      }
      Serial.println();
    } else {
      Serial.println("*** POSITION SHUFFLE DISABLED ***");
      Serial.print("Fixed position: ");
      Serial.println(selection);
    }
  }
}

void loop() {
  // ===== PURE SELF-PACED MODE =====
  if (PURE_SELF_PACED) {
  
    static bool initialDispenseComplete = false;
    static unsigned long lickCount = 0;
    static unsigned long lastLickTime = 0;
    const int lickDelay = 25; //debouncing to filter out multi-lick detection

    // Perform initial 6-dispense sequence once
    if (!initialDispenseComplete) {
      Serial.println("Initial 6-dispense sequence");
      dispense(selection, false);
      delay(166);
      dispense(selection, false);
      delay(166);
      dispense(selection, false);
      delay(166);
      dispense(selection, false);
      delay(166);
      dispense(selection, false);
      delay(166);
      dispense(selection, false);
      delay(166);
      initialDispenseComplete = true;
      Serial.println("Pure self-paced mode active");
      UpdateDisplayPureSelfPaced(lickCount);
    }

    // Continuous lick detection and dispense using interrupt flag
    if (lickDetectedFlag) {
      unsigned long currentLickTime = millis();
      if ((currentLickTime - lastLickTime) >= lickDelay) {
        lastLickTime = currentLickTime;
        lickCount++;
        
        // Dispense on each lick
        dispense(selection, false);
        
        // Update display with lick count
        UpdateDisplayPureSelfPaced(lickCount);
      }
      lickDetectedFlag = false;  // Reset the flag
    }
    
    return; // Skip all trial structure code below
  }

  // ===== STANDARD TRIAL MODE (existing code) =====
  static unsigned long lastLickTime = 0;
  static int lastPin4State = LOW;
  const int lickDelay = 25; //debouncing to filter out multiple lick detection

  // Determine trial type: Change the threshold to control reward/punishment ratio
  // randomValue >= 80 → 20% punishment, 80% reward
  // randomValue > 100 → 0% punishment, 100% reward (always reward)
  int randomValue = random(0, 100);  // Generate random number 0-99
  bool isPunishmentTrial = (randomValue >= 0);  // SET THIS THRESHOLD: >=80 for discrimination, >100 for always reward
  int toneFrequency;
  
  if (isPunishmentTrial) {
    toneFrequency = PUNISHMENT_TONE_FREQ;
    numPulses=2;
    Serial.print("Trial ");
    Serial.print(Trial);
    Serial.println(" - PUNISHMENT TRIAL (4kHz tone + air puff)");
  } else {
    toneFrequency = REWARD_TONE_FREQ;
    numPulses=1;
    Serial.print("Trial ");
    Serial.print(Trial);
    Serial.println(" - REWARD TRIAL (12kHz tone + dispense)");
  }

  // Determine if this is a laser trial (every 2nd trial) - only if laser is enabled
  bool isLaserTrial = LASER_ENABLED && (Trial % 2 == 0);
  
  // Lick bout detection variables
  #define MAX_LICK_HISTORY 20
  unsigned long lickTimes[MAX_LICK_HISTORY];
  int lickIndex = 0;
  bool boutDetected = false;
  unsigned long boutDetectionTime = 0;
  
  // NON-BLOCKING LASER VARIABLES
  bool laserOn = false;
  unsigned long laserOnTime = 0;
  bool laserCompleted = false;
  
  // CLOSED-LOOP LASER VARIABLES
  bool closedLoopActive = false;
  unsigned long modulationStartTime = 0;
  unsigned long lastPulseTime = 0;
  bool pulseState = false;  // false = OFF, true = ON

  // MULTI-STIM LASER VARIABLES
  int currentBurst = 0;
  unsigned long burstStartTime = 0;
  unsigned long interBurstStartTime = 0;
  bool inBurst = false;
  bool inInterBurst = false;
  bool multiStimActive = false;
  bool multiStimCompleted = false;

  // Initialize lick history
  for (int i = 0; i < MAX_LICK_HISTORY; i++) {
    lickTimes[i] = 0;
  }

  if (isLaserTrial) {
    Serial.print("Trial ");
    Serial.print(Trial);
    if (TONE_STIM) {
      Serial.println(" - LASER TRIAL ARMED (TONE-TRIGGERED)");
    } else if (LASER_CLOSED_LOOP) {
      Serial.println(" - LASER TRIAL ARMED (CLOSED-LOOP 20Hz PULSED)");
    } else if (MULTI_STIM) {
      Serial.println(" - LASER TRIAL ARMED (MULTI-BURST 20Hz PULSED)");
    } else {
      Serial.println(" - LASER TRIAL ARMED (LICK-BOUT TRIGGERED)");
    }
  }

  // ===== PHASE 1: LOCKOUT =====
  lockout(); // gets truncatedExponential() passed in the call to lockout(), which seeds a new duration everytime it's violated

  // ===== PHASE 2: Trial Start Period (with truncated exponential distribution, it's actually fixed, see the parameters up top) =====
  runITI(); // see the function for more details

  // ===== PHASE 3: TONE =====
  Serial.println("Playing Tone");
  isToneActive = true;
  tone(12, toneFrequency, TONE_DURATION);
  //sendTrialSignal(numPulses);
  digitalWrite(13, HIGH);

  // Start countdown timer
  unsigned long toneStartTime = millis();

  // Turn on laser at tone onset if TONE_STIM mode and laser trial
  if (TONE_STIM && isLaserTrial) {
    digitalWrite(25, HIGH);
    laserOn = true;
    laserOnTime = millis();
    Serial.println(">>> LASER ON (TONE-TRIGGERED) <<<");
    Serial.print("Laser on time: ");
    Serial.println(laserOnTime);
  }
  
  // Extend with display updates showing tone countdown
  extend(selection, true, &toneStartTime, TONE_DURATION);
  
  // If extend finished before tone, wait for tone to complete
  while (millis() - toneStartTime < TONE_DURATION) {
    float remainingTime = (TONE_DURATION - (millis() - toneStartTime)) / 1000.0;
    UpdateDisplay(false, remainingTime, false, true, false);
  }
  
  // Stop tone signal
  digitalWrite(13, LOW);
  noTone(12);
  isToneActive = false;
    
  // ===== PHASE 4: DELAY BEFORE INITIAL DISPENSE =====
  unsigned long delayStart = millis();
  while (millis() - delayStart < TONE_TO_DISPENSE_DELAY) {
    float remainingTime = (TONE_TO_DISPENSE_DELAY - (millis() - delayStart)) / 1000.0;
    UpdateDisplay(false, remainingTime, false, false, true);
  }

  // Deliver air puff on punishment trials (after same delay as reward trials)
  if (isPunishmentTrial) {
    Serial.println("Delivering air puff");
    digitalWrite(A1, HIGH);
    delay(AIR_PUFF_DURATION);
    digitalWrite(A1, LOW);
    Serial.println("Air puff delivered");
  }

  // ===== PHASE 5: INITIAL DISPENSE (reward trials only) =====
  if (!isPunishmentTrial) {
    Serial.println("Initial dispense starting");
    dispense(selection, true);
    delay(166);
    dispense(selection, true);
    delay(166);
    dispense(selection, true);
    delay(166);
    dispense(selection, true);
    delay(166);
    dispense(selection, true);
    delay(166);
    dispense(selection, false);
    delay(166);
  } else {
    Serial.println("Skipping dispense (punishment trial)");
  }

  // Initialize MULTI_STIM if enabled and laser trial
  if (MULTI_STIM && isLaserTrial) {
    multiStimActive = true;
    currentBurst = 1;
    inBurst = true;
    burstStartTime = millis();
    lastPulseTime = burstStartTime;
    pulseState = false;  // Will turn on at first pulse cycle
    Serial.println(">>> MULTI-STIM BURST 1 STARTED <<<");
  }

  // ===== PHASE 6: TRIAL DURATION (self-paced licking with lick-triggered dispense) =====
  startTimeout = millis();
  while ((millis() - startTimeout) < trialDuration * 1000) {
    unsigned long currentTime = millis();
    float trialRemainingTime = trialDuration - (currentTime - startTimeout) / 1000.0;
    UpdateDisplay(false, trialRemainingTime, false, false, false);

    // MULTI-STIM LASER CONTROL
    if (MULTI_STIM && isLaserTrial && multiStimActive && !multiStimCompleted) {
      if (inBurst) {
        // Check if current burst has completed
        if ((currentTime - burstStartTime) >= BURST_DURATION) {
          // Turn off laser and end burst
          if (pulseState) {
            digitalWrite(25, LOW);
            pulseState = false;
          }
          inBurst = false;
          Serial.print(">>> MULTI-STIM BURST ");
          Serial.print(currentBurst);
          Serial.println(" ENDED <<<");
          
          // Check if all bursts are complete
          if (currentBurst >= NUM_BURST) {
            multiStimCompleted = true;
            Serial.println(">>> ALL MULTI-STIM BURSTS COMPLETED <<<");
          } else {
            // Start inter-burst interval
            inInterBurst = true;
            interBurstStartTime = currentTime;
            Serial.print(">>> INTER-BURST INTERVAL STARTED (");
            Serial.print(INTER_BURST_DURATION);
            Serial.println(" ms) <<<");
          }
        } else {
          // Continue 20Hz pulsing during burst
          if (pulseState) {
            // Laser is currently ON, check if it's time to turn OFF
            if ((currentTime - lastPulseTime) >= PULSE_ON_TIME) {
              digitalWrite(25, LOW);
              pulseState = false;
              lastPulseTime = currentTime;
            }
          } else {
            // Laser is currently OFF, check if it's time to turn ON
            if ((currentTime - lastPulseTime) >= PULSE_OFF_TIME) {
              digitalWrite(25, HIGH);
              pulseState = true;
              lastPulseTime = currentTime;
            }
          }
        }
      } else if (inInterBurst) {
        // Check if inter-burst interval has completed
        if ((currentTime - interBurstStartTime) >= INTER_BURST_DURATION) {
          inInterBurst = false;
          currentBurst++;
          inBurst = true;
          burstStartTime = currentTime;
          lastPulseTime = currentTime;
          pulseState = false;  // Will turn on at first pulse cycle
          Serial.print(">>> MULTI-STIM BURST ");
          Serial.print(currentBurst);
          Serial.println(" STARTED <<<");
        }
      }
    }
    // CLOSED-LOOP LASER CONTROL
    else if (LASER_CLOSED_LOOP && isLaserTrial) {
      if (closedLoopActive) {
        // Check if modulation window has expired
        if ((currentTime - modulationStartTime) >= MODULATION_TIME) {
          // Turn off laser and deactivate closed loop
          if (pulseState) {
            digitalWrite(25, LOW);
            pulseState = false;
          }
          closedLoopActive = false;
          boutDetected = false;  // RESET bout detection to allow re-triggering
          Serial.println(">>> CLOSED-LOOP LASER MODULATION ENDED <<<");
          Serial.print("Total modulation duration: ");
          Serial.print(currentTime - modulationStartTime);
          Serial.println(" ms");
        } else {
          // Continue 20Hz pulsing
          if (pulseState) {
            // Laser is currently ON, check if it's time to turn OFF
            if ((currentTime - lastPulseTime) >= PULSE_ON_TIME) {
              digitalWrite(25, LOW);
              pulseState = false;
              lastPulseTime = currentTime;
            }
          } else {
            // Laser is currently OFF, check if it's time to turn ON
            if ((currentTime - lastPulseTime) >= PULSE_OFF_TIME) {
              digitalWrite(25, HIGH);
              pulseState = true;
              lastPulseTime = currentTime;
            }
          }
        }
      }
    }
    // LICK-BOUT TRIGGERED LASER CONTROL (only if NOT TONE_STIM, NOT LASER_CLOSED_LOOP, and NOT MULTI_STIM)
    else if (!TONE_STIM && !LASER_CLOSED_LOOP && !MULTI_STIM) {
      if (isLaserTrial && boutDetected && !laserOn && !laserCompleted) {
        if ((currentTime - boutDetectionTime) >= LASER_DELAY_AFTER_BOUT) {
          Serial.println(">>> LASER ON (LICK-BOUT TRIGGERED) <<<");
          Serial.print("Laser on time: ");
          Serial.println(currentTime);
          digitalWrite(25, HIGH);
          laserOn = true;
          laserOnTime = currentTime;
        }
      }
      
      if (laserOn) {
        unsigned long laserElapsed = currentTime - laserOnTime;
        if (laserElapsed >= LASER_PULSE_DURATION) {
          Serial.println(">>> LASER OFF <<<");
          Serial.print("Laser was on for: ");
          Serial.print(laserElapsed);
          Serial.println(" ms");
          digitalWrite(25, LOW);
          laserOn = false;
          laserCompleted = true;
        }
      }
    }

    // Lick detection using pin 4
    int currentPin4State = digitalRead(4);
    if (currentPin4State == HIGH && lastPin4State == LOW) {
      unsigned long currentLickTime = millis();
      if ((currentLickTime - lastLickTime) >= lickDelay) {
        lastLickTime = currentLickTime;
        
        lickTimes[lickIndex] = currentLickTime;
        lickIndex = (lickIndex + 1) % MAX_LICK_HISTORY;
        
        Serial.print("Lick detected at ");
        Serial.println(currentLickTime);
        
        // Only dispense if SELF_PACED mode is enabled AND not a punishment trial
        if (SELF_PACED && !isPunishmentTrial) {
          dispense(selection, false);
        }

        // CLOSED-LOOP: Reset modulation timer on any lick during active modulation
        if (LASER_CLOSED_LOOP && isLaserTrial && closedLoopActive) {
          modulationStartTime = currentLickTime;
          Serial.println(">>> MODULATION TIMER RESET (lick during modulation) <<<");
        }

        // Check for lick bout to trigger laser (can re-trigger if boutDetected was reset)
        // Only relevant for LASER_CLOSED_LOOP mode (not MULTI_STIM or TONE_STIM)
        if (LASER_CLOSED_LOOP && isLaserTrial && !boutDetected) {
          int licksInWindow = 0;
          for (int i = 0; i < MAX_LICK_HISTORY; i++) {
            if (lickTimes[i] > 0 && (currentLickTime - lickTimes[i]) <= LICK_BOUT_WINDOW) {
              licksInWindow++;
            }
          }
          
          if (licksInWindow >= LICK_BOUT_THRESHOLD) {
            boutDetected = true;
            boutDetectionTime = currentLickTime;
            Serial.print(">>> LICK BOUT DETECTED: ");
            Serial.print(licksInWindow);
            Serial.println(" licks within window <<<");
            
            closedLoopActive = true;
            modulationStartTime = currentLickTime + LASER_DELAY_AFTER_BOUT;
            lastPulseTime = modulationStartTime;
            pulseState = false;  // Will turn on at first pulse cycle
            Serial.println(">>> CLOSED-LOOP LASER MODULATION STARTED <<<");
            Serial.print("Laser will pulse at 20Hz for ");
            Serial.print(MODULATION_TIME);
            Serial.println(" ms (resets on lick)");
          }
        }
        // Check for lick bout for standard lick-bout triggered mode
        else if (!TONE_STIM && !LASER_CLOSED_LOOP && !MULTI_STIM && isLaserTrial && !boutDetected) {
          int licksInWindow = 0;
          for (int i = 0; i < MAX_LICK_HISTORY; i++) {
            if (lickTimes[i] > 0 && (currentLickTime - lickTimes[i]) <= LICK_BOUT_WINDOW) {
              licksInWindow++;
            }
          }
          
          if (licksInWindow >= LICK_BOUT_THRESHOLD) {
            boutDetected = true;
            boutDetectionTime = currentLickTime;
            Serial.print(">>> LICK BOUT DETECTED: ");
            Serial.print(licksInWindow);
            Serial.println(" licks within window <<<");
            Serial.print("Laser will trigger in ");
            Serial.print(LASER_DELAY_AFTER_BOUT);
            Serial.println(" ms...");
          }
        }
      }
    }
    lastPin4State = currentPin4State;
  }

  // Turn off laser at end of trial (regardless of mode)
  if (laserOn || pulseState) {
    digitalWrite(25, LOW);
    if (laserOn) {
      unsigned long totalLaserTime = millis() - laserOnTime;
      Serial.println(">>> LASER OFF (trial ended) <<<");
      Serial.print("Total laser duration: ");
      Serial.print(totalLaserTime);
      Serial.println(" ms");
    }
    if (pulseState && LASER_CLOSED_LOOP) {
      Serial.println(">>> CLOSED-LOOP LASER FORCED OFF (trial ended) <<<");
    }
    if (pulseState && MULTI_STIM) {
      Serial.print(">>> MULTI-STIM LASER FORCED OFF (trial ended at burst ");
      Serial.print(currentBurst);
      Serial.println(") <<<");
    }
  }

  // Retract spout at end of trial
  //retract();

  // Increment trial count
  Trial++;
  
  // Update spout position for next trial if position shuffle is enabled
  if (POSITION_SHUFFLE_ENABLED) {
    selection = positionShuffle();
    spoutPosition(selection);
    Serial.print("Next trial position: ");
    Serial.println(selection);
  }
  
  // Extend spout for next trial
  //extend(selection);
}

// New function for pure self-paced mode display
void UpdateDisplayPureSelfPaced(unsigned long lickCount) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Self-Paced Mode");
  display.println();
  display.print("Licks: ");
  display.println(lickCount);
  display.display();
}