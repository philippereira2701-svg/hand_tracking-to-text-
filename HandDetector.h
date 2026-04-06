#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
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

    int min_h = 0,  max_h = 25;
    int min_s = 48, max_s = 180;
    int min_v = 60, max_v = 255;

    HandDetector();
    void createTrackbars();
    void calibrate(Mat frame);
    vector<Point> detect(Mat input);
};
