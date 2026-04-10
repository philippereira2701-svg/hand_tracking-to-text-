# Hand Sign To Sentence (C++ + OpenCV)

Real-time webcam application that detects hand signs, converts them into sentence text, and speaks sentence text using Linux TTS.

## Features

- Rule-based gesture recognition using contour, convex hull, and convexity-defect features.
- Config-driven thresholds for detector/recognizer/word builder behavior.
- Sentence stabilization to prevent repeated character spam.
- Non-blocking text-to-speech with backend fallback (`spd-say` then `espeak-ng`).
- Debug overlays for recognized gesture, confidence, progress, sentence, and TTS status.

## Dependencies

- Linux
- OpenCV 4 development package
- g++
- Optional TTS backends:
  - `speech-dispatcher` (`spd-say`)
  - `espeak-ng`

## Build

```bash
cd sign_language_recognition
chmod +x compile.sh
./compile.sh
```

## Run

```bash
cd sign_language_recognition
./sign_lang
```

## Controls

- `q`: quit
- `c`: re-calibrate skin color sample
- `s`: inject space
- `x`: inject backspace
- `r`: clear sentence
- `p` or `Enter`: speak current sentence

## Config

Runtime parameters are in `config.json`.

- `hand_detector`: HSV range, morphology, contour filtering, shape filtering.
- `calibration`: sample box and tolerance settings.
- `gesture_recognizer`: min area, defect depth, angle threshold, rule confidence.
- `word_builder`: frame stability and retrigger delay.
- `tts`: backend enable and command priority.

## Validation Workflow

1. Start with neutral background and good frontal lighting.
2. Run calibration and hold each sign for ~1 second.
3. Confirm confidence and sentence stability on screen.
4. Tune `gesture_recognizer.min_rule_confidence` if false positives occur.
5. Tune `hand_detector.contour_filter.min_area` for your camera distance.

Target acceptance for first pass:
- Core gestures (A, B, C, D, V, W, Space) should be recognized >=80% in controlled lighting.



1 Finger / Closed Hand
A : Normal closed fist
I : Single index finger pointing strictly UP
O : Thumb and index forming a circle/ring (the system recognizes the "hollow" bounding box dropping your solidity score)
C : Hand curved horizontally
2 Fingers
N : V-sign (Index & Middle parted)
S : U-sign (Index & Middle closed tight together)
L : L-sign (Thumb wide horizontal, Index up)
H : Index & Middle pointing strictly sideways horizontally (detects the exceptionally wide rectangle)
3 Fingers
T : W-shape (three fingers up)
4+ Fingers
E : Standard open palm facing the camera
