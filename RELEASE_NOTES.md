# insynic 1.10 Lite — 版本说明

## 版本号

**1.10 Lite**

## 版本概述

insynic 1.10 Lite 是首个公开发布版本。基于 scrcpy 引擎，为 macOS 用户提供完整的 Android 设备控制能力，集成了屏幕镜像、文件管理、OTG 输入、虚拟按键映射等核心功能。

## 功能列表

### 设备连接
- USB 数据线连接，自动检测已连接的 Android 设备
- 网络 (TCP/IP) 无线连接，支持自定义端口（默认 5555）
- 设备下拉列表自动列出可用设备

### 屏幕镜像
- 基于 scrcpy 的实时屏幕镜像
- 串流设置：可配置分辨率、帧率、比特率
- 关屏控制：关闭屏幕显示但仍可操作设备（不锁屏）
- 屏幕窗口大小自动适配视频帧分辨率

### 远程控制按钮
- 导航：Back、Home、Task、Menu
- 音量：音量增大 / 减小
- 面板：通知栏展开、设置面板展开 / 收起
- 屏幕旋转：一键切换横竖屏
- 电源：关屏 / 开屏切换

### OTG 输入
- 通过 USB OTG 模式将 Mac 键盘映射为 Android 输入设备
- 独立进程运行，不干扰主程序屏幕镜像
- 支持 Cmd+Q 快捷键退出
- 网络连接模式下自动禁用（AOA 仅支持 USB）

### 文件管理
- 基于 ADB 的文件浏览器
- 浏览设备内部存储文件

### 虚拟按键映射 (Utilities)
- Add Key：在屏幕窗口上放置可拖动的虚拟按键圆点
- 双击配置：监听键盘输入进行键位映射，可配置大小（20-100 像素）
- 支持同时添加多个虚拟按键
- 通过 scrcpy 控制协议发送触摸事件到设备对应坐标
- Save Profile：保存窗口位置、大小及所有虚拟按键配置
- Profile 管理：Apply 应用配置、Delete 删除配置

### 其他功能
- 系统状态栏菜单，支持显示 / 隐藏窗口
- 中英文双语切换，语言偏好自动保存
- macOS 原生深色主题
- 自定义应用图标

## 技术信息

- **平台**：macOS 12.0+
- **构建工具**：CMake 3.20+
- **核心依赖**：Qt 6、scrcpy、SDL3、FFmpeg、libusb
- **adb 和 scrcpy-server 已内置于应用包中**

## 已知限制

- OTG 输入仅支持 USB 连接，不支持网络连接
- 屏幕镜像窗口为独立弹窗，不支持嵌入主窗口
- 仅支持 H.264 视频编码
- 仅供 macOS x86_64 架构

## 作者

厨房 / OMADAFAKA

## 致谢

本项目基于以下开源工具构建：
- [scrcpy](https://github.com/Genymobile/scrcpy) — by Romain Vimont
- [Android Debug Bridge (ADB)](https://developer.android.com/tools/adb) — Google
- [Qt](https://www.qt.io/)
- [SDL](https://github.com/libsdl-org/SDL)
- [FFmpeg](https://ffmpeg.org/)

---

© 2025 厨房 / OMADAFAKA. All rights reserved.
