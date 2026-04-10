#include "GestureRecognizer.h"

#include <algorithm>

using namespace cv;
using namespace std;

GestureRecognizer::GestureRecognizer() : cfg(RecognizerConfig{}) {}

GestureRecognizer::GestureRecognizer(const RecognizerConfig& config) : cfg(config) {}

void GestureRecognizer::applyConfig(const RecognizerConfig& config) {
    cfg = config;
}

double GestureRecognizer::getLastConfidence() const {
    return lastConfidence;
}

GestureFeatures GestureRecognizer::getLastFeatures() const {
    return lastFeatures;
}

string GestureRecognizer::classifyFeatures(const GestureFeatures& f) {
    if (!f.valid || f.area < cfg.min_contour_area) {
        return "None";
    }

    vector<RuleScore> rules;

    const int fingers = f.finger_count;
    const double ar = f.aspect_ratio;
    const double solidity = f.solidity;
    const double extent = f.extent;
    const double avgDepth = f.avg_defect_depth;
    const int defects = f.defect_count;

    if (fingers <= 1) {
        if (ar > 1.2) {
            rules.push_back({"C", 0.95}); // Curve
        } else if (ar < 0.65) {
            rules.push_back({"I", 0.95}); // Index up
        } else if (solidity < 0.78) {
            rules.push_back({"O", 0.95}); // O-shape (Circle)
        } else {
            rules.push_back({"A", 0.95}); // Fist
        }
    }
    else if (fingers == 2) {
        if (ar < 0.55) {
            rules.push_back({"S", 0.95}); // U-sign -> S
        } else if (ar > 1.25) {
            rules.push_back({"H", 0.95}); // Two fingers sideways -> H
        } else if (solidity < 0.75) {
            rules.push_back({"L", 0.95}); // L-sign -> L
        } else {
            rules.push_back({"N", 0.95}); // V-sign -> N
        }
    }
    else if (fingers == 3) {
        rules.push_back({"T", 0.95}); // W-shape -> T
    }
    else if (fingers >= 4) {
        if (ar > 1.2) {
            rules.push_back({"Space", 0.95}); // Horizontal open palm -> Space
        } else {
            rules.push_back({"E", 0.95}); // Open palm -> E
        }
    }

    // Keep space as explicit keyboard control for conservative mode.

    if (rules.empty()) {
        return "Unknown";
    }

    auto best = max_element(rules.begin(), rules.end(), [](const RuleScore& a, const RuleScore& b) {
        return a.score < b.score;
    });
    lastConfidence = best->score;
    double secondBest = 0.0;
    for (const auto& r : rules) {
        if (r.label != best->label) {
            secondBest = max(secondBest, r.score);
        }
    }
    const double conservativeMargin = 0.06;
    if (best->score < cfg.min_rule_confidence || (best->score - secondBest) < conservativeMargin) {
        return "None";
    }
    return best->label;
}

string GestureRecognizer::recognize(const vector<Point>& cnt) {
    lastConfidence = 0.0;
    if (cnt.empty()) return "None";

    lastFeatures = extractor.extract(cnt, cfg.min_defect_depth, cfg.max_finger_angle_deg);
    return classifyFeatures(lastFeatures);
}

string GestureRecognizer::recognizeFromFeaturesForTest(const GestureFeatures& features) {
    lastFeatures = features;
    return classifyFeatures(lastFeatures);
}
