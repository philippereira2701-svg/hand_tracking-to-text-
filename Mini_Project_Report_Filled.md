# Mini Project Report

## Title of the Mini Project
**Hand Sign to Sentence Conversion using C++ and OpenCV**

### Submitted by
- Student Name 1 (PRN: __________)
- Student Name 2 (PRN: __________)
- Student Name 3 (PRN: __________)

SY CSE (Panel A)  
Course: Object Oriented Programming in C++  
Course Code: CSE1003B  

Under the Internal Guidance of  
Prof./Dr. ____________________

Department of Computer Engineering and Technology  
MIT World Peace University, Kothrud, Pune 411038, Maharashtra, India  
Academic Year: 2025-2026

---

## Certificate
This is to certify that Student Name 1, Student Name 2, and Student Name 3 of SY-BTech (Computer Science and Engineering) have completed their mini project titled **"Hand Sign to Sentence Conversion using C++ and OpenCV"** and have submitted this Object Oriented Programming in C++ report for the academic year 2025-26 (Sem-IV).

Mini Project Guide: Prof./Dr. ____________________  
Department of CET, MIT World Peace University, Pune

---

## Abstract
This mini project presents a real-time hand sign recognition system that converts recognized hand gestures into sentence text and speaks the sentence through text-to-speech (TTS). The application is implemented in C++ using OpenCV and follows a rule-based recognition pipeline suitable for Object Oriented Programming concepts.

The system captures live webcam input, performs skin-color segmentation with calibration, removes face regions, applies motion and contour-based filtering, extracts geometric hand features, and maps them to gesture labels using configurable recognition rules. A stabilization module commits letters only after frame-level consistency, reducing repeated or noisy outputs. The generated sentence can be spoken asynchronously through Linux TTS tools (`spd-say` with `espeak-ng` fallback).

The project emphasizes modular OOP design through dedicated classes for configuration loading, detection, feature extraction, recognition, sentence building, and speech. It supports runtime tuning through `config.json` and includes rule-level test cases for recognizer validation. The implementation demonstrates practical computer vision with robust software structuring.

---

## List of Figures and Tables
- **Figure 1:** High-level architecture of the system  
- **Figure 2:** Class relationship diagram  
- **Table 1:** Gesture mapping rules  
- **Table 2:** Test cases and observed outputs

---

## Contents
1. Introduction  
2. Problem Statement  
3. Objectives  
4. List of Classes and Class Diagram  
5. Description of OOP Concepts Used  
6. Implementation Details and Code  
7. Test Cases and Results  
8. Individual Contributions  
9. Conclusion and Future Scope

---

## Chapter 1: Introduction
Communication barriers faced by speech-impaired individuals can be reduced by systems that automatically convert sign gestures into machine-readable and audible forms. This project implements a webcam-based sign recognition tool that identifies hand postures and constructs text sentences in real time.

Unlike deep-learning-heavy systems that need large datasets and training infrastructure, this solution uses a compact rule-based method with geometric and contour features. The approach is suitable for an OOP mini project because it provides understandable logic, modular class design, configurable behavior, and testability.

The final application offers:
- real-time gesture recognition,
- stable sentence formation,
- keyboard-assisted editing controls,
- asynchronous text-to-speech output,
- configurable thresholds for different environments.

---

## Chapter 2: Problem Statement
Design and implement a real-time C++ application that:
- captures live hand gestures through a webcam,
- recognizes predefined sign patterns reliably under normal indoor lighting,
- converts recognized signs into sentence text with noise control,
- provides speech output from the generated sentence,
- follows Object Oriented Programming principles and modular architecture.

---

## Chapter 3: Objectives
- Build a complete real-time sign interpretation pipeline in C++ and OpenCV.
- Apply robust hand detection using skin segmentation, face masking, and motion filtering.
- Extract gesture features (area, aspect ratio, solidity, convexity defects, finger count).
- Recognize signs using rule-based classification with confidence thresholding.
- Stabilize output to avoid repeated character spam.
- Provide configurable runtime behavior through JSON configuration.
- Integrate non-blocking TTS for speech generation.
- Validate recognizer logic with deterministic test cases.

---

## Chapter 4: List of Classes and Class Diagram

### Classes Used
- `AppConfig` (with `DetectorConfig`, `RecognizerConfig`, `WordBuilderConfig`, `TTSConfig`)
- `HandDetector`
- `FeatureExtractor`
- `GestureRecognizer`
- `WordBuilder`
- `TTSEngine`
- `main` (orchestration)

### Class Diagram (Textual)
```text
main.cpp
  |-- uses --> AppConfig (loadConfig)
  |-- owns --> HandDetector
  |-- owns --> GestureRecognizer
  |-- owns --> WordBuilder
  |-- owns --> TTSEngine

GestureRecognizer
  |-- has --> FeatureExtractor
  |-- uses --> RecognizerConfig

HandDetector
  |-- uses --> DetectorConfig

TTSEngine
  |-- uses --> TTSConfig values (via configure)
```

### Figure 1: High-Level Architecture
```text
Webcam Frame
   -> HandDetector (HSV + ROI + face mask + motion + contour filtering)
   -> GestureRecognizer (FeatureExtractor + rule mapping)
   -> WordBuilder (stability + re-trigger control)
   -> Sentence Buffer
   -> TTSEngine (async speak)
```

---

## Chapter 5: Description of OOP Concepts Used

