#include "TTSEngine.h"
#include <cstdlib>
#include <chrono>

TTSEngine::~TTSEngine() {
    stop();
}

void TTSEngine::configure(bool isEnabled, const string& primary, const string& fallback) {
    enabled = isEnabled;
    if (!primary.empty()) primaryCmd = primary;
    if (!fallback.empty()) fallbackCmd = fallback;
    available = enabled && (commandExists(primaryCmd) || commandExists(fallbackCmd));
}

void TTSEngine::start() {
    if (!enabled || running) return;
    running = true;
    worker = thread(&TTSEngine::runLoop, this);
}

void TTSEngine::stop() {
    running = false;
    if (worker.joinable()) {
        worker.join();
    }
}

void TTSEngine::speakAsync(const string& text) {
    if (!enabled || text.empty()) return;
    lock_guard<mutex> lock(queueMutex);
    if (speakQueue.size() < 12) {
        speakQueue.push(text);
    }
}

bool TTSEngine::isSpeaking() const {
    return speaking;
}

bool TTSEngine::hasError() const {
    return errorState;
}

bool TTSEngine::isAvailable() const {
    return available;
}

void TTSEngine::runLoop() {
    while (running) {
        string nextText;
        {
            lock_guard<mutex> lock(queueMutex);
            if (!speakQueue.empty()) {
                nextText = speakQueue.front();
                speakQueue.pop();
            }
        }

        if (nextText.empty()) {
            this_thread::sleep_for(chrono::milliseconds(20));
            continue;
        }

        speaking = true;
        bool success = false;
        if (commandExists(primaryCmd)) {
            success = runCommand(primaryCmd, nextText);
        }
        if (!success && commandExists(fallbackCmd)) {
            success = runCommand(fallbackCmd, nextText);
        }
        errorState = !success;
        speaking = false;
    }
}

bool TTSEngine::runCommand(const string& cmd, const string& text) const {
    string safe = escapeShell(text);
    string systemCmd;
    if (cmd == "spd-say") {
        systemCmd = "spd-say '" + safe + "'";
    } else if (cmd == "espeak-ng") {
        systemCmd = "espeak-ng '" + safe + "'";
    } else {
        systemCmd = cmd + " '" + safe + "'";
    }
    int rc = system(systemCmd.c_str());
    return rc == 0;
}

string TTSEngine::escapeShell(const string& text) {
    string out;
    out.reserve(text.size());
    for (char c : text) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    return out;
}

bool TTSEngine::commandExists(const string& cmd) {
    if (cmd.empty()) return false;
    string check = "command -v " + cmd + " >/dev/null 2>&1";
    return system(check.c_str()) == 0;
}
