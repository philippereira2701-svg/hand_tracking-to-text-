#include "AppConfig.h"

static int readInt(const FileNode& n, int fallback) {
    return n.empty() ? fallback : static_cast<int>(n.real());
}

static double readDouble(const FileNode& n, double fallback) {
    return n.empty() ? fallback : static_cast<double>(n.real());
}

static bool readBool(const FileNode& n, bool fallback) {
    return n.empty() ? fallback : static_cast<int>(n.real()) != 0;
}

static string readString(const FileNode& n, const string& fallback) {
    return n.empty() ? fallback : static_cast<string>(n);
}

bool loadConfig(const string& configPath, AppConfig& cfg) {
    FileStorage fs;
    if (!fs.open(configPath, FileStorage::READ | FileStorage::FORMAT_JSON)) {
        return false;
    }

    FileNode detector = fs["hand_detector"];
    if (!detector.empty()) {
        FileNode hsv = detector["hsv"];
        cfg.detector.min_h = readInt(hsv["min_h"], cfg.detector.min_h);
        cfg.detector.max_h = readInt(hsv["max_h"], cfg.detector.max_h);
        cfg.detector.min_s = readInt(hsv["min_s"], cfg.detector.min_s);
        cfg.detector.max_s = readInt(hsv["max_s"], cfg.detector.max_s);
        cfg.detector.min_v = readInt(hsv["min_v"], cfg.detector.min_v);
        cfg.detector.max_v = readInt(hsv["max_v"], cfg.detector.max_v);

        FileNode contour = detector["contour_filter"];
        cfg.detector.min_area = readInt(contour["min_area"], cfg.detector.min_area);
        cfg.detector.max_area = readInt(contour["max_area"], cfg.detector.max_area);

        FileNode shape = detector["shape_filter"];
        cfg.detector.min_aspect_ratio = readDouble(shape["min_aspect_ratio"], cfg.detector.min_aspect_ratio);
        cfg.detector.max_aspect_ratio = readDouble(shape["max_aspect_ratio"], cfg.detector.max_aspect_ratio);
        cfg.detector.max_solidity = readDouble(shape["max_solidity"], cfg.detector.max_solidity);

        FileNode morph = detector["morphology"];
        cfg.detector.morph_kernel = readInt(morph["kernel_size"], cfg.detector.morph_kernel);
        cfg.detector.morph_iterations = readInt(morph["iterations"], cfg.detector.morph_iterations);

        FileNode blur = detector["blur"];
        cfg.detector.pre_hsv_blur = readInt(blur["pre_hsv_kernel"], cfg.detector.pre_hsv_blur);

        FileNode roi = detector["roi"];
        cfg.detector.roi_enabled = readBool(roi["enabled"], cfg.detector.roi_enabled);
        cfg.detector.roi_y_start_fraction = readDouble(roi["y_start_fraction"], cfg.detector.roi_y_start_fraction);
        cfg.detector.roi_y_end_fraction = readDouble(roi["y_end_fraction"], cfg.detector.roi_y_end_fraction);
        cfg.detector.roi_x_start_fraction = readDouble(roi["x_start_fraction"], cfg.detector.roi_x_start_fraction);
        cfg.detector.roi_x_end_fraction = readDouble(roi["x_end_fraction"], cfg.detector.roi_x_end_fraction);

        FileNode motion = detector["motion_filter"];
        cfg.detector.min_motion_factor = readDouble(motion["min_motion_factor"], cfg.detector.min_motion_factor);
        cfg.detector.track_motion_factor = readDouble(motion["track_motion_factor"], cfg.detector.track_motion_factor);
    }

    FileNode calibration = fs["calibration"];
    if (!calibration.empty()) {
        cfg.detector.calibration_box_size = readInt(calibration["box_size"], cfg.detector.calibration_box_size);
        cfg.detector.calibration_offset = readInt(calibration["offset_from_edge"], cfg.detector.calibration_offset);
        cfg.detector.h_tolerance = readInt(calibration["h_tolerance"], cfg.detector.h_tolerance);
        cfg.detector.s_tolerance_down = readInt(calibration["s_tolerance_down"], cfg.detector.s_tolerance_down);
        cfg.detector.s_tolerance_up = readInt(calibration["s_tolerance_up"], cfg.detector.s_tolerance_up);
        cfg.detector.v_tolerance_down = readInt(calibration["v_tolerance_down"], cfg.detector.v_tolerance_down);
        cfg.detector.v_tolerance_up = readInt(calibration["v_tolerance_up"], cfg.detector.v_tolerance_up);
        cfg.detector.min_s_floor = readInt(calibration["min_s_floor"], cfg.detector.min_s_floor);
        cfg.detector.min_v_floor = readInt(calibration["min_v_floor"], cfg.detector.min_v_floor);
    }

    FileNode recognizer = fs["gesture_recognizer"];
    if (!recognizer.empty()) {
        cfg.recognizer.min_contour_area = readInt(recognizer["min_contour_area"], cfg.recognizer.min_contour_area);
        cfg.recognizer.min_defect_depth = readInt(recognizer["min_defect_depth"], cfg.recognizer.min_defect_depth);
        cfg.recognizer.max_finger_angle_deg = readInt(recognizer["max_finger_angle_deg"], cfg.recognizer.max_finger_angle_deg);
        cfg.recognizer.min_rule_confidence = readDouble(recognizer["min_rule_confidence"], cfg.recognizer.min_rule_confidence);
    }

    FileNode wb = fs["word_builder"];
    if (!wb.empty()) {
        cfg.wordBuilder.stable_threshold_frames = readInt(wb["stable_threshold_frames"], cfg.wordBuilder.stable_threshold_frames);
        cfg.wordBuilder.re_trigger_delay_frames = readInt(wb["re_trigger_delay_frames"], cfg.wordBuilder.re_trigger_delay_frames);
    }

    FileNode tts = fs["tts"];
    if (!tts.empty()) {
        cfg.tts.enabled = readBool(tts["enabled"], cfg.tts.enabled);
        cfg.tts.primary = readString(tts["primary"], cfg.tts.primary);
        cfg.tts.fallback = readString(tts["fallback"], cfg.tts.fallback);
    }

    fs.release();
    return true;
}
