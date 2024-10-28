#!/bin/sh

initial_dir=$(dirname "$(readlink -f "$0")")
CMAKE_PATH="/usr/bin/cmake"

if [ ! -x "$CMAKE_PATH" ]; then
    echo "cmake is not installed. Please install cmake and try again."
    exit 1
fi

rm -rf "$initial_dir/build"
mkdir -p "$initial_dir/build"
cd "$initial_dir/build" || { echo "Failed to change to the build directory."; exit 1; }

"$CMAKE_PATH" .. || { echo "Error while running cmake."; exit 1; }
make || { echo "Error while running make."; exit 1; }
cd "$initial_dir" || { echo "Failed to exit build directory."; exit 1; }

CONFIG_FILE="$initial_dir/config.txt"
DAEMON_FILE="$initial_dir/build/daemon"

sudo "$DAEMON_FILE" "$CONFIG_FILE" &
sleep 2

if pgrep -f "$DAEMON_FILE" > /dev/null; then
    sudo pkill -f "$DAEMON_FILE"
fi

sudo rm -f "$DAEMON_FILE"
