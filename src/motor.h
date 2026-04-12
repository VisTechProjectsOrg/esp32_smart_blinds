#pragma once

void motorInit();
void motorLoop();

// Position control (0-100%)
void motorMoveTo(int percent);
void motorOpen();
void motorClose();
void motorStop();
int  motorGetPercent();
bool motorIsMoving();
bool motorIsCalibrated();

// Calibration (jog + set endpoints)
void motorJog(int direction);   // +1 or -1
void motorJogStop();
void motorSetClosed();
void motorSetOpen();
