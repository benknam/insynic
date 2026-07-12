# insynic

---

<div align="center">
<img width="266" height="642" alt="image" src="https://github.com/user-attachments/assets/95dc0b68-7435-4de2-8dd5-4872865e320d" /><img width="269" height="645" alt="image" src="https://github.com/user-attachments/assets/74c44a7a-00f8-4a69-8ad0-9d981136cf13" />
<img width="1082" height="514" alt="image" src="https://github.com/user-attachments/assets/c58c2f4a-a5c0-48b2-bbc1-443812d40322" />
</div>

---


<div align="center">

一个用于控制 Android 设备的 macOS 桌面应用程序

[功能特性](#功能特性) | [截图](#截图) | [安装与使用](#安装与使用) | [从源码构建](#从源码构建) | [技术栈](#技术栈) | [致谢](#致谢)

</div>

---

## 简介

insynic 是一款 macOS 原生桌面应用程序，用于在 Mac 上控制和操作 Android 设备。它基于 [scrcpy](https://github.com/Genymobile/scrcpy) 引擎构建，集成了屏幕镜像、文件管理、OTG 输入、虚拟按键映射等功能，并提供中英文双语界面。

## 功能特性

### 设备连接
- **USB 连接**：通过 USB 数据线连接 Android 设备，自动检测已连接设备
- **网络连接**：通过 TCP/IP (WiFi) 连接远程 Android 设备，支持自定义端口（默认 5555）
- **自动设备检测**：启动时自动扫描并列出所有可用设备

### 屏幕镜像
- 基于 scrcpy 引擎的实时屏幕镜像
- 支持自定义分辨率、帧率、比特率
- 支持关屏控制（关闭屏幕显示但仍可操作设备）
- 屏幕窗口大小自动适配视频帧分辨率

### 远程控制
- **导航按键**：Back、Home、Task、Menu
- **音量控制**：音量增大、音量减小
- **系统面板**：通知栏、设置面板展开/收起
- **屏幕旋转**：一键旋转设备屏幕方向
- **电源控制**：关屏/开屏切换（不锁屏）

### OTG 输入
- 通过 USB OTG 模式将 Mac 键盘映射为 Android 输入设备
- 独立进程运行，不干扰主程序屏幕镜像
- 支持 Cmd+Q 快捷键退出 OTG 模式

### 文件管理
- 基于 ADB 的文件浏览器
- 支持设备内部存储文件浏览和管理

### 虚拟按键映射
- **添加虚拟按键**：在屏幕窗口上放置可拖动的虚拟按键
- **按键配置**：双击虚拟按键，监听键盘输入进行映射，可配置按键大小（20-100 像素）
- **多按键支持**：可同时添加多个虚拟按键
- **触摸模拟**：通过 scrcpy 控制协议发送触摸事件到设备对应坐标

### Profile 配置管理
- **保存配置**：保存当前窗口位置、大小及所有虚拟按键配置
- **配置命名**：支持自定义命名，支持覆盖确认
- **配置应用**：一键加载已保存的配置
- **配置删除**：删除不需要的配置文件

### 其他功能
- **系统状态栏**：菜单栏图标，支持显示/隐藏窗口
- **串流设置**：可配置分辨率、帧率、比特率
- **国际化**：支持中英文切换，语言偏好自动保存
- **深色主题**：原生 macOS 深色界面

## 截图

> 截图将在后续补充

## 安装与使用

### 系统要求

- macOS 12.0 或更高版本
- Android 设备需启用 USB 调试

### 前置准备

1. 在 Android 设备上启用 **开发者选项** 和 **USB 调试**
2. 首次通过 USB 连接时，在设备上授权 USB 调试

### 下载安装

1. 从 [Releases](../../releases) 页面下载最新的 `insynic.app`
2. 将应用拖入 `应用程序` 文件夹
3. 首次打开时如遇安全提示，前往 `系统设置 → 隐私与安全性 → 允许打开`

### 基本使用

1. 通过 USB 连接 Android 设备到 Mac（或使用网络连接）
2. 在设备下拉列表中选择设备（或选择「网络连接」输入 IP 和端口）
3. 点击「Connect」连接设备
4. 连接成功后，屏幕窗口自动弹出
5. 使用控制面板上的按钮操作设备

## 从源码构建

### 构建依赖

- **CMake** >= 3.20
- **Qt** 6.x（通过 Homebrew 安装：`brew install qtbase`）
- **SDL3**（通过 Homebrew 安装：`brew install sdl3`）
- **FFmpeg**（通过 Homebrew 安装：`brew install ffmpeg`）
- **libusb**（通过 Homebrew 安装：`brew install libusb`）
- **scrcpy 源码**：需要 scrcpy 的 app 源码目录用于编译

### 构建步骤

```bash
# 1. 克隆仓库
git clone https://github.com/厨房/insynic.git
cd insynic

# 2. 确保 scrcpy 源码在同级目录
# 项目结构应为：
#   insynic/          <- 本项目
#   scrcpy/           <- scrcpy 源码

# 3. 创建构建目录并编译
cd insynic
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

构建完成后，应用位于 `build/insynic.app`。

### 构建说明

构建过程会自动：
- 编译 scrcpy 静态库
- 将 `adb` 和 `scrcpy-server` 打包到应用 Resources 目录
- 使用 `macdeployqt` 部署 Qt 框架
- 复制翻译文件到应用包

## 项目结构

```
insynic/
├── insynic/
│   ├── CMakeLists.txt           # 主构建配置
│   ├── Info.plist.in            # macOS 应用信息模板
│   ├── src/
│   │   ├── main.cpp             # 程序入口
│   │   ├── scrcpy_lib/          # scrcpy 封装层 (C)
│   │   ├── ui/                  # 用户界面层
│   │   │   ├── insynic_mainwindow.*      # 主窗口
│   │   │   ├── insynic_settingsdialog.*  # 串流设置对话框
│   │   │   ├── insynic_networkdialog.*  # 网络连接对话框
│   │   │   ├── insynic_filebrowser.*    # 文件浏览器
│   │   │   ├── insynic_customtranslator.*# 自定义翻译器
│   │   │   ├── macos_menu.mm            # macOS 原生菜单
│   │   │   └── qt_nsview.mm            # NSView 嵌入
│   │   ├── controlpanel/        # 控制面板模块
│   │   │   ├── insynic_controlpanel.*   # 控制面板
│   │   │   ├── insynic_draggablekey.*   # 可拖动虚拟按键
│   │   │   ├── insynic_keyconfigdialog.*# 按键配置对话框
│   │   │   ├── insynic_saveprofiledialog.*# 保存配置对话框
│   │   │   └── insynic_profilemanager.*  # 配置管理器
│   │   └── filemanager/        # ADB 文件管理模块
│   └── translations/
│       └── insynic_zh.ts        # 中文翻译文件
└── README.md
```

## 技术栈

| 技术 | 用途 |
|------|------|
| [Qt 6](https://www.qt.io/) | 应用框架、UI 界面 |
| [scrcpy](https://github.com/Genymobile/scrcpy) | Android 屏幕镜像与控制核心引擎 |
| [SDL3](https://github.com/libsdl-org/SDL) | 视频渲染窗口 |
| [FFmpeg](https://ffmpeg.org/) | 视频解码 (H.264) |
| [ADB](https://developer.android.com/tools/adb) | Android Debug Bridge，设备通信 |
| [libusb](https://libusb.info/) | USB 通信（OTG 模式） |
| CMake | 构建系统 |

## 致谢

本项目基于以下开源项目构建，感谢这些项目的开发者：

- **scrcpy** — Android 屏幕镜像与控制工具，作者 [Romain Vimont](https://github.com/rom1v)
- **Android Debug Bridge (ADB)** — Google 官方 Android 调试工具
- **Qt** — 跨平台 C++ 应用开发框架
- **SDL** — 简单直接媒体层库
- **FFmpeg** — 多媒体处理库

## 作者

**厨房 / OMADAFAKA**

<div align="center">

</div>
