# OpenIGTLink Mobile

A cross-platform mobile application for iOS and Android that captures device orientation data and transmits it to an OpenIGTLink server over TCP/IP.

## Features

- Real-time device orientation tracking using Qt Sensors
- OpenIGTLink protocol integration for medical imaging applications
- Cross-platform support (iOS and Android)
- Clean, intuitive user interface
- Configurable server connection settings

## Building the Application

### Prerequisites

- Qt 6.5 or later with mobile components
- CMake 3.20 or later
- OpenIGTLink library
- Platform-specific tools:
  - **iOS**: Xcode and iOS SDK
  - **Android**: Android Studio, Android SDK, NDK

### Setup

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd mobile_igtl
   ```

2. Install OpenIGTLink:
   ```bash
   git clone https://github.com/openigtlink/OpenIGTLink.git third_party/openigtlink
   cd third_party/openigtlink
   mkdir build && cd build
   cmake .. -DCMAKE_INSTALL_PREFIX=../install
   make install
   ```

3. Configure and build:
   ```bash
   mkdir build && cd build
   cmake .. -DQt6_DIR=/path/to/qt6
   make
   ```

## Project Structure

```
mobile_igtl/
├── CMakeLists.txt              # Main CMake configuration
├── src/                        # C++ source files
│   ├── main.cpp               # Application entry point
│   ├── applicationcontroller.*# Main application logic
│   ├── orientationsensor.*    # Device orientation handling
│   ├── igtlclient.*          # OpenIGTLink client implementation
│   └── networkmanager.*      # Network communication layer
├── qml/                       # QML user interface files
│   ├── main.qml              # Main window
│   ├── MainWindow.qml        # App layout
│   ├── ConnectionPanel.qml   # Server connection UI
│   └── OrientationView.qml   # Orientation display
├── android/                   # Android-specific files
├── ios/                       # iOS-specific files
└── third_party/              # External dependencies
    └── openigtlink/          # OpenIGTLink library
```

## Usage

1. Launch the application on your mobile device
2. Enter the OpenIGTLink server hostname and port (default: 18944)
3. Tap "Connect" to establish connection
4. Once connected, tap "Start Sending" to begin transmitting orientation data
5. Move your device to see real-time orientation changes

## OpenIGTLink Server

This application sends orientation data as OpenIGTLink `ORIENTATION` messages. You can test with:

- 3D Slicer with OpenIGTLink extension
- PLUS toolkit
- Custom OpenIGTLink server implementation

## License

[Your license here]