#pragma once

#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;

struct DetectorConfig {
    int min_h = 0, max_h = 25;
    int min_s = 48, max_s = 180;
    int min_v = 60, max_v = 255;
    int min_area = 1500;
    int max_area = 180000;
    double min_aspect_ratio = 0.25;
    double max_aspect_ratio = 3.5;
    double max_solidity = 0.93;
    int morph_kernel = 3;
    int morph_iterations = 2;
    int pre_hsv_blur = 7;
    int calibration_box_size = 80;
    int calibration_offset = 40;
    int h_tolerance = 25;
    int s_tolerance_down = 65;
    int s_tolerance_up = 120;
    int v_tolerance_down = 90;
    int v_tolerance_up = 140;
    int min_s_floor = 25;
    int min_v_floor = 30;
    bool roi_enabled = true;
    double roi_y_start_fraction = 0.25;
    double roi_y_end_fraction = 1.0;
    double roi_x_start_fraction = 0.0;
    double roi_x_end_fraction = 1.0;
    double min_motion_factor = 0.03;
    double track_motion_factor = 0.01;
};

struct RecognizerConfig {
    int min_contour_area = 3500;
    int min_defect_depth = 20;
    int max_finger_angle_deg = 90;
    double min_rule_confidence = 0.55;
};

struct WordBuilderConfig {
    int stable_threshold_frames = 25;
    int re_trigger_delay_frames = 15;
};

struct TTSConfig {
    bool enabled = true;
    string primary = "spd-say";
    string fallback = "espeak-ng";
};

struct AppConfig {
    DetectorConfig detector;
    RecognizerConfig recognizer;
    WordBuilderConfig wordBuilder;
    TTSConfig tts;
};

bool loadConfig(const string& configPath, AppConfig& cfg);
