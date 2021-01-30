
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
bool didFrogLand = false;
byte frogEnergy = 0;
#define FROG_LIGHT_GREEN MAKECOLOR_5BIT_RGB(176 >> 3, 242 >> 3, 10 >> 3);
#define FROG_DARK_GREEN MAKECOLOR_5BIT_RGB(42 >> 3, 94 >> 3, 3 >> 3);
#define LILY_PAD_GREEN MAKECOLOR_5BIT_RGB(90 >> 3, 140 >> 3, 3 >> 3);
#define ENERGY_PINK MAKECOLOR_5BIT_RGB(245 >> 3, 144 >> 3, 190 >> 3);

void setup() {
  randomize();
}

void loop() {
  // if double-clicked toggle lily pad / frog
  if (buttonDoubleClicked()) {
    if (blinkState == BlinkStates::LILY_PAD || blinkState == BlinkStates::GOLDEN_LILY_PAD) {
      blinkState = BlinkStates::FROG;
    } else if (blinkState == BlinkStates::FROG) {
      blinkState = BlinkStates::LILY_PAD;
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
    // -- if frog attached, randomize breakage
    FOREACH_FACE(f) {
      if (getLastValueReceivedOnFace(f) == Messages::I_AM_A_FROG) {
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
    setValueSentOnAllFaces(Messages::I_AM_A_LILY_PAD);
  }

  if (blinkState == BlinkStates::FROG) {
    // always send out frog message
    setValueSentOnAllFaces(Messages::I_AM_A_FROG);
    // -- if clicked
    if (buttonSingleClicked()) {
      determineFrogEnergy();
    }
    // -- if lily pad message rx, minus one energy energy
    // -- if broke rx, minus one energy
  }

  if (blinkState == BlinkStates::WATER) {
    setValueSentOnAllFaces(Messages::I_AM_WATER);
  }

  displayLoop();
}

void displayLoop() {
  switch (blinkState) {
    case BlinkStates::LILY_PAD:
      setColor(LILY_PAD_GREEN);
      setColorOnFace(OFF, 0);
      break;
    case BlinkStates::FROG:
      setColor(GREEN);
      setColorOnFace(FROG_LIGHT_GREEN, 0);
      setColorOnFace(FROG_DARK_GREEN, 3);
      showEnergy();
      break;
    case BlinkStates::GOLDEN_LILY_PAD:
      setColor(YELLOW);
      setColorOnFace(OFF, 0);
      break;
    case BlinkStates::WATER:
      setColor(BLUE);
      break;
  }
}

bool didLilyPadBreak() {
  short randomNumber = random(100);
  if (randomNumber % 10) {
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
  if ((millis() % 4000 > 1000)  || (buttonDown())) {
    FOREACH_FACE(f) {
      if (frogEnergy == 0) {
        setColorOnFace(RED, f);
      } else {
        if (f <= frogEnergy - 1) {
          setColorOnFace(CYAN, f);
        } else {
          setColorOnFace(OFF, f);
        }
      }  // frogEnergy == 0
    }    // for-each face
  }      // millis
}