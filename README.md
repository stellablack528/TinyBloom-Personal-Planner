TinyBloom 🌱

一个免费、开源、注重真实使用体验的个人计划管理桌面应用，帮助用户把复杂的目标变成容易开始的小步骤。

📖 项目简介

TinyBloom 是一个基于 C++20、Qt 6、QML、SQLite 和 CMake 开发的桌面计划管理应用。

项目的核心理念是：

Small Steps, Real Progress.

很多时候，真正困难的不是完成任务，而是开始任务。

TinyBloom 希望通过简单的任务管理、子任务拆分、专注计时、统计和轻量成就系统，让用户能够把一个较大的目标拆分成多个容易完成的小步骤，并持续看到自己的进步。

项目采用 免费 + 开源 + Local-first 的设计理念。

核心功能无需注册账号、无需订阅，也不依赖网络服务。用户的数据默认保存在本地 SQLite 数据库中，并支持数据导入与导出。

✨ 主要功能

📋 任务管理

支持：

创建任务

编辑任务

删除任务

完成 / 恢复任务

设置截止日期

设置优先级

设置预计时间

设置任务分类

🧩 子任务

可以将一个较大的目标拆分成多个小任务。

例如：

学习计算机网络

✓ 阅读第一节
✓ 整理知识点
□ 完成两道练习题
□ 总结今天的内容

通过逐步完成子任务，降低开始大型任务时的负担。

🌱 Small Steps

提供小步开始功能。

用户可以选择：

5 min
10 min
15 min
25 min

例如：

Study Data Structures
        ↓
Start Small
        ↓
5 Minutes

帮助用户先开始行动，再逐渐进入状态。

🕐 Focus Timer

提供简单的专注计时器。

支持：

Start

Pause

Resume

Stop

完成专注后记录本次专注时间，并加入统计数据。

⭐ XP 与成就

完成任务可以获得 XP。

例如：

完成任务        +10 XP
完成较长任务    +20 XP
完成今日计划    +50 XP

随着 XP 增长提升等级，并通过成就记录长期进度。

🌿 Garden

完成任务、专注学习和解锁成就后，可以获得成长值。

成长值用于培育属于自己的小花园。

Task
 ↓
XP
 ↓
Growth
 ↓
🌱
 ↓
🌷
 ↓
🌳

让长期坚持变成能够直观看见的成长。

📊 数据统计

统计内容包括：

完成任务数量

专注总时间

XP

当前等级

连续完成天数

帮助用户回顾一段时间以来的实际进度。

🎨 UI 主题

支持多种界面风格：

🌿 Mint Garden

🌸 Sakura

🌊 Ocean

💜 Lavender

🌙 Midnight

用户可以根据自己的喜好切换主题。

💾 本地数据

使用 SQLite 保存：

Tasks

Subtasks

Achievements

XP

Focus Sessions

Statistics

Settings

Garden Data

并支持：

Export JSON
Import JSON
Backup Database

🛠️ 技术栈

技术

用途

C++20

核心业务逻辑

Qt 6

桌面应用框架

Qt Quick / QML

UI 与动画

SQLite

本地数据持久化

Qt SQL

SQLite 数据访问

CMake

项目构建

Qt Test / CTest

自动化测试

🏗️ 项目架构

项目采用 C++ 与 QML 分离的结构。

QML UI
   ↓
Application Controller
   ↓
Managers
   ↓
Database Manager
   ↓
SQLite

核心模块包括：

TaskManager
FocusManager
AchievementManager
StatisticsManager
GardenManager
ThemeManager
DatabaseManager

C++ 负责：

数据管理

业务逻辑

任务系统

XP 与成就

数据库操作

统计

QML 负责：

页面布局

UI 组件

动画

主题

用户交互

📂 项目结构

tinybloom/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── CHANGELOG.md
├── CONTRIBUTING.md
│
├── docs/
│   ├── architecture.md
│   └── development-log.md
│
├── src/
│   ├── main.cpp
│   ├── core/
│   ├── managers/
│   ├── database/
│   └── services/
│
├── qml/
│   ├── Main.qml
│   ├── pages/
│   ├── components/
│   ├── dialogs/
│   └── themes/
│
├── resources/
│   └── resources.qrc
│
└── tests/

🚀 运行方式

环境要求

C++20

Qt 6.x

CMake 3.20+

支持 C++20 的编译器

Qt SQLite Driver

Clone

git clone https://github.com/stellablack528/tinybloom.git

cd tinybloom

Configure

cmake -S . -B build

Build

cmake --build build --config Release

随后运行生成的 TinyBloom 可执行文件。

🧪 测试

项目重点测试：

任务创建与编辑

任务完成

子任务

SQLite 数据保存与读取

XP 计算

成就解锁

Focus Timer

数据导入与导出

主题切换

设置持久化

测试命令：

ctest --test-dir build --output-on-failure

🔐 隐私

TinyBloom 默认采用本地优先设计。

用户的任务、统计和个人数据默认保存在本地设备中。

项目不要求：

注册账号

登录

云端数据库

网络连接

核心功能可以在离线环境下使用。

🎯 项目目标

TinyBloom 希望成为一个：

真正免费的个人计划工具

简单易用的任务管理工具

注重用户体验的桌面软件

C++ / Qt 学习项目

开源社区项目

项目的重点不是增加尽可能多的功能，而是让用户真正愿意使用它。

当功能数量与使用体验发生冲突时，优先保证：

简单、流畅、清晰和实用。

🗺️ Roadmap

v0.1

基础桌面框架

任务管理

SQLite 持久化

Today 页面

v0.2

Focus Timer

基础统计

v0.3

XP

等级

成就系统

v0.4

Garden

成长系统

v0.5

多主题

UI 动画

个性化设置

v0.6

数据导入

数据导出

数据备份

v1.0

稳定版本

完整测试

完整文档

跨平台支持

🤖 AI-assisted Development

TinyBloom 也用于实践 AI 辅助软件开发。

开发流程：

Product Idea
    ↓
Requirement Design
    ↓
Architecture Design
    ↓
C++ Implementation
    ↓
QML UI
    ↓
SQLite Integration
    ↓
Testing
    ↓
Bug Fixing
    ↓
Performance Optimization
    ↓
Documentation

AI 生成的代码需要经过实际编译、运行、测试和修改。

👤 Author

CS Student · Backend & Systems Enthusiast

Currently learning and experimenting with:

Modern C++ / C++20
Linux Network Programming
Concurrency & Server Architecture
Database Fundamentals
Qt Desktop Development
Mostly learning by building small projects and messing around with ideas I find interesting.
📄 License

MIT License

See LICENSE for details.

🌱 Small Steps, Real Progress.
