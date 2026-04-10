#include "HandDetector.h"
#include <iostream>

using namespace cv;
using namespace std;

static double rectIoU(const Rect& a, const Rect& b) {
    Rect inter = a & b;
    if (inter.area() <= 0) return 0.0;
    double unionArea = static_cast<double>(a.area() + b.area() - inter.area());
    if (unionArea <= 0.0) return 0.0;
    return static_cast<double>(inter.area()) / unionArea;
}

HandDetector::HandDetector() {
    applyConfig(DetectorConfig{});
    string cascadePath = "./haarcascade_frontalface_default.xml";
    if (!faceCascade.load(cascadePath)) {
        cerr << "Error: Could not load face cascade" << endl;
    }
    bgSubtractor = createBackgroundSubtractorMOG2(500, 16, true);
}

HandDetector::HandDetector(const DetectorConfig& config) {
    applyConfig(config);
    string cascadePath = "./haarcascade_frontalface_default.xml";
    if (!faceCascade.load(cascadePath)) {
        cerr << "Error: Could not load face cascade" << endl;
    }
    bgSubtractor = createBackgroundSubtractorMOG2(500, 16, true);
}

void HandDetector::applyConfig(const DetectorConfig& config) {
    cfg = config;
    min_h = cfg.min_h;
    max_h = cfg.max_h;
    min_s = cfg.min_s;
    max_s = cfg.max_s;
    min_v = cfg.min_v;
    max_v = cfg.max_v;
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
    int box_size = cfg.calibration_box_size, offset = cfg.calibration_offset;
    Rect roi(input.cols - box_size - offset, input.rows - box_size - offset, box_size, box_size);
    Mat sample = hsv(roi);
    Scalar mean_val, stddev;
    meanStdDev(sample, mean_val, stddev);

    min_h = static_cast<int>(mean_val[0]) - cfg.h_tolerance;
    max_h = static_cast<int>(mean_val[0]) + cfg.h_tolerance;
    if (min_h < 0) min_h += 180;
    if (max_h > 179) max_h -= 180;
    min_s = max(cfg.min_s_floor, static_cast<int>(mean_val[1]) - cfg.s_tolerance_down);
    max_s = min(255, static_cast<int>(mean_val[1]) + cfg.s_tolerance_up);
    min_v = max(cfg.min_v_floor, static_cast<int>(mean_val[2]) - cfg.v_tolerance_down);
    max_v = min(255, static_cast<int>(mean_val[2]) + cfg.v_tolerance_up);
}

