#!/bin/bash

cd "`dirname "$0"`"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./bin/linux
exec ./bin/linux/SDL2GamepadTest.x86_64
