# insynic 1.10 Lite — Release Notes

## Version

**1.10 Lite**

## Overview

insynic 1.10 Lite is the first public release. Built on the scrcpy engine, it provides macOS users with full Android device control capabilities, integrating screen mirroring, file management, OTG input, virtual key mapping, and more.

## Features

### Device Connection
- USB cable connection with automatic device detection
- Network (TCP/IP) wireless connection with customizable port (default 5555)
- Device dropdown automatically lists available devices

### Screen Mirroring
- Real-time screen mirroring powered by scrcpy
- Streaming settings: configurable resolution, frame rate, and bit rate
- Screen-off control: turn off display while still operating the device (no lock)
- Screen window auto-resizes to match video frame resolution

### Remote Control Buttons
- Navigation: Back, Home, Task, Menu
- Volume: Volume up / down
- Panels: Expand notification bar, expand/collapse settings panel
- Screen rotation: toggle portrait/landscape with one click
- Power: toggle screen on/off

### OTG Input
- Maps Mac keyboard as an Android input device via USB OTG mode
- Runs as a separate process without interfering with screen mirroring
- Supports Cmd+Q shortcut to exit
- Automatically disabled in network connection mode (AOA supports USB only)

### File Management
- ADB-based file browser
- Browse device internal storage files

### Virtual Key Mapping (Utilities)
- Add Key: place draggable virtual key buttons on the screen window
- Double-click to configure: listen for keyboard input mapping, configurable size (20-100 pixels)
- Support for multiple virtual keys simultaneously
- Sends touch events to the device via the scrcpy control protocol
- Save Profile: save window position, size, and all virtual key configurations
- Profile management: Apply to load configurations, Delete to remove configurations

### Other Features
- System tray menu with show/hide window support
- English/Chinese bilingual switching with automatic preference saving
- Native macOS dark theme
- Custom app icon

## Technical Information

- **Platform**: macOS 12.0+
- **Build System**: CMake 3.20+
- **Core Dependencies**: Qt 6, scrcpy, SDL3, FFmpeg, libusb
- **adb and scrcpy-server are bundled in the app package**

## Known Limitations

- OTG input supports USB connection only, not network connection
- Screen mirroring window is a standalone popup, not embedded in the main window
- Only H.264 video codec is supported
- macOS x86_64 architecture only

## Author

厨房 / OMADAFAKA

## Credits

This project is built upon the following open-source tools:
- [scrcpy](https://github.com/Genymobile/scrcpy) — by Romain Vimont
- [Android Debug Bridge (ADB)](https://developer.android.com/tools/adb) — Google
- [Qt](https://www.qt.io/)
- [SDL](https://github.com/libsdl-org/SDL)
- [FFmpeg](https://ffmpeg.org/)

---

© 2025 厨房 / OMADAFAKA. All rights reserved.
