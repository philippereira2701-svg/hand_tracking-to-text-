#!/bin/bash
set -e

# Force system toolchain; avoid conda-injected linker/runtime paths.
export PATH="/usr/bin:/bin:/usr/sbin:/sbin"
unset LD_LIBRARY_PATH
unset LIBRARY_PATH
unset CPATH
unset C_INCLUDE_PATH
unset CPLUS_INCLUDE_PATH
unset PKG_CONFIG_PATH
unset CONDA_PREFIX
unset CONDA_DEFAULT_ENV

chmod +x ./env_check.sh
./env_check.sh

CPP_COMPILER="/usr/bin/g++"
if [ ! -x "$CPP_COMPILER" ]; then
  CPP_COMPILER="g++"
fi

$CPP_COMPILER -B/usr/bin RecognizerRuleTests.cpp GestureRecognizer.cpp FeatureExtractor.cpp -std=c++17 -o rule_tests `pkg-config --cflags opencv4` -lopencv_core -lopencv_imgproc
./rule_tests
