#include "motor.h"
#include "storage.h"
#include "config.h"
#include <AccelStepper.h>

static AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

static long calClosed = 0;
static long calOpen = 0;
static bool calibrated = false;
static bool wasMoving = false;
static bool lastDirOpening = true;

static void motorEnable() {
    digitalWrite(EN_PIN, LOW);   // LOW = driver enabled
}

static void motorDisable() {
    digitalWrite(EN_PIN, HIGH);  // HIGH = driver disabled (silent, no holding torque)
}

void motorInit() {
    pinMode(EN_PIN, OUTPUT);
    motorDisable();  // start disabled (silent)

    pinMode(MS1_PIN, OUTPUT);
    pinMode(MS2_PIN, OUTPUT);
    digitalWrite(MS1_PIN, HIGH);
    digitalWrite(MS2_PIN, HIGH);  // 1/16 microstepping

    stepper.setMaxSpeed(DEFAULT_MAX_SPEED);
    stepper.setAcceleration(DEFAULT_ACCEL);

    calibrated = hasCalibration();
    if (calibrated) {
        loadCalibration(calClosed, calOpen);
        long savedPos = loadCurrentPosition();
        stepper.setCurrentPosition(savedPos);
        Serial.printf("Motor: calibrated, closed=%ld open=%ld pos=%ld\n",
                       calClosed, calOpen, savedPos);
    } else {
        stepper.setCurrentPosition(0);
        Serial.println("Motor: not calibrated");
    }
}

void motorLoop() {
    stepper.run();

    bool moving = stepper.distanceToGo() != 0;
    if (wasMoving && !moving) {
        motorDisable();  // kill holding current - no more hissing
        saveCurrentPosition(stepper.currentPosition());
        Serial.printf("Motor stopped at %ld (%d%%)\n",
                       stepper.currentPosition(), motorGetPercent());
    }
    wasMoving = moving;
}

void motorMoveTo(int percent) {
    if (!calibrated) return;
    percent = constrain(percent, 0, 100);
    long target = map((long)percent, 0, 100, calClosed, calOpen);
    motorEnable();
    stepper.moveTo(target);
    Serial.printf("Motor: moving to %d%% (step %ld)\n", percent, target);
}

void motorOpen()  { lastDirOpening = true;  motorMoveTo(100); }
void motorClose() { lastDirOpening = false; motorMoveTo(0); }

void motorStop() {
    stepper.stop();  // decelerates to stop
}

int motorGetPercent() {
    if (!calibrated || calOpen == calClosed) return 0;
    long pos = stepper.currentPosition();
    return constrain((int)map(pos, calClosed, calOpen, 0, 100), 0, 100);
}

bool motorIsMoving()     { return stepper.distanceToGo() != 0; }
bool motorIsCalibrated() { return calibrated; }
bool motorWasOpening()   { return lastDirOpening; }

// --- Calibration ---

void motorJog(int direction) {
    motorEnable();
    long target = stepper.currentPosition() + (direction > 0 ? 100000 : -100000);
    stepper.moveTo(target);
}

void motorJogStop() {
    stepper.stop();
}

void motorSetClosed() {
    calClosed = stepper.currentPosition();
    Serial.printf("Calibration: closed set to %ld\n", calClosed);
}

void motorSetOpen() {
    calOpen = stepper.currentPosition();
    calibrated = true;
    saveCalibration(calClosed, calOpen);
    saveCurrentPosition(stepper.currentPosition());
    Serial.printf("Calibration: open set to %ld (range: %ld steps)\n",
                   calOpen, abs(calOpen - calClosed));
}