vector<Point> HandDetector::detect(Mat input) {
    frame = input;
    Mat hsv, blurred, gray;
    
    // 1. Core Processing
    int blurSize = max(3, cfg.pre_hsv_blur | 1);
    GaussianBlur(frame, blurred, Size(blurSize, blurSize), 0);
    cvtColor(blurred, hsv, COLOR_BGR2HSV);
    cvtColor(frame, gray, COLOR_BGR2GRAY);

    // 2. Detection Layers
    Mat skinMask;
    if (min_h > max_h) {
        Mat mask1, mask2;
        inRange(hsv, Scalar(0, min_s, min_v), Scalar(max_h, max_s, max_v), mask1);
        inRange(hsv, Scalar(min_h, min_s, min_v), Scalar(179, max_s, max_v), mask2);
        skinMask = mask1 | mask2;
    } else {
        inRange(hsv, Scalar(min_h, min_s, min_v), Scalar(max_h, max_s, max_v), skinMask);
    }

    bgSubtractor->apply(blurred, fgMask, 0.002);
    threshold(fgMask, fgMask, 220, 255, THRESH_BINARY);

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

    Mat roiMask = Mat::zeros(frame.size(), CV_8UC1);
    if (cfg.roi_enabled) {
        int x1 = static_cast<int>(frame.cols * cfg.roi_x_start_fraction);
        int x2 = static_cast<int>(frame.cols * cfg.roi_x_end_fraction);
        int y1 = static_cast<int>(frame.rows * cfg.roi_y_start_fraction);
        int y2 = static_cast<int>(frame.rows * cfg.roi_y_end_fraction);
        x1 = max(0, min(frame.cols - 1, x1));
        x2 = max(1, min(frame.cols, x2));
        y1 = max(0, min(frame.rows - 1, y1));
        y2 = max(1, min(frame.rows, y2));
        rectangle(roiMask, Rect(Point(x1, y1), Point(x2, y2)), Scalar(255), FILLED);
    } else {
        roiMask = Mat::ones(frame.size(), CV_8UC1) * 255;
    }

    // 3. COMBINE: Keep skin in ROI that is NOT a face
    mask = skinMask & faceMask & roiMask;
    
    // 4. CLEANUP
    int kernel = max(3, cfg.morph_kernel | 1);
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(kernel, kernel));
    morphologyEx(mask, mask, MORPH_OPEN, element);
    dilate(mask, mask, element, Point(-1, -1), max(1, cfg.morph_iterations));
    morphologyEx(fgMask, fgMask, MORPH_OPEN, element);
    dilate(fgMask, fgMask, element, Point(-1, -1), 1);

    // 5. FIND HAND (Best Score Heuristic)
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Point> best_hand;
    double max_score = -1;

    for (const auto& cnt : contours) {
        double area = contourArea(cnt);
        if (area < cfg.min_area || area > cfg.max_area) continue;

        // Heuristics
        Rect bbox = boundingRect(cnt);
        double aspect_ratio = (double)bbox.width / bbox.height;
        
        vector<Point> hull;
        convexHull(cnt, hull);
        double hullArea = contourArea(hull);
        double solidity = (hullArea > 0) ? area / hullArea : 0;

        // Motion overlap used as strict filter against wall/background blobs
        Mat roi_fg = fgMask(bbox);
        double motionFactor = (double)countNonZero(roi_fg) / area;
        double iou = has_prev_bbox ? rectIoU(prev_bbox, bbox) : 0.0;

        bool motionOk = motionFactor >= cfg.min_motion_factor;
        if (has_prev_bbox && iou > 0.15) {
            motionOk = motionFactor >= cfg.track_motion_factor;
        }

        bool stableShape =
            solidity < cfg.max_solidity &&
            aspect_ratio >= cfg.min_aspect_ratio &&
            aspect_ratio <= cfg.max_aspect_ratio &&
            area >= cfg.min_area * 1.15 &&
            bbox.y > static_cast<int>(frame.rows * 0.22);

        // Reject static background: objects MUST have motion unless already actively tracked
        if (!motionOk) {
            // Allow tracking a hand that stopped moving, provided it maintains a stable shape
            if (has_prev_bbox && iou > 0.15) {
                if (!stableShape) continue;
            } else {
                // Reject new motionless blobs (e.g. skin-colored background walls)
                continue;
            }
        }

        // SCORING ENGINE:
        double score = area;
        if (solidity > cfg.max_solidity) score *= 0.2;
        if (aspect_ratio > cfg.max_aspect_ratio || aspect_ratio < cfg.min_aspect_ratio) score *= 0.1;
        if (motionFactor > 0.12) score *= 1.8;
        if (!motionOk && stableShape) score *= 0.85;
        if (bbox.y < static_cast<int>(frame.rows * 0.18)) score *= 0.25;
        if (has_prev_bbox) {
            // Heavily favor locking onto the actively tracked object
            score *= (1.0 + 4.0 * iou);
        }

        if (score > max_score && score > cfg.min_area * 0.4) {
            max_score = score;
            best_hand = cnt;
            prev_bbox = bbox;
            has_prev_bbox = true;
        }
    }

    if (best_hand.empty()) {
        has_prev_bbox = false;
    }

    return best_hand;
}
