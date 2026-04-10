#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

using namespace std;

class TTSEngine {
private:
    bool enabled = true;
    string primaryCmd = "spd-say";
    string fallbackCmd = "espeak-ng";
    atomic<bool> running{false};
    atomic<bool> speaking{false};
    atomic<bool> available{false};
    atomic<bool> errorState{false};
    thread worker;
    mutex queueMutex;
    queue<string> speakQueue;

    void runLoop();
    bool runCommand(const string& cmd, const string& text) const;
    static string escapeShell(const string& text);
    static bool commandExists(const string& cmd);

public:
    TTSEngine() = default;
    ~TTSEngine();
    void configure(bool isEnabled, const string& primary, const string& fallback);
    void start();
    void stop();
    void speakAsync(const string& text);
    bool isSpeaking() const;
    bool hasError() const;
    bool isAvailable() const;
};
