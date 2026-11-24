#pragma once
// Lets another usermod borrow the I2S port: pause releases the mic's I2S channel and pins.
// Returns false if audioreactive has no initialized I2S source (nothing to pause).
bool arPauseInput();
void arResumeInput();   // no-op unless arPauseInput() returned true
