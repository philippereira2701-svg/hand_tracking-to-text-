#!/bin/bash
g++ main.cpp HandDetector.cpp GestureRecognizer.cpp -o sign_lang `pkg-config --cflags --libs opencv4`
if [ $? -eq 0 ]; then
    echo "Compilation successful. Run with ./sign_lang"
else
    echo "Compilation failed. Make sure OpenCV 4 is installed."
fi
