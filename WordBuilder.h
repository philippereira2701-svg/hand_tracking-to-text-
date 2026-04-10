#pragma once
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class WordBuilder {
private:
    string current_sentence = "";
    string last_letter = "None";
    int stable_count = 0;
    int stable_threshold = 25;
    int re_trigger_delay_frames = 15;
    bool letter_triggered = false;

public:
    void configure(int stableThresholdFrames, int reTriggerDelayFrames) {
        if (stableThresholdFrames > 0) {
            stable_threshold = stableThresholdFrames;
        }
        if (reTriggerDelayFrames >= 0) {
            re_trigger_delay_frames = reTriggerDelayFrames;
        }
    }

    string process(const string& letter) {
        if (letter == "None" || letter == "Unknown") {
            stable_count = 0;
            letter_triggered = false;
            last_letter = "None";
            return "";
        }

        if (letter == last_letter) {
            stable_count++;
        } else {
            last_letter = letter;
            stable_count = 0;
            letter_triggered = false;
        }

        string committed = "";
        if (stable_count >= stable_threshold && !letter_triggered) {
            if (letter == "Space") {
                current_sentence += " ";
                committed = " ";
            } else if (letter == "Backspace") {
                if (!current_sentence.empty()) {
                    current_sentence.pop_back();
                }
            } else if (letter == "Clear") {
                current_sentence = "";
            } else if (letter == "Speak") {
                committed = "__SPEAK__";
            } else if (letter != "Open") { // Ignore "Open" as an actual character
                current_sentence += letter;
                committed = letter;
            }

            letter_triggered = true;
            stable_count = -re_trigger_delay_frames;
        }

        if (stable_count < 0 && letter != last_letter) {
            stable_count = 0;
            letter_triggered = false;
        }
        return committed;
    }

    void clearSentence() {
        current_sentence.clear();
        stable_count = 0;
        letter_triggered = false;
        last_letter = "None";
    }

    string getSentence() const {
        return current_sentence;
    }

    float getProgress() const {
        if (stable_count < 0) return 1.0f;
        if (stable_count > stable_threshold) return 1.0f;
        return static_cast<float>(stable_count) / static_cast<float>(stable_threshold);
    }
};
