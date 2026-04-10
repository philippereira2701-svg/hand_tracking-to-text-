#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

struct GestureFeatures {
    bool valid = false;
    double area = 0.0;
    double aspect_ratio = 0.0;
    double solidity = 0.0;
    double extent = 0.0;
    int finger_count = 0;
    int defect_count = 0;
    double avg_defect_depth = 0.0;
    Point2f palm_center = Point2f(0, 0);
    vector<Point> hull_points;
    vector<int> hull_indices;
    vector<Vec4i> defects;
    vector<Point> fingertip_candidates;
};

class FeatureExtractor {
public:
    GestureFeatures extract(const vector<Point>& contour, int minDefectDepth, int maxFingerAngleDeg) const;
};
