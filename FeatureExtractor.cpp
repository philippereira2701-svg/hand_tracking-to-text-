#include "FeatureExtractor.h"
#include <cmath>

static double computeAngleDeg(const Point& a, const Point& b, const Point& c) {
    double ab = norm(a - b);
    double cb = norm(c - b);
    double ac = norm(a - c);
    if (ab < 1e-3 || cb < 1e-3) {
        return 180.0;
    }
    double cosTheta = (ab * ab + cb * cb - ac * ac) / (2.0 * ab * cb);
    cosTheta = max(-1.0, min(1.0, cosTheta));
    return acos(cosTheta) * 180.0 / CV_PI;
}

GestureFeatures FeatureExtractor::extract(const vector<Point>& contour, int minDefectDepth, int maxFingerAngleDeg) const {
    GestureFeatures out;
    if (contour.empty()) {
        return out;
    }

    Moments m = moments(contour);
    if (fabs(m.m00) > 1e-3) {
        out.palm_center = Point2f(static_cast<float>(m.m10 / m.m00), static_cast<float>(m.m01 / m.m00));
    }

    out.area = contourArea(contour);
    Rect bbox = boundingRect(contour);
    out.aspect_ratio = (bbox.height > 0) ? static_cast<double>(bbox.width) / bbox.height : 0.0;
    out.extent = (bbox.width > 0 && bbox.height > 0) ? out.area / static_cast<double>(bbox.width * bbox.height) : 0.0;

    convexHull(contour, out.hull_points);
    double hull_area = contourArea(out.hull_points);
    out.solidity = (hull_area > 1e-3) ? out.area / hull_area : 0.0;

    convexHull(contour, out.hull_indices, false);
    if (out.hull_indices.size() <= 3) {
        out.valid = true;
        return out;
    }

    try {
        convexityDefects(contour, out.hull_indices, out.defects);
    } catch (...) {
        out.valid = true;
        return out;
    }

    double depth_sum = 0.0;
    int valid_defects = 0;
    for (const auto& defect : out.defects) {
        Point start = contour[defect[0]];
        Point end = contour[defect[1]];
        Point far = contour[defect[2]];
        double depth = defect[3] / 256.0;
        double angle = computeAngleDeg(start, far, end);

        if (depth >= minDefectDepth && angle <= maxFingerAngleDeg) {
            valid_defects++;
            depth_sum += depth;
            out.fingertip_candidates.push_back(start);
            out.fingertip_candidates.push_back(end);
        }
    }

    out.defect_count = valid_defects;
    out.finger_count = min(5, valid_defects + 1);
    out.avg_defect_depth = valid_defects > 0 ? depth_sum / valid_defects : 0.0;
    out.valid = true;
    return out;
}
