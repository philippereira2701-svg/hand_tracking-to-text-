#include "GestureRecognizer.h"
#include <cmath>
#include <algorithm>

using namespace cv;
using namespace std;

string GestureRecognizer::recognize(vector<Point> cnt) {
    if (cnt.empty()) return "None";
    
    double area = contourArea(cnt);
    if (area < 3500) return "None";

    // Basic geometric features
    Rect bbox = boundingRect(cnt);
    double aspect_ratio = (double)bbox.width / bbox.height;
    
    vector<Point> hull;
    convexHull(cnt, hull);
    double hull_area = contourArea(hull);
    
    // BUG FIX 1: Prevent division by zero if hull_area is 0
    double solidity = (hull_area > 0.001) ? area / hull_area : 0;
    double extent = area / (double)(bbox.width * bbox.height);

    // Convexity defects for finger counting
    vector<int> hull_indices;
    // We MUST use indices for convexityDefects to work correctly with the contour
    convexHull(cnt, hull_indices, false);
    
    // BUG FIX 2: convexityDefects REQUIRES hull_indices to be in clockwise or counter-clockwise order
    // as they appear on the contour. sort() might break this ordering depending on implementation.
    // However, OpenCV's convexHull already returns them in order. Let's ensure they are valid.
    
    vector<Vec4i> defects;
    int fingers = 0;

    if (hull_indices.size() > 3) {
        try {
            convexityDefects(cnt, hull_indices, defects);
            for (const auto& defect : defects) {
                Point start = cnt[defect[0]];
                Point end = cnt[defect[1]];
                Point far = cnt[defect[2]];
                double depth = defect[3] / 256.0;

                // BUG FIX 3: Refined depth and angle filters for better finger isolation
                if (depth > 20) { 
                    double a = norm(end - start);
                    double b = norm(far - start);
                    double c = norm(end - far);
                    
                    // Cosine rule to find the angle at the defect point (between fingers)
                    double cos_theta = (b * b + c * c - a * a) / (2 * b * c);
                    
                    // Clamp for safety
                    if (cos_theta < -1.0) cos_theta = -1.0;
                    if (cos_theta > 1.0) cos_theta = 1.0;
                    
                    double angle = acos(cos_theta) * 57.29;
                    if (angle <= 90) fingers++;
                }
            }
        } catch (...) {
            // Silently fail if defects can't be computed for complex shapes
        }
    }

    // --- REFINED SIGN LOGIC (Based on common ASL shapes) ---
    // Note: fingers 1-4 are usually the gaps between 2-5 fingers.
    // 0 defects = 1 finger or 0 fingers (fist)
    // 1 defect  = 2 fingers (gap between them)
    // 4 defects = 5 fingers (gaps between all 5)

    if (fingers == 0) {
        if (solidity > 0.90) return "A"; // Solid fist
        if (aspect_ratio > 1.4) return "C"; // Curved/wide
        if (extent < 0.5) return "S"; // Compact fist
        return "None";
    } 
    else if (fingers == 1) {
        if (aspect_ratio > 0.8) return "L"; // Thumb + Index
        if (aspect_ratio < 0.6) return "D"; // Index pointing up
        return "G";
    } 
    else if (fingers == 2) {
        if (aspect_ratio < 0.7) return "V"; // Victory
        return "H";
    } 
    else if (fingers == 3) {
        return "W";
    } 
    else if (fingers == 4) {
        if (solidity > 0.8) return "B"; // Flat palm (high solidity)
        return "Open"; // Spread hand
    }

    return "Unknown";
}
