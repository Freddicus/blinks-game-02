
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

void setup() {
}


void loop() {
  // if double-clicked toggle lily pad / frog
  if (buttonDoubleClicked()) {
    if (blinkState == BlinkStates::LILY_PAD || blinkState == BlinkStates::GOLDEN_LILY_PAD) {
      blinkState = BlinkStates::FROG;
    } else if (blinkState == BlinkStates::FROG) {
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
    // -- if breakage, send broke message and turn to water
  }

  if (blinkState == BlinkStates::FROG) {
    // always send out frog message
    setValueSentOnAllFaces(Messages::I_AM_A_FROG);
    // -- if clicked, increase
    // -- if lily pad message rx, minus one energy energy
    // -- if broke rx, minus one energy
  }

  displayLoop();
}

void displayLoop() {
  switch (blinkState) {
    case BlinkStates::LILY_PAD:
      setColor(GREEN);
      setColorOnFace(OFF, 0);
      break;
    case BlinkStates::FROG:
      setColor(GREEN);
      setColorOnFace(OFF, 0);
      setColorOnFace(OFF, 3);
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