#!/bin/bash

PLATFORM_ID=3
DEVICE_ID=0

export CL_LOG_ERRORS="stdout"

./build/src/GpuFFT $PLATFORM_ID $DEVICE_ID
