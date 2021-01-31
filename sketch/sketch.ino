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

bool lostATurn = false;

// frog variables
byte frogEnergy = 0;
bool didFrogLoseEnergyFromLilyPad = false;
bool didFrogLoseEnergyFromWater = false;

// Timers
Timer sparkleTimer;

#define FROG_DISPLAY_TIME_MS 1500
#define ENERGY_DISPLAY_TIME_MS 500

#define FROG_LIGHT_GREEN MAKECOLOR_5BIT_RGB(176 >> 3, 242 >> 3, 10 >> 3)
#define FROG_DARK_GREEN MAKECOLOR_5BIT_RGB(42 >> 3, 94 >> 3, 3 >> 3)

#define FROG_LIGHT_YELLOW MAKECOLOR_5BIT_RGB(255 >> 3, 255 >> 3, 180 >> 3)
#define FROG_DARK_YELLOW MAKECOLOR_5BIT_RGB(80 >> 3, 80 >> 3, 0)

#define LILY_PAD_GREEN MAKECOLOR_5BIT_RGB(10, 29, 1)
#define ENERGY_PINK MAKECOLOR_5BIT_RGB(245 >> 3, 144 >> 3, 190 >> 3)

void setup() {
  randomize();
}

void loop() {
  throwAwayWakingClicks();
  setValueSentOnAllFaces(Messages::NO_MSG);

  // ---------------------
  // --- SINGLE CLICK ----
  // ---------------------

  if (buttonSingleClicked()) {
    switch (blinkState) {
      case BlinkStates::LILY_PAD:
      case BlinkStates::GOLDEN_LILY_PAD:
        blinkState = BlinkStates::WATER;
        break;
      case BlinkStates::WATER:
        if (isGoldenLilyPadLockedIn) {
          blinkState = BlinkStates::GOLDEN_LILY_PAD;
        } else {
          blinkState = BlinkStates::LILY_PAD;
        }
        break;
      case BlinkStates::FROG:
        determineFrogEnergy();
        break;
    }
  }

  // ---------------------
  // --- DOUBLE CLICK ----
  // ---------------------

  if (buttonDoubleClicked()) {
    switch (blinkState) {
      case BlinkStates::WATER:
      case BlinkStates::LILY_PAD:
      case BlinkStates::GOLDEN_LILY_PAD:
        blinkState = BlinkStates::FROG;
        isGoldenLilyPadLockedIn = false;
        break;
      case BlinkStates::FROG:
        blinkState = BlinkStates::LILY_PAD;
        didFrogWin = false;
        break;
    }
  }

  // ---------------------
  // ---- MULTI CLICK ----
  // ---------------------

  if (buttonMultiClicked() && buttonClickCount() == 3) {
    switch (blinkState) {
      case BlinkStates::LILY_PAD:
        blinkState = BlinkStates::GOLDEN_LILY_PAD;
        break;
      case BlinkStates::GOLDEN_LILY_PAD:
        blinkState = BlinkStates::LILY_PAD;
        break;
    }
  }

  // ---------------------------------
  // ------------ LILY PAD LOOP ------
  // ---------------------------------

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
    // if frog leaves, set didFrogLand to false
    if (isAlone()) {
      didFrogLand = false;
    }
  }

  // ---------------------------------
  // ----- GOLDEN LILY PAD LOOP ------
  // ---------------------------------

  if (blinkState == BlinkStates::GOLDEN_LILY_PAD) {
    setValueSentOnAllFaces(Messages::I_AM_A_GOLDEN_LILY_PAD);
    if (buttonLongPressed()) {
      isGoldenLilyPadLockedIn = true;
    }
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Messages::I_AM_A_FROG) {
        didFrogWin = true;
      }
    }
  }  // golden lily pad

  // -----------------------
  // ----- WATER LOOP ------
  // -----------------------

  if (blinkState == BlinkStates::WATER) {
    setValueSentOnAllFaces(Messages::I_AM_WATER);
  }  // water

  // ----------------------
  // ----- FROG LOOP ------
  // ----------------------

  if (blinkState == BlinkStates::FROG) {
    // always send out frog message
    setValueSentOnAllFaces(Messages::I_AM_A_FROG);

    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Messages::I_AM_A_LILY_PAD) {
        if (!didFrogLoseEnergyFromLilyPad) {
          decreaseFrogEnergy(1);
          didFrogLoseEnergyFromLilyPad = true;
        }
      }  // attached to lily pad

      if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Messages::I_AM_A_GOLDEN_LILY_PAD) {
        didFrogWin = true;
      }  // attached to golden lily pad

      if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == Messages::I_AM_WATER) {
        // we were on a lily pad, we didn't lose energy from water yet, and we need to lose one more energy
        if (didFrogLoseEnergyFromLilyPad && !didFrogLoseEnergyFromWater) {
          decreaseFrogEnergy(1);
          didFrogLoseEnergyFromWater = true;
        }

        // if we landed on water, then we need to lose two energy
        if (!didFrogLoseEnergyFromLilyPad && !didFrogLoseEnergyFromWater) {
          decreaseFrogEnergy(2);
          didFrogLoseEnergyFromLilyPad = true;
          didFrogLoseEnergyFromWater = true;
        }
      }  // attached to water
    }    // for each face

    if (isAlone()) {
      didFrogLoseEnergyFromLilyPad = false;
      didFrogLoseEnergyFromWater = false;
    }
  }  // frog routine

  // ---------------------
  // -- UPDATE COLORS ----
  // ---------------------

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
        sparkle();
      } else {
        setColor(GREEN);
        setColorOnFace(FROG_LIGHT_GREEN, 0);
        setColorOnFace(FROG_DARK_GREEN, 3);
        showEnergy();
      }
      break;
    case BlinkStates::GOLDEN_LILY_PAD:
      if (isGoldenLilyPadLockedIn && !didFrogWin) {
        setColor(LILY_PAD_GREEN);
        setColorOnFace(OFF, 0);
      } else {
        setColor(YELLOW);
        sparkle();
        setColorOnFace(OFF, 0);
      }
      break;
    case BlinkStates::WATER:
      setColor(BLUE);
      break;
  }
}

void sparkle() {
  byte randomFace = random(5);
  byte randomBrightness = random(50) + 205;
  if (sparkleTimer.isExpired()) {
    setColorOnFace(dim(WHITE, randomBrightness), randomFace);
    sparkleTimer.set(125);
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
  lostATurn = false;
  do {
    frogEnergy = (1 + random(99)) % 4;
  } while (frogEnergy == 0);
}

void showEnergy() {
  if ((millis() % (FROG_DISPLAY_TIME_MS + ENERGY_DISPLAY_TIME_MS) > FROG_DISPLAY_TIME_MS)) {
    FOREACH_FACE(f) {
      if (frogEnergy == 0 && lostATurn) {
        setColorOnFace(RED, f);
      } else if (frogEnergy == 0 && !lostATurn) {
        setColorOnFace(MAKECOLOR_5BIT_RGB(1, 1, 1), f);
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
  if (numEnergyLost > frogEnergy) {
    lostATurn = true;
  }

  if (frogEnergy <= numEnergyLost) {
    frogEnergy = 0;
  } else {
    frogEnergy -= numEnergyLost;
  }
}

void throwAwayWakingClicks() {
  if (hasWoken()) {
    buttonSingleClicked();
    buttonDoubleClicked();
    buttonMultiClicked();
    buttonPressed();
  }
}