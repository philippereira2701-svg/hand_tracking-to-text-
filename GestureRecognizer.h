#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

class GestureRecognizer {
public:
    string recognize(vector<Point> cnt);
};
