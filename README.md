# TinyBloom 🌱

一个免费、开源、注重真实使用体验的个人计划管理桌面应用，帮助用户把复杂目标拆分成更容易开始的小步骤。

> **Small Steps, Real Progress.**

## 📖 项目简介

TinyBloom 是一个基于 **C++20、Qt 6、QML、SQLite 和 CMake** 开发的桌面计划管理应用。

很多时候，真正困难的不是完成任务，而是开始任务。TinyBloom 希望通过任务管理、子任务拆分、平静的每日计划与本地数据存储，帮助用户把一个较大的目标拆分成多个更容易完成的小步骤，并持续看到自己的进步。

项目采用 **免费 + 开源 + Local-first** 的设计理念：

- 核心功能无需注册账号
- 无需订阅
- 不依赖网络服务
- 用户数据默认保存在本地 SQLite 数据库中
- 支持数据导入与导出

## ⬇️ 下载

Windows 用户可以从 [GitHub Releases](https://github.com/stellablack528/TinyBloom-Personal-Planner/releases) 下载 `TinyBloom-v0.1.0-windows-x64.zip`。解压完整目录后运行 `TinyBloom.exe`，无需安装 Qt 或注册账号。

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

### 🌱 每日计划与快速开始

- Today 首页和当日完成进度
- Today / Tomorrow / Later 自动分类
- 快速添加任务与键盘快捷键
- All / Active / Completed 状态筛选
- 根据标题、描述和分类搜索任务

### 🎨 UI 主题

TinyBloom v0.1.0 支持：

- 🌿 Mint Garden
- 🌙 Midnight
- 🇨🇳 简体中文（默认）
- 🇬🇧 English

主题和语言选择都会保存在本地数据库中，并在下次启动时恢复。

### 💾 本地数据

TinyBloom 使用 SQLite 保存本地数据，包括：

- Tasks
- Subtasks
- Settings

同时支持：

- Export JSON
- Import JSON

Focus Timer、XP、成就、Garden 和高级统计仍在 Roadmap 中，尚未包含在 v0.1.0。

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
- SettingsManager
- DatabaseManager
- DataService
- TaskListModel

C++ 主要负责：

- 数据管理
- 业务逻辑
- 任务系统
- 数据库操作
- JSON 导入与导出
- 输入校验与分类筛选

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
├── docs/
│   ├── architecture.md
│   └── development-log.md
│
├── src/
│   ├── main.cpp
│   ├── models/
│   ├── managers/
│   ├── database/
│   └── services/
│
├── qml/
│   ├── Main.qml
│   ├── pages/
│   ├── components/
│   └── themes/
│
├── resources/icons/
├── translations/
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
git clone https://github.com/stellablack528/TinyBloom-Personal-Planner.git
cd TinyBloom-Personal-Planner
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
- 数据导入与导出
- 主题切换
- 中英文语言设置
- 设置持久化

测试命令：

```bash
ctest --test-dir build --output-on-failure
```

## 🔐 隐私

TinyBloom 默认采用本地优先设计。

用户的任务、设置和个人数据默认保存在本地设备中。项目不要求：

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
- 子任务、搜索与筛选
- SQLite 持久化
- Today 页面
- 双主题与中英文界面
- JSON 导入与导出

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

- 更多主题
- 数据备份
- 个性化设置

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
