# insynic 1.21Lite Release Notes

## 🎉 New Features

### Per-Device Streaming Settings Profile
- Each device now has its own independent Streaming Settings profile
- Right-click any device in the list and select "Streaming Settings..." to configure
- Settings are automatically saved by device serial number and applied on next connection

### Extended scrcpy Connection Options
Added a new "scrcpy Options" group in the Streaming Settings dialog:
- **Turn screen off during mirroring**: Automatically turn off the device screen when mirroring starts to save battery
- **Keep device awake**: Prevent the device from sleeping while connected
- **Power on on connect**: Automatically wake the device screen when connection is established
- **Disable screensaver**: Disable the computer screensaver while a device is connected
- **Enable control (mouse/keyboard)**: Allow controlling the device with mouse and keyboard

### Enhanced Device List Context Menu
- Right-click on a device: shows "Streaming Settings..." + "Connect All Devices"
- Right-click on empty area: shows "Connect All Devices"

## 🔧 Improvements

- Removed "Streaming Settings" from the system menu bar; access it via device right-click menu instead
- Fixed device status color display in Chinese language mode (connected devices now show green instead of orange)
- Optimized window layer management: each device window group (screen + sidebar + control bar) has independent layering, only raised when active
- When switching to other apps, all insynic windows are properly obscured instead of staying on top

## 🐛 Bug Fixes

- Fixed potential crash when disconnecting devices
- Fixed screen window freezing after long-running sessions
- Fixed UI not updating immediately after language switch
- Fixed File Manager button text not updating with language switch
- Fixed device selection anomaly after canceling network connection dialog

## 📦 Other

- Project code structure refactored, source code moved to project root
- Debug output disabled in Release mode for better performance
- Version updated to 1.21Lite