### 1. Encapsulation
Each module manages its own state and operations:
- `HandDetector` encapsulates calibration and detection masks.
- `GestureRecognizer` encapsulates feature-to-label rules and confidence.
- `WordBuilder` encapsulates sentence state and stability counters.
- `TTSEngine` encapsulates worker thread, speech queue, and backend execution.

### 2. Abstraction
Complex internals are hidden behind clear interfaces:
- `detect()`, `recognize()`, `process()`, and `speakAsync()` expose simple operations.
- Caller code in `main.cpp` remains readable and high-level.

### 3. Composition
The system is built by combining objects:
- `main.cpp` composes detector, recognizer, builder, and TTS engine.
- `GestureRecognizer` composes `FeatureExtractor` for reusable feature logic.

### 4. Modularity and Reusability
Configuration loading (`loadConfig`) is isolated from processing classes.  
`FeatureExtractor` and recognizer tests (`RecognizerRuleTests.cpp`) allow independent validation.

### 5. Constructor Overloading and Method Interfaces
`HandDetector` and `GestureRecognizer` provide default and config-based constructors, enabling both quick start and tuned deployment.

---

## Chapter 6: Implementation Details and Code

### 6.1 Workflow
1. Load runtime parameters from `config.json`.  
2. Start webcam stream.  
3. Perform initial skin calibration in a fixed ROI.  
4. Detect best hand contour using layered filtering.  
5. Extract gesture features and classify sign.  
6. Stabilize sign frames before committing character.  
7. Build sentence and optionally speak asynchronously.  
8. Display overlays (gesture, confidence, progress, sentence, TTS state).

### 6.2 Key Modules

**A) Configuration (`AppConfig.cpp/.h`)**
- Reads detector, calibration, recognizer, word-builder, and TTS parameters from JSON.
- Applies fallback defaults when keys are absent.

**B) Hand Detection (`HandDetector.cpp/.h`)**
- Gaussian blur + HSV conversion.
- Skin thresholding with wrap-around hue support.
- Background subtraction for motion consistency.
- Face region suppression using Haar cascade.
- Optional ROI bounds.
- Morphological cleanup and contour scoring to keep best candidate.

**C) Feature Extraction (`FeatureExtractor.cpp/.h`)**
- Computes contour area, bounding-box aspect ratio, extent, solidity.
- Finds convex hull and convexity defects.
- Estimates fingertip candidates and finger count.

**D) Gesture Recognition (`GestureRecognizer.cpp/.h`)**
- Uses extracted features and rule conditions to classify:
  - 1-finger/closed: `A`, `I`, `O`, `C`
  - 2-finger: `N`, `S`, `L`, `H`
  - 3-finger: `T`
  - 4+ fingers: `E`, `Space`
- Applies minimum confidence and conservative margin checks.

**E) Sentence Builder (`WordBuilder.h`)**
- Commits only after stable frame threshold.
- Supports delayed re-trigger to avoid repeated commits.
- Handles `Space`, `Backspace`, `Clear`, and `Speak` actions.

**F) Text to Speech (`TTSEngine.cpp/.h`)**
- Non-blocking queue-based speaking thread.
- Primary backend `spd-say`, fallback `espeak-ng`.
- Tracks speaking, availability, and error status.

### 6.3 Build and Run
- Build command: `./compile.sh`
- Run command: `./sign_lang`
- Rule-test command: `./test_rules.sh`

---

## Chapter 7: Test Cases and Results

### Table 1: Gesture Rule Test Cases (from `RecognizerRuleTests.cpp`)
| Sr No | Test Name          | Expected | Result (Current Logic) |
|------:|--------------------|----------|-------------------------|
| 1 | A_fist             | A        | Pass |
| 2 | I_IndexUp          | I        | Pass |
| 3 | C_curve            | C        | Pass |
| 4 | N_sign             | N        | Pass |
| 5 | H_sign             | H        | Pass |
| 6 | T_sign             | T        | Pass |
| 7 | E_openPalm         | E        | Pass |
| 8 | Space_palm_flat    | Space    | Pass |

**Observed:** 8/8 rule tests pass in deterministic unit-style execution.

### Runtime Validation Notes
- Core gestures are recognized effectively in controlled lighting.
- Calibration and threshold tuning in `config.json` significantly improve stability.
- Stabilization mechanism reduces accidental repeated characters.

---

## Chapter 8: Individual Contributions
> Replace placeholder names with actual member details.

- **Student Name 1 (PRN: ______):** Hand detection pipeline, calibration workflow, contour filtering.
- **Student Name 2 (PRN: ______):** Feature extraction and gesture rule-classifier implementation.
- **Student Name 3 (PRN: ______):** Word-building stability logic, TTS integration, testing and documentation.

---

## Chapter 9: Conclusion and Future Scope
The project successfully demonstrates an object-oriented, real-time hand-sign-to-sentence system using C++ and OpenCV. It integrates computer vision, configurable recognition rules, stable text generation, and speech synthesis into a usable pipeline. The architecture is modular, readable, and testable, making it suitable for academic demonstration and further development.

### Future Scope
- Add dataset-driven or hybrid ML classification for higher gesture vocabulary.
- Introduce adaptive illumination handling and multi-hand scenarios.
- Support full sign language words/phrases beyond alphabet-like mappings.
- Add GUI settings panel for live threshold tuning.
- Port to cross-platform desktop/mobile targets.

---

## References
1. OpenCV Documentation: https://docs.opencv.org/  
2. C++ Reference: https://en.cppreference.com/  
3. Linux TTS tools: `speech-dispatcher`, `espeak-ng`
