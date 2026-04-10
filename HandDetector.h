#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "AppConfig.h"
using namespace cv;
using namespace std;

class HandDetector {
public:
    Mat frame;
    Mat mask;
    CascadeClassifier faceCascade;
    
    // Motion isolation
    Ptr<BackgroundSubtractorMOG2> bgSubtractor;
    Mat fgMask;

    DetectorConfig cfg;
    int min_h = 0, max_h = 25;
    int min_s = 48, max_s = 180;
    int min_v = 60, max_v = 255;
    Rect prev_bbox;
    bool has_prev_bbox = false;

    HandDetector();
    explicit HandDetector(const DetectorConfig& config);
    void applyConfig(const DetectorConfig& config);
    void createTrackbars();
    void calibrate(Mat frame);
    vector<Point> detect(Mat input);
};
