#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include "HandDetector.h"
#include "GestureRecognizer.h"
#include "WordBuilder.h"

using namespace cv;
using namespace std;

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error: Could not open webcam" << endl;
        return -1;
    }

    HandDetector detector;
    GestureRecognizer recognizer;
    WordBuilder wordBuilder;

    namedWindow("Sign Language Recognition", WINDOW_NORMAL);
    resizeWindow("Sign Language Recognition", 1280, 720);
    
    Mat frame;
    bool calibrated = false;
    int calibration_countdown = 150; 

    cout << "Press 'c' to re-calibrate, 's' for space, 'x' for backspace, 'q' to quit." << endl;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        if (!calibrated) {
            // FIX: Match HandDetector::calibrate — bottom-right corner, away from face
            int box_size = 80;
            int offset   = 40;
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
            wordBuilder.process(letter);

            if (!hand.empty()) {
                vector<vector<Point>> contours = {hand};
                drawContours(frame, contours, -1, Scalar(0, 255, 0), 2);
                Rect bbox = boundingRect(hand);
                rectangle(frame, bbox, Scalar(255, 0, 0), 1);
            }

            // UI Elements
            putText(frame, "Stabilizing: " + letter, Point(30, 60), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0, 0, 255), 2);
            
            float progress = wordBuilder.getProgress();
            if (progress > 0) {
                rectangle(frame, Point(30, 80), Point(30 + (int)(250 * progress), 95), Scalar(0, 255, 0), FILLED);
                rectangle(frame, Point(30, 80), Point(280, 95), Scalar(255, 255, 255), 1);
            }

            string sentence = "Sentence: " + wordBuilder.getSentence();
            putText(frame, sentence, Point(30, 450), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0, 255, 0), 2);
            
            imshow("Skin Mask", detector.mask);
            imshow("Motion Mask", detector.fgMask);
        }

        imshow("Sign Language Recognition", frame);

        char key = (char)waitKey(1);
        if (key == 'q') break;
        if (key == 'c') { calibrated = false; calibration_countdown = 150; }
        if (key == 's') wordBuilder.process("Space");
        if (key == 'x') wordBuilder.process("Backspace");
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
