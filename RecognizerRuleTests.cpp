#include <iostream>
#include <vector>
#include "GestureRecognizer.h"

using namespace std;

struct TestCase {
    string name;
    GestureFeatures f;
    string expected;
};

int main() {
    GestureRecognizer r;
    RecognizerConfig cfg;
    cfg.min_contour_area = 3000;
    cfg.min_rule_confidence = 0.60;
    r.applyConfig(cfg);

    vector<TestCase> tests = {
        {"A_fist", {true, 9000, 0.82, 0.96, 0.42, 1, 0, 0.0}, "A"},
        {"I_IndexUp", {true, 9000, 0.40, 0.96, 0.42, 1, 0, 0.0}, "I"},
        {"C_curve", {true, 10000, 1.52, 0.83, 0.62, 1, 0, 0.0}, "C"},
        {"N_sign", {true, 9500, 0.72, 0.78, 0.55, 2, 1, 34.0}, "N"},
        {"H_sign", {true, 9500, 1.40, 0.78, 0.55, 2, 1, 34.0}, "H"},
        {"T_sign", {true, 12000, 0.88, 0.76, 0.58, 3, 3, 29.0}, "T"},
        {"E_openPalm", {true, 14000, 0.78, 0.90, 0.75, 4, 3, 25.0}, "E"},
        {"Space_palm_flat", {true, 14000, 1.60, 0.90, 0.75, 5, 3, 25.0}, "Space"}
    };

    int pass = 0;
    for (const auto& t : tests) {
        string got = r.recognizeFromFeaturesForTest(t.f);
        bool ok = (got == t.expected);
        cout << (ok ? "[PASS] " : "[FAIL] ") << t.name
             << " expected=" << t.expected << " got=" << got
             << " conf=" << r.getLastConfidence() << endl;
        pass += ok ? 1 : 0;
    }

    cout << "Passed " << pass << "/" << tests.size() << " tests." << endl;
    return pass == static_cast<int>(tests.size()) ? 0 : 1;
}
