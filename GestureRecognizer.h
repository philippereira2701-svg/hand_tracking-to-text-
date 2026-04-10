#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "AppConfig.h"
#include "FeatureExtractor.h"

using namespace cv;
using namespace std;

class GestureRecognizer {
private:
    struct RuleScore {
        string label;
        double score;
    };

    RecognizerConfig cfg;
    FeatureExtractor extractor;
    double lastConfidence = 0.0;
    GestureFeatures lastFeatures;
    string classifyFeatures(const GestureFeatures& f);

public:
    GestureRecognizer();
    explicit GestureRecognizer(const RecognizerConfig& config);
    void applyConfig(const RecognizerConfig& config);
    string recognize(const vector<Point>& cnt);
    string recognizeFromFeaturesForTest(const GestureFeatures& features);
    double getLastConfidence() const;
    GestureFeatures getLastFeatures() const;
};
