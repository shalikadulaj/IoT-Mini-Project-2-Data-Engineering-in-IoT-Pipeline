#!/bin/bash

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo "git is not installed. Please install git and try again."
    exit 1
fi

# Clone the AWS IoT Device SDK for Python repository if it does not exist
if [ ! -d "aws-iot-device-sdk-python" ]; then
    git clone https://github.com/aws/aws-iot-device-sdk-python.git
fi

# Check if the clone was successful
if [ ! -d "aws-iot-device-sdk-python" ]; then
    echo "Failed to clone the AWS IoT Device SDK. Exiting script."
    exit 1
fi

# Change into the SDK directory
cd aws-iot-device-sdk-python

# Install the Python package
python3 setup.py install

# Check if the installation was successful
if [ $? -ne 0 ]; then
    echo "Failed to install the AWS IoT Device SDK. Exiting script."
    exit 1
fi

# Return to the original directory
cd ..

# Clone the IoT Mini Project repository
if [ ! -d "IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline" ]; then
    git clone https://github.com/shalikadulaj/IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline.git
fi

# Check if the clone was successful
if [ ! -d "IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline" ]; then
    echo "Failed to clone the IoT Mini Project repository. Exiting script."
    exit 1
fi

echo "AWS IoT SDK and IoT Mini Project repository have been successfully cloned and installed."
