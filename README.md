# TinyBloom 🌱

一个免费、开源、注重真实使用体验的个人计划管理桌面应用，帮助用户把复杂目标拆分成更容易开始的小步骤。

> **Small Steps, Real Progress.**

## 📖 项目简介

TinyBloom 是一个基于 **C++20、Qt 6、QML、SQLite 和 CMake** 开发的桌面计划管理应用。

很多时候，真正困难的不是完成任务，而是开始任务。TinyBloom 希望通过任务管理、子任务拆分、专注计时、数据统计和轻量成就系统，帮助用户把一个较大的目标拆分成多个更容易完成的小步骤，并持续看到自己的进步。

项目采用 **免费 + 开源 + Local-first** 的设计理念：

- 核心功能无需注册账号
- 无需订阅
- 不依赖网络服务
- 用户数据默认保存在本地 SQLite 数据库中
- 支持数据导入、导出与备份

## ✨ 主要功能

### 📋 任务管理

TinyBloom 支持基础任务管理能力，包括：

- 创建任务
- 编辑任务
- 删除任务
- 完成 / 恢复任务
- 设置截止日期
- 设置优先级
- 设置预计时间
- 设置任务分类

### 🧩 子任务

用户可以将一个较大的目标拆分成多个更小的子任务，降低开始大型任务时的心理负担。

例如：

```text
学习计算机网络

✓ 阅读第一节
✓ 整理知识点
□ 完成两道练习题
□ 总结今天的内容
```

### 🌱 Small Steps

TinyBloom 提供“小步开始”功能，帮助用户先行动起来，再逐渐进入状态。

可选择的时间包括：

- 5 min
- 10 min
- 15 min
- 25 min

示例流程：

```text
Study Data Structures
        ↓
   Start Small
        ↓
    5 Minutes
```

### 🕐 Focus Timer

TinyBloom 提供简单的专注计时器，支持：

- Start
- Pause
- Resume
- Stop

完成一次专注后，应用会记录本次专注时间，并将其加入统计数据。

### ⭐ XP 与成就

完成任务可以获得 XP，用于记录用户的持续进步。

示例：

```text
完成任务        +10 XP
完成较长任务    +20 XP
完成今日计划    +50 XP
```

随着 XP 增长，用户可以提升等级，并通过成就系统记录长期进度。

### 🌿 Garden

完成任务、专注学习和解锁成就后，用户可以获得成长值。

成长值用于培育属于自己的小花园，让长期坚持变成可以直观看见的成长。

```text
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
```

### 📊 数据统计

TinyBloom 会统计用户的实际进度，包括：

- 完成任务数量
- 专注总时间
- XP
- 当前等级
- 连续完成天数

这些统计数据可以帮助用户回顾一段时间以来的积累。

### 🎨 UI 主题

TinyBloom 支持多种界面风格：

- 🌿 Mint Garden
- 🌸 Sakura
- 🌊 Ocean
- 💜 Lavender
- 🌙 Midnight

用户可以根据自己的喜好切换主题。

### 💾 本地数据

TinyBloom 使用 SQLite 保存本地数据，包括：

- Tasks
- Subtasks
- Achievements
- XP
- Focus Sessions
- Statistics
- Settings
- Garden Data

同时支持：

- Export JSON
- Import JSON
- Backup Database

## 🛠️ 技术栈

| 技术 | 用途 |
| --- | --- |
| C++20 | 核心业务逻辑 |
| Qt 6 | 桌面应用框架 |
| Qt Quick / QML | UI 与动画 |
| SQLite | 本地数据持久化 |
| Qt SQL | SQLite 数据访问 |
| CMake | 项目构建 |
| Qt Test / CTest | 自动化测试 |

## 🏗️ 项目架构

TinyBloom 采用 C++ 与 QML 分离的结构。

```text
QML UI
  ↓
Application Controller
  ↓
Managers
  ↓
Database Manager
  ↓
SQLite
```

核心模块包括：

- TaskManager
- FocusManager
- AchievementManager
- StatisticsManager
- GardenManager
- ThemeManager
- DatabaseManager

C++ 主要负责：

- 数据管理
- 业务逻辑
- 任务系统
- XP 与成就
- 数据库操作
- 统计

QML 主要负责：

- 页面布局
- UI 组件
- 动画
- 主题
- 用户交互

## 📂 项目结构

```text
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
```

## 🚀 运行方式

### 环境要求

运行和构建 TinyBloom 需要：

- C++20
- Qt 6.x
- CMake 3.20+
- 支持 C++20 的编译器
- Qt SQLite Driver

### Clone

```bash
git clone https://github.com/stellablack528/tinybloom.git
cd tinybloom
```

### Configure

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build --config Release
```

构建完成后，运行生成的 TinyBloom 可执行文件。

## 🧪 测试

项目重点测试以下内容：

- 任务创建与编辑
- 任务完成
- 子任务
- SQLite 数据保存与读取
- XP 计算
- 成就解锁
- Focus Timer
- 数据导入与导出
- 主题切换
- 设置持久化

测试命令：

```bash
ctest --test-dir build --output-on-failure
```

## 🔐 隐私

TinyBloom 默认采用本地优先设计。

用户的任务、统计和个人数据默认保存在本地设备中。项目不要求：

- 注册账号
- 登录
- 云端数据库
- 网络连接

核心功能可以在离线环境下使用。

## 🎯 项目目标

TinyBloom 希望成为一个：

- 真正免费的个人计划工具
- 简单易用的任务管理工具
- 注重用户体验的桌面软件
- C++ / Qt 学习项目
- 开源社区项目

项目的重点不是堆叠尽可能多的功能，而是让用户真正愿意使用它。

当功能数量与使用体验发生冲突时，优先保证：

> 简单、流畅、清晰和实用。

## 🗺️ Roadmap

### v0.1

- 基础桌面框架
- 任务管理
- SQLite 持久化
- Today 页面

### v0.2

- Focus Timer
- 基础统计

### v0.3

- XP
- 等级
- 成就系统

### v0.4

- Garden
- 成长系统

### v0.5

- 多主题
- UI 动画
- 个性化设置

### v0.6

- 数据导入
- 数据导出
- 数据备份

### v1.0

- 稳定版本
- 完整测试
- 完整文档
- 跨平台支持

## 🤖 AI-assisted Development

TinyBloom 也用于实践 AI 辅助软件开发。

开发流程：

```text
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
```

AI 生成的代码需要经过实际编译、运行、测试和修改后，才能进入项目。

## 👤 Author

**CS Student · Backend & Systems Enthusiast**

Currently learning and experimenting with:

- Modern C++ / C++20
- Linux Network Programming
- Concurrency & Server Architecture
- Database Fundamentals
- Qt Desktop Development

Mostly learning by building small projects and experimenting with ideas I find interesting.

## 📄 License

MIT License.

See [LICENSE](LICENSE) for details.

---

🌱 **Small Steps, Real Progress.**
