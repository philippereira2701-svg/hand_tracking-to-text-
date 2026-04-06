#include "HandDetector.h"
#include <iostream>

using namespace cv;
using namespace std;

HandDetector::HandDetector() {
    string cascadePath = "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml";
    if (!faceCascade.load(cascadePath)) {
        cerr << "Error: Could not load face cascade" << endl;
    }
    bgSubtractor = createBackgroundSubtractorMOG2(500, 16, true);
}

void HandDetector::createTrackbars() {
    namedWindow("Tuning", WINDOW_AUTOSIZE);
    createTrackbar("Min H", "Tuning", &min_h, 179);
    createTrackbar("Max H", "Tuning", &max_h, 179);
    createTrackbar("Min S", "Tuning", &min_s, 255);
    createTrackbar("Max S", "Tuning", &max_s, 255);
    createTrackbar("Min V", "Tuning", &min_v, 255);
    createTrackbar("Max V", "Tuning", &max_v, 255);
}

void HandDetector::calibrate(Mat input) {
    Mat hsv;
    cvtColor(input, hsv, COLOR_BGR2HSV);
    int box_size = 80, offset = 40;
    Rect roi(input.cols - box_size - offset, input.rows - box_size - offset, box_size, box_size);
    Mat sample = hsv(roi);
    Scalar mean_val, stddev;
    meanStdDev(sample, mean_val, stddev);

    // WIDER TOLERANCE for more reliable initial detection
    min_h = max(0,   (int)mean_val[0] - 25);
    max_h = min(179, (int)mean_val[0] + 25);
    min_s = max(25,  (int)mean_val[1] - 65); // More lenient on saturation
    max_s = min(255, (int)mean_val[1] + 120);
    min_v = max(30,  (int)mean_val[2] - 90); // More lenient on lighting
    max_v = min(255, (int)mean_val[2] + 140);
}

vector<Point> HandDetector::detect(Mat input) {
    frame = input;
    Mat hsv, blurred, gray;
    
    // 1. Core Processing
    GaussianBlur(frame, blurred, Size(7, 7), 0);
    cvtColor(blurred, hsv, COLOR_BGR2HSV);
    cvtColor(frame, gray, COLOR_BGR2GRAY);

    // 2. Detection Layers
    Mat skinMask;
    inRange(hsv, Scalar(min_h, min_s, min_v), Scalar(max_h, max_s, max_v), skinMask);

    bgSubtractor->apply(blurred, fgMask);
    threshold(fgMask, fgMask, 200, 255, THRESH_BINARY);

    vector<Rect> faces;
    faceCascade.detectMultiScale(gray, faces, 1.1, 3, 0, Size(80, 80));
    Mat faceMask = Mat::ones(frame.size(), CV_8UC1) * 255;
    for (const auto& face : faces) {
        Rect expanded = face;
        expanded.height *= 1.3; // Less aggressive neck mask
        expanded.y -= face.height * 0.05;
        expanded &= Rect(0, 0, frame.cols, frame.rows);
        rectangle(faceMask, expanded, Scalar(0), FILLED);
    }

    // 3. COMBINE: Keep skin that is NOT a face
    mask = skinMask & faceMask;
    
    // 4. CLEANUP
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
    morphologyEx(mask, mask, MORPH_OPEN, element);
    dilate(mask, mask, element, Point(-1,-1), 2);

    // 5. FIND HAND (Best Score Heuristic)
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Point> best_hand;
    double max_score = -1;

    for (const auto& cnt : contours) {
        double area = contourArea(cnt);
        if (area < 1500) continue; // MUCH LOWER threshold (was 5000)

        // Heuristics
        Rect bbox = boundingRect(cnt);
        double aspect_ratio = (double)bbox.width / bbox.height;
        
        vector<Point> hull;
        convexHull(cnt, hull);
        double hullArea = contourArea(hull);
        double solidity = (hullArea > 0) ? area / hullArea : 0;

        // Check for motion overlap (Bonus, not requirement)
        Mat roi_fg = fgMask(bbox);
        double motionFactor = (double)countNonZero(roi_fg) / area;

        // SCORING ENGINE:
        double score = area;
        if (solidity > 0.93) score *= 0.2; // Penalize face-like blobs
        if (aspect_ratio > 3.5 || aspect_ratio < 0.25) score *= 0.1; // Penalize lines/noise
        
        // Motion Bonus: If the blob is moving, it's MUCH more likely to be the hand
        if (motionFactor > 0.1) score *= 2.0; 

        if (score > max_score && score > 800) {
            max_score = score;
            best_hand = cnt;
        }
    }

    return best_hand;
}
