#include "Serial.h"

ServicePortSerial Serial;

enum BlinkStates {
  LILY_PAD,
  GOLDEN_LILY_PAD,
  FROG,
  WATER
};

enum Messages {
  NO_MSG,
  I_AM_A_LILY_PAD,
  I_AM_A_FROG,
  I_AM_A_GOLDEN_LILY_PAD,
  I_AM_WATER
};

byte blinkState = BlinkStates::LILY_PAD;
bool isGoldenLilyPadLockedIn = false;

bool didFrogLand = false;
bool didFrogWin = false;

// frog variables
byte frogEnergy = 0;
bool didFrogLoseEnergyFromLilyPad = false;
bool didFrogLoseEnergyFromWater = false;

#define FROG_LIGHT_GREEN MAKECOLOR_5BIT_RGB(176 >> 3, 242 >> 3, 10 >> 3)
#define FROG_DARK_GREEN MAKECOLOR_5BIT_RGB(42 >> 3, 94 >> 3, 3 >> 3)

#define FROG_LIGHT_YELLOW MAKECOLOR_5BIT_RGB(255 >> 3, 255 >> 3, 180 >> 3)
#define FROG_DARK_YELLOW MAKECOLOR_5BIT_RGB(80 >> 3, 80 >> 3, 0)

#define LILY_PAD_GREEN MAKECOLOR_5BIT_RGB(90 >> 3, 140 >> 3, 3 >> 3)
#define ENERGY_PINK MAKECOLOR_5BIT_RGB(245 >> 3, 144 >> 3, 190 >> 3)

void setup() {
  Serial.begin();
  randomize();
}

void loop() {
  // if double-clicked toggle lily pad / frog
  if (buttonDoubleClicked()) {
    if (blinkState == BlinkStates::LILY_PAD || blinkState == BlinkStates::GOLDEN_LILY_PAD) {
      blinkState = BlinkStates::FROG;
      isGoldenLilyPadLockedIn = false;
    } else if (blinkState == BlinkStates::FROG) {
      blinkState = BlinkStates::LILY_PAD;
      didFrogWin = false;
    } else if (blinkState == BlinkStates::WATER) {
      blinkState = BlinkStates::LILY_PAD;
    } else {
      // future token states?
    }
  }

  // if triple-clicked, toggle golden
  if (buttonMultiClicked() && buttonClickCount() == 3) {
    if (blinkState == BlinkStates::LILY_PAD) {
      blinkState = BlinkStates::GOLDEN_LILY_PAD;
    } else if (blinkState == BlinkStates::GOLDEN_LILY_PAD) {
      blinkState = BlinkStates::LILY_PAD;
    } else {
      // future states?
    }
  }

  // -------

  // if lily pad
  if (blinkState == BlinkStates::LILY_PAD) {
    // always send out lily pad message
    setValueSentOnAllFaces(Messages::I_AM_A_LILY_PAD);
    // if frog attached, randomize breakage
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Messages::I_AM_A_FROG) {
        if (didLilyPadBreak() && !didFrogLand) {
          blinkState = BlinkStates::WATER;
        } else if (didFrogLand) {
          // check to see if lily pad breaks from staying on it too long
        }
        didFrogLand = true;
      }
    }
    // -- if breakage, send broke message and turn to water
    // if frog leaves, set didFrogLand to false
    if (isAlone()) {
      didFrogLand = false;
    }
  }

  if (blinkState == BlinkStates::GOLDEN_LILY_PAD) {
    setValueSentOnAllFaces(Messages::I_AM_A_GOLDEN_LILY_PAD);
    if (buttonLongPressed()) {
      isGoldenLilyPadLockedIn = true;
    }
  }

  if (blinkState == BlinkStates::FROG) {
    // always send out frog message
    setValueSentOnAllFaces(Messages::I_AM_A_FROG);
    // -- if clicked
    if (buttonSingleClicked()) {
      determineFrogEnergy();
    }
    // -- if lily pad message rx, minus one energy
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Messages::I_AM_A_LILY_PAD) {
        if (!didFrogLoseEnergyFromLilyPad) {
          decreaseFrogEnergy(1);
          didFrogLoseEnergyFromLilyPad = true;
        }
      }

      if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Messages::I_AM_A_GOLDEN_LILY_PAD) {
        didFrogWin = true;
      }

      if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Messages::I_AM_WATER) {
        // we were on a lily pad, we didn't lose energy from water yet, and we need to lose one more energy
        if (didFrogLoseEnergyFromLilyPad && !didFrogLoseEnergyFromWater) {
          decreaseFrogEnergy(1);
          didFrogLoseEnergyFromLilyPad = true;
        }

        // if we landed on water, then we need to lose two energy
        if (!didFrogLoseEnergyFromLilyPad && !didFrogLoseEnergyFromWater) {
          decreaseFrogEnergy(2);
          didFrogLoseEnergyFromLilyPad = true;
          didFrogLoseEnergyFromWater = true;
        }
      }
    }  // for each face

    if (isAlone()) {
      didFrogLoseEnergyFromLilyPad = false;
      didFrogLoseEnergyFromWater = false;
    }

    if (blinkState == BlinkStates::WATER) {
      setValueSentOnAllFaces(Messages::I_AM_WATER);
    }
  }  // frog routine
  displayLoop();
}  // main loop

void displayLoop() {
  switch (blinkState) {
    case BlinkStates::LILY_PAD:
      setColor(LILY_PAD_GREEN);
      setColorOnFace(OFF, 0);
      break;
    case BlinkStates::FROG:
      if (didFrogWin) {
        setColor(YELLOW);
        setColorOnFace(FROG_LIGHT_YELLOW, 0);
        setColorOnFace(FROG_DARK_YELLOW, 3);
      } else {
        setColor(GREEN);
        setColorOnFace(FROG_LIGHT_GREEN, 0);
        setColorOnFace(FROG_DARK_GREEN, 3);
        showEnergy();
      }
      break;
    case BlinkStates::GOLDEN_LILY_PAD:
      // TODO: replace isAlone with frog detect
      if (isGoldenLilyPadLockedIn && isAlone()) {
        setColor(LILY_PAD_GREEN);
        setColorOnFace(OFF, 0);
      } else {
        setColor(YELLOW);
        setColorOnFace(OFF, 0);
      }
      break;
    case BlinkStates::WATER:
      setColor(BLUE);
      break;
  }
}

bool didLilyPadBreak() {
  short randomNumber = random(100);
  if (randomNumber > 75) {
    return true;
  } else {
    return false;
  }
}

void determineFrogEnergy() {
  do {
    frogEnergy = (1 + random(99)) % 4;
  } while (frogEnergy == 0);
}

void showEnergy() {
  if ((millis() % 1500 > 1000)) {
    FOREACH_FACE(f) {
      if (frogEnergy == 0) {
        setColorOnFace(RED, f);
      } else {
        if (f <= frogEnergy - 1) {
          setColorOnFace(ENERGY_PINK, f);
        } else {
          setColorOnFace(OFF, f);
        }
      }  // frogEnergy == 0
    }    // for-each face
  }      // millis
}

void decreaseFrogEnergy(byte numEnergyLost) {
  if (frogEnergy <= numEnergyLost) {
    frogEnergy = 0;
  } else {
    frogEnergy -= numEnergyLost;
  }
}