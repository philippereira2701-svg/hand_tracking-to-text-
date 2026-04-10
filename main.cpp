#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include "AppConfig.h"
#include "HandDetector.h"
#include "GestureRecognizer.h"
#include "WordBuilder.h"
#include "TTSEngine.h"

using namespace cv;
using namespace std;

int main() {
    AppConfig appCfg;
    bool configLoaded = loadConfig("config.json", appCfg);

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error: Could not open webcam" << endl;
        return -1;
    }

    HandDetector detector(appCfg.detector);
    GestureRecognizer recognizer(appCfg.recognizer);
    WordBuilder wordBuilder;
    wordBuilder.configure(appCfg.wordBuilder.stable_threshold_frames, appCfg.wordBuilder.re_trigger_delay_frames);
    TTSEngine tts;
    tts.configure(appCfg.tts.enabled, appCfg.tts.primary, appCfg.tts.fallback);
    tts.start();

    namedWindow("Sign Language Recognition", WINDOW_NORMAL);
    resizeWindow("Sign Language Recognition", 1280, 720);
    
    Mat frame;
    bool calibrated = false;
    int calibration_countdown = 150; 

    cout << "Press 'c' recalibrate, 's' space, 'x' backspace, 'r' reset, 'p' speak, 'q' quit." << endl;
    if (!configLoaded) {
        cout << "Warning: config.json was not loaded, using defaults." << endl;
    }

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        if (!calibrated) {
            int box_size = appCfg.detector.calibration_box_size;
            int offset = appCfg.detector.calibration_offset;
            Rect roi(
                frame.cols - box_size - offset,
                frame.rows - box_size - offset,
                box_size, box_size
            );
            rectangle(frame, roi, Scalar(0, 255, 255), 3);
            
            putText(frame, "PLACE HAND IN BOTTOM-RIGHT BOX", Point(50, 50), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 255), 2);
            
            if (calibration_countdown > 0) {
                detector.calibrate(frame);
                calibration_countdown--;
                string countdown_text = "Calibrating: " + to_string(calibration_countdown / 30 + 1) + "s";
                putText(frame, countdown_text, Point(50, 400), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0, 255, 0), 2);
            } else {
                calibrated = true;
                destroyWindow("Calibration Sample"); // Clean up
            }
        } else {
            vector<Point> hand = detector.detect(frame);
            string letter = recognizer.recognize(hand);
            string committed = wordBuilder.process(letter);
            if (committed == "__SPEAK__") {
                tts.speakAsync(wordBuilder.getSentence());
            }

            if (!hand.empty()) {
                vector<vector<Point>> contours = {hand};
                drawContours(frame, contours, -1, Scalar(0, 255, 0), 2);
                Rect bbox = boundingRect(hand);
                rectangle(frame, bbox, Scalar(255, 0, 0), 1);
            }

            // UI Elements
            putText(frame, "Stabilizing: " + letter, Point(30, 60), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0, 0, 255), 2);
            string conf = "Confidence: " + to_string(static_cast<int>(recognizer.getLastConfidence() * 100.0)) + "%";
            putText(frame, conf, Point(30, 95), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 0), 2);
            
            float progress = wordBuilder.getProgress();
            if (progress > 0) {
                rectangle(frame, Point(30, 110), Point(30 + (int)(250 * progress), 125), Scalar(0, 255, 0), FILLED);
                rectangle(frame, Point(30, 110), Point(280, 125), Scalar(255, 255, 255), 1);
            }

            string sentence = "Sentence: " + wordBuilder.getSentence();
            putText(frame, sentence, Point(30, 450), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0, 255, 0), 2);

            string ttsStatus = "TTS: ";
            if (!tts.isAvailable()) {
                ttsStatus += "Unavailable";
            } else if (tts.hasError()) {
                ttsStatus += "Error";
            } else if (tts.isSpeaking()) {
                ttsStatus += "Speaking";
            } else {
                ttsStatus += "Idle";
            }
            putText(frame, ttsStatus, Point(30, 485), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(200, 255, 200), 2);
            
            imshow("Skin Mask", detector.mask);
            imshow("Motion Mask", detector.fgMask);
        }

        imshow("Sign Language Recognition", frame);

        char key = (char)waitKey(1);
        if (key == 'q') break;
        if (key == 'c') { calibrated = false; calibration_countdown = 150; }
        if (key == 's') (void)wordBuilder.process("Space");
        if (key == 'x') (void)wordBuilder.process("Backspace");
        if (key == 'r') wordBuilder.clearSentence();
        if (key == 'p' || key == 13) tts.speakAsync(wordBuilder.getSentence());
    }

    tts.stop();
    cap.release();
    destroyAllWindows();
    return 0;
}
