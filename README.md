[简体中文](https://github.com/benknam/insynic/blob/main/README_CN.md)

A macOS desktop application for controlling Android devices

[Features](#features) | [Screenshots](#screenshots) | [Installation](#installation--usage) | [Build from Source](#build-from-source) | [Tech Stack](#tech-stack) | [Credits](#credits)

</div>

---

## Overview

insynic is a native macOS desktop application for controlling and operating Android devices from a Mac. Built on top of the [scrcpy](https://github.com/Genymobile/scrcpy) engine, it integrates screen mirroring, file management, OTG input, virtual key mapping, and more, with bilingual support for English and Chinese.

## Features

### Device Connection
- **USB Connection**: Connect Android devices via USB cable with automatic device detection
- **Network Connection**: Connect to remote Android devices via TCP/IP (WiFi) with customizable port (default 5555)
- **Auto Device Detection**: Automatically scans and lists all available devices on startup

### Screen Mirroring
- Real-time screen mirroring powered by the scrcpy engine
- Customizable resolution, frame rate, and bit rate
- Screen-off control (turn off display while still operating the device)
- Screen window auto-resizes to match video frame resolution

### Remote Control
- **Navigation Keys**: Back, Home, Task, Menu
- **Volume Control**: Volume up, Volume down
- **System Panels**: Expand/collapse notification and settings panels
- **Screen Rotation**: Rotate device screen orientation with one click
- **Power Control**: Toggle screen on/off without locking the device

### OTG Input
- Maps Mac keyboard as an Android input device via USB OTG mode
- Runs as a separate process without interfering with screen mirroring
- Supports Cmd+Q shortcut to exit OTG mode

### File Management
- ADB-based file browser
- Browse and manage files on device internal storage

### Virtual Key Mapping
- **Add Virtual Keys**: Place draggable virtual key buttons on the screen window
- **Key Configuration**: Double-click a virtual key to listen for keyboard input mapping; configurable key size (20-100 pixels)
- **Multi-key Support**: Add multiple virtual keys simultaneously
- **Touch Simulation**: Sends touch events to the device via the scrcpy control protocol

### Profile Management
- **Save Profiles**: Save current window position, size, and all virtual key configurations
- **Custom Naming**: Support for custom profile names with overwrite confirmation
- **Apply Profiles**: Load saved configurations with one click
- **Delete Profiles**: Remove unwanted configuration files

### Other Features
- **System Tray**: Menu bar icon with show/hide window support
- **Streaming Settings**: Configurable resolution, frame rate, and bit rate
- **Internationalization**: English/Chinese language switching with automatic preference saving
- **Dark Theme**: Native macOS dark interface

## Screenshots

> Screenshots will be added later

## Installation & Usage

### System Requirements

- macOS 12.0 or later
- Android device with USB debugging enabled

### Prerequisites

1. Enable **Developer Options** and **USB Debugging** on your Android device
2. Authorize USB debugging on the device when connecting via USB for the first time

### Download & Install

1. Download the latest `insynic.app` from the [Releases](../../releases) page
2. Drag the app into the `Applications` folder
3. On first launch, if prompted by security settings, go to `System Settings → Privacy & Security → Allow Anyway`

### Basic Usage

1. Connect your Android device to your Mac via USB (or use network connection)
2. Select your device from the dropdown list (or choose "Network Connection" to enter IP and port)
3. Click "Connect" to connect to the device
4. The screen window will pop up automatically upon successful connection
5. Use the buttons on the control panel to operate the device

## Build from Source

### Build Dependencies

- **CMake** >= 3.20
- **Qt** 6.x (install via Homebrew: `brew install qtbase`)
- **SDL3** (install via Homebrew: `brew install sdl3`)
- **FFmpeg** (install via Homebrew: `brew install ffmpeg`)
- **libusb** (install via Homebrew: `brew install libusb`)
- **scrcpy source**: The scrcpy app source directory is required for compilation

### Build Steps

```bash
# 1. Clone the repository
git clone https://github.com/your-username/insynic.git
cd insynic

# 2. Ensure scrcpy source is in the parent directory
# Expected directory structure:
#   insynic/          <- This project
#   scrcpy/           <- scrcpy source

# 3. Create build directory and compile
cd insynic
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

After building, the app is located at `build/insynic.app`.

### Build Notes

The build process automatically:
- Compiles the scrcpy static library
- Bundles `adb` and `scrcpy-server` into the app's Resources directory
- Deploys Qt frameworks using `macdeployqt`
- Copies translation files into the app bundle

## Project Structure

```
insynic/
├── insynic/
│   ├── CMakeLists.txt           # Main build configuration
│   ├── Info.plist.in            # macOS app info template
│   ├── src/
│   │   ├── main.cpp             # Application entry point
│   │   ├── scrcpy_lib/          # scrcpy wrapper layer (C)
│   │   ├── ui/                  # User interface layer
│   │   │   ├── insynic_mainwindow.*      # Main window
│   │   │   ├── insynic_settingsdialog.*  # Streaming settings dialog
│   │   │   ├── insynic_networkdialog.*  # Network connection dialog
│   │   │   ├── insynic_filebrowser.*    # File browser
│   │   │   ├── insynic_customtranslator.*# Custom translator
│   │   │   ├── macos_menu.mm            # macOS native menu
│   │   │   └── qt_nsview.mm            # NSView embedding
│   │   ├── controlpanel/        # Control panel module
│   │   │   ├── insynic_controlpanel.*   # Control panel
│   │   │   ├── insynic_draggablekey.*   # Draggable virtual key
│   │   │   ├── insynic_keyconfigdialog.*# Key config dialog
│   │   │   ├── insynic_saveprofiledialog.*# Save profile dialog
│   │   │   └── insynic_profilemanager.*  # Profile manager
│   │   └── filemanager/        # ADB file management module
│   └── translations/
│       └── insynic_zh.ts        # Chinese translation file
└── README.md
```

## Tech Stack

| Technology | Purpose |
|------------|---------|
| [Qt 6](https://www.qt.io/) | Application framework, UI |
| [scrcpy](https://github.com/Genymobile/scrcpy) | Android screen mirroring & control engine |
| [SDL3](https://github.com/libsdl-org/SDL) | Video rendering window |
| [FFmpeg](https://ffmpeg.org/) | Video decoding (H.264) |
| [ADB](https://developer.android.com/tools/adb) | Android Debug Bridge, device communication |
| [libusb](https://libusb.info/) | USB communication (OTG mode) |
| CMake | Build system |

## Credits

This project is built upon the following open-source projects. Thanks to their developers:

- **scrcpy** — Android screen mirroring and control tool, by [Romain Vimont](https://github.com/rom1v)
- **Android Debug Bridge (ADB)** — Google's official Android debugging tool
- **Qt** — Cross-platform C++ application development framework
- **SDL** — Simple DirectMedia Layer
- **FFmpeg** — Multimedia processing library

## Author

**厨房 / OMADAFAKA**
