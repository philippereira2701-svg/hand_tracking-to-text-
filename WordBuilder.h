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
    const int THRESHOLD = 25; // Number of frames for a letter to lock in
    bool letter_triggered = false; // BUG FIX: Track if current letter has been added

public:
    void process(string letter) {
        if (letter == "None" || letter == "Unknown") {
            stable_count = 0;
            letter_triggered = false;
            last_letter = "None";
            return;
        }

        if (letter == last_letter) {
            stable_count++;
        } else {
            last_letter = letter;
            stable_count = 0;
            letter_triggered = false; // Reset trigger for new letter
        }

        // Logic fix: Only add the letter once per stable event
        if (stable_count >= THRESHOLD && !letter_triggered) {
            if (letter == "Space") {
                current_sentence += " ";
            } else if (letter == "Backspace") {
                if (!current_sentence.empty()) current_sentence.pop_back();
            } else if (letter == "Clear") {
                current_sentence = "";
            } else if (letter != "Open") { // Ignore "Open" as an actual character
                current_sentence += letter;
            }
            
            letter_triggered = true; // Prevents spamming the letter while holding the sign
            stable_count = -15; // Delay before next detection (cool-down)
        }
        
        // BUG FIX: Allow stable_count to recover from negative cool-down
        if (stable_count < 0 && letter != last_letter) {
            stable_count = 0;
            letter_triggered = false;
        }
    }

    string getSentence() const {
        return current_sentence;
    }

    float getProgress() const {
        if (stable_count < 0) return 1.0f;
        if (stable_count > THRESHOLD) return 1.0f;
        return (float)stable_count / THRESHOLD;
    }
};
